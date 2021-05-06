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

void Graphics::Scene::EmplaceGraphicObject(const GraphicObject* object, bool isDynamic)
{
	octree->AddObject(object, isDynamic);
}

void Graphics::Scene::Draw(ID3D12GraphicsCommandList* commandList) const
{
	if (mainCamera == nullptr)
		return;

	std::vector<const GraphicObject*> visibleObjectsList;

	octree->PrepareVisibleObjectsList(*mainCamera, visibleObjectsList);

	for (auto& visibleObject : visibleObjectsList)
		visibleObject->Draw(commandList);
}
