#include "Scene.h"

Graphics::Scene::Scene()
	: mainCamera(nullptr)
{
	const uint32_t octreeDepth = 5;
	const BoundingBox octreeBoundingBox = { {-1024.0f, -1024.0f, -1024.0f}, {1024.0f, 1024.0f, 1024.0f} };

	octree = std::make_shared<Octree>(octreeDepth, octreeBoundingBox);
}

Graphics::Scene::~Scene()
{

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

void Graphics::Scene::ExecuteScripts(ID3D12GraphicsCommandList* commandList)
{
	for (auto& computeObject : computeObjects)
		computeObject->Present(commandList);

	if (mainCamera == nullptr)
		return;

	visibleObjectsList.clear();
	octree->PrepareVisibleObjectsList(*mainCamera, visibleObjectsList);

	for (auto& visibleObject : visibleObjectsList)
		visibleObject->Execute(commandList);
}

void Graphics::Scene::Draw(ID3D12GraphicsCommandList* commandList) const
{
	for (auto& visibleObject : visibleObjectsList)
		visibleObject->Draw(commandList);
}
