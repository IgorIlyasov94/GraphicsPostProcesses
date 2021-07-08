#include "Scene.h"

Graphics::Scene::Scene()
	: mainCamera(nullptr)
{
	const uint32_t octreeDepth = 5;
	const BoundingBox octreeBoundingBox = { {-1024.0f, -1024.0f, -1024.0f}, {1024.0f, 1024.0f, 1024.0f} };

	octree = std::shared_ptr<Octree>(new Octree(octreeDepth, octreeBoundingBox));

	lightingSystem = std::shared_ptr<LightingSystem>(new LightingSystem());
	uiSystem = std::shared_ptr<UISystem>(new UISystem());
}

Graphics::Scene::~Scene()
{

}

Graphics::LightingSystem* Graphics::Scene::GetLightingSystem()
{
	return lightingSystem.get();
}

Graphics::UISystem* Graphics::Scene::GetUISystem()
{
	return uiSystem.get();
}

void Graphics::Scene::SetMainCamera(const Camera* camera)
{
	mainCamera = camera;
}

void Graphics::Scene::EmplaceComputeObject(const ComputeObject* object)
{
	if (object == nullptr)
		return;

	if (!object->IsComposed())
		throw std::exception("Graphics::Scene::EmplaceComputeObject: ComputeObject is not composed");

	computeObjects.push_back(object);
}

void Graphics::Scene::EmplaceGraphicObject(const GraphicObject* object, bool isDynamic)
{
	octree->AddObject(object, isDynamic);
}

void Graphics::Scene::ExecuteScripts(ID3D12GraphicsCommandList* commandList, size_t mouseX, size_t mouseY)
{
	for (auto& computeObject : computeObjects)
		computeObject->Present(commandList);

	if (mainCamera == nullptr)
		return;

	visibleObjectsList.clear();
	visibleTransparentObjectsList.clear();
	visibleEffectObjectsList.clear();
	octree->PrepareVisibleObjectsList(*mainCamera, visibleObjectsList, visibleTransparentObjectsList, visibleEffectObjectsList);

	for (auto& visibleObject : visibleObjectsList)
		visibleObject->Execute(commandList);

	for (auto& visibleObject : visibleTransparentObjectsList)
		visibleObject->Execute(commandList);

	for (auto& visibleObject : visibleEffectObjectsList)
		visibleObject->Execute(commandList);

	lightingSystem->UpdateCluster(commandList);

	uiSystem->ExecuteScripts(mouseX, mouseY);
}

void Graphics::Scene::Draw(ID3D12GraphicsCommandList* commandList) const
{
	for (auto& visibleObject : visibleObjectsList)
		visibleObject->Draw(commandList);

	for (auto& visibleObject : visibleTransparentObjectsList)
		visibleObject->Draw(commandList);

	for (auto& visibleObject : visibleEffectObjectsList)
		visibleObject->Draw(commandList);
}

void Graphics::Scene::DrawUI(ID3D12GraphicsCommandList* commandList) const
{
	uiSystem->Draw(commandList);
}
