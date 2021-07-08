#pragma once

#include "GraphicObject.h"
#include "Camera.h"

namespace Graphics
{
	using ObjectPtrPool = std::vector<const GraphicObject*>;

	struct Octree
	{
	public:
		Octree(uint32_t _depth, BoundingBox rootBoundingBox);
		~Octree();

		void AddObject(const GraphicObject* newObject, bool isDynamic);
		void PrepareVisibleObjectsList(const Camera& targetCamera, ObjectPtrPool& visibleObjectsList, ObjectPtrPool& visibleTransparentObjectsList,
			ObjectPtrPool& visibleEffectObjectsList);
		
	private:
		Octree() = delete;

		struct Node
		{
			BoundingBox boundingBox;

			ObjectPtrPool objects;
			ObjectPtrPool dynamicObjects;
			std::array<std::shared_ptr<Node>, 8> nextNodes;
		};

		void CreateNodeChain(uint32_t currentDepth, Node* currentNode);
		void SplitBoundingBox(const BoundingBox& boundingBox, std::array<BoundingBox, 8>& splittedBoundingBox);

		void AddObject(Node* currentNode, const GraphicObject* newObject, bool isDynamic);
		void PushObjectToNode(Node* node, const GraphicObject* newObject, bool isDynamic);

		void PrepareVisibleObjectsList(const Node* currentNode, const Camera& targetCamera, ObjectPtrPool& visibleObjectsList,
			ObjectPtrPool& visibleTransparentObjectsList, ObjectPtrPool& visibleEffectObjectsList);

		Node root;

		uint32_t depth;
	};
}
