#include "Octree.h"

Graphics::Octree::Octree(uint32_t _depth, BoundingBox rootBoundingBox)
	: depth(_depth)
{
	root.boundingBox = rootBoundingBox;
	CreateNodeChain(depth, &root);
}

Graphics::Octree::~Octree()
{

}

void Graphics::Octree::AddObject(const GraphicObject* newObject, bool isDynamic)
{
	if (newObject == nullptr)
		return;

	for (auto& nextNode : root.nextNodes)
		if (CheckBoxInBox(newObject->GetBoundingBox(), nextNode->boundingBox))
		{
			AddObject(nextNode.get(), newObject, isDynamic);

			return;
		}

	PushObjectToNode(&root, newObject, isDynamic);
}

void Graphics::Octree::PrepareVisibleObjectsList(const Camera& targetCamera, std::vector<const GraphicObject*>& visibleObjectsList)
{
	for (auto& currentObject : root.objects)
		if (targetCamera.BoundingBoxInScope(currentObject->GetBoundingBox()))
			visibleObjectsList.push_back(currentObject);

	for (auto& currentObject : root.dynamicObjects)
		if (targetCamera.BoundingBoxInScope(currentObject->GetBoundingBox()))
			visibleObjectsList.push_back(currentObject);

	for (auto& nextNode : root.nextNodes)
		PrepareVisibleObjectsList(nextNode.get(), targetCamera, visibleObjectsList);
}

void Graphics::Octree::CreateNodeChain(uint32_t currentDepth, Node* currentNode)
{
	if (currentNode == nullptr)
		return;

	if (currentDepth == 0)
		return;

	std::array<Graphics::BoundingBox, 8> boundingBoxes;
	SplitBoundingBox(currentNode->boundingBox, boundingBoxes);

	for (uint32_t nextNodeId = 0; nextNodeId < currentNode->nextNodes.size(); nextNodeId++)
	{
		currentNode->nextNodes[nextNodeId] = std::make_shared<Node>();
		currentNode->nextNodes[nextNodeId]->boundingBox = boundingBoxes[nextNodeId];

		CreateNodeChain(currentDepth - 1, currentNode->nextNodes[nextNodeId].get());
	}
}

void Graphics::Octree::SplitBoundingBox(const BoundingBox& boundingBox, std::array<BoundingBox, 8>& splittedBoundingBox)
{
	float3 center;
	XMStoreFloat3(&center, (XMLoadFloat3(&boundingBox.minCornerPoint) + XMLoadFloat3(&boundingBox.maxCornerPoint)) / 2.0f);

	const float3& minPoint = boundingBox.minCornerPoint;
	const float3& maxPoint = boundingBox.maxCornerPoint;

	splittedBoundingBox =
	{
		BoundingBox{center, minPoint},
		BoundingBox{{minPoint.x, center.y, center.z}, {center.x, maxPoint.y, maxPoint.z}},
		BoundingBox{{minPoint.x, center.y, minPoint.z}, {center.x, maxPoint.y, center.z}},
		BoundingBox{{center.x, minPoint.y, minPoint.z}, {maxPoint.x, maxPoint.y, center.z}},
		BoundingBox{{center.x, minPoint.y, center.z}, {maxPoint.x, center.y, maxPoint.z}},
		BoundingBox{{minPoint.x, minPoint.y, center.z}, {center.x, center.y, maxPoint.z}},
		BoundingBox{minPoint, center},
		BoundingBox{{center.x, minPoint.y, minPoint.z}, {maxPoint.x, center.y, center.z}}
	};
}

void Graphics::Octree::AddObject(Node* currentNode, const GraphicObject* newObject, bool isDynamic)
{
	if (currentNode->nextNodes[0] == nullptr)
		return;

	for (auto& nextNode : currentNode->nextNodes)
		if (CheckBoxInBox(newObject->GetBoundingBox(), nextNode->boundingBox))
		{
			AddObject(nextNode.get(), newObject, isDynamic);

			return;
		}

	PushObjectToNode(currentNode, newObject, isDynamic);
}

void Graphics::Octree::PushObjectToNode(Node* node, const GraphicObject* newObject, bool isDynamic)
{
	if (isDynamic)
		node->dynamicObjects.push_back(newObject);
	else
		node->objects.push_back(newObject);
}

void Graphics::Octree::PrepareVisibleObjectsList(const Node* currentNode, const Camera& targetCamera, std::vector<const GraphicObject*>& visibleObjectsList)
{
	if (currentNode == nullptr)
		return;

	for (auto& currentObject : currentNode->objects)
		if (targetCamera.BoundingBoxInScope(currentObject->GetBoundingBox()))
			visibleObjectsList.push_back(currentObject);

	for (auto& currentObject : currentNode->dynamicObjects)
		if (targetCamera.BoundingBoxInScope(currentObject->GetBoundingBox()))
			visibleObjectsList.push_back(currentObject);

	for (auto& nextNode : currentNode->nextNodes)
		PrepareVisibleObjectsList(nextNode.get(), targetCamera, visibleObjectsList);
}
