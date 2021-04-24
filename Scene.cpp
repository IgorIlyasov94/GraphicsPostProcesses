#include "Scene.h"

Graphics::Scene::Scene()
{
	const float fovAngleY = XM_PI / 4.0f;
	const float aspectRatio = GraphicsSettings::GetResolutionX() / static_cast<float>(GraphicsSettings::GetResolutionY());
	const float nearZ = 0.01f;
	const float farZ = 1024.0f;

	camera = std::make_shared<Camera>(fovAngleY, aspectRatio, nearZ, farZ);

	const uint32_t octreeDepth = 5;
	const BoundingBox octreeBoundingBox = { {-1024.0f, -1024.0f, -1024.0f}, {1024.0f, 1024.0f, 1024.0f} };

	octree = std::make_shared<Octree>(octreeDepth, octreeBoundingBox);
}

Graphics::Scene::~Scene()
{

}

void Graphics::Scene::EmplaceGraphicObject(const GraphicObject* object, bool isDynamic)
{
	octree->AddObject(object, isDynamic);
}

void Graphics::Scene::Draw(ID3D12GraphicsCommandList* commandList) const
{
	std::vector<const GraphicObject*> visibleObjectsList;

	octree->PrepareVisibleObjectsList(*camera.get(), visibleObjectsList);

	for (auto& visibleObject : visibleObjectsList)
		visibleObject->Draw(commandList);
}
