#pragma once

#include "stdafx.h"
#include "GeometryStructures.h"

namespace Graphics
{
	class IMeshLoader
	{
	public:
		virtual ~IMeshLoader() {};

		virtual void Load(const std::filesystem::path& filePath, SplittedMeshData& splittedMeshData) = 0;
	};
}
