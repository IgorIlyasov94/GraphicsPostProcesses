#pragma once

#include "GraphicObject.h"
#include "Camera.h"

namespace Graphics
{
	struct Octree
	{
	public:
		Octree(uint32_t _depth, BoundingBox rootBoundingBox);
		~Octree();

		void AddObject(const GraphicObject* newObject, bool isDynamic);
		void PrepareVisibleObjectsList(const Camera& targetCamera, std::vector<const GraphicObject*>& visibleObjectsList);
		
	private:
		Octree() = delete;

		using ObjectPtrPool = std::list<const GraphicObject*>;

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

		void PrepareVisibleObjectsList(const Node* currentNode, const Camera& targetCamera, std::vector<const GraphicObject*>& visibleObjectsList);

		Node root;

		uint32_t depth;
	};
}
