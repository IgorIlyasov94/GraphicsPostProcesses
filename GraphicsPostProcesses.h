#pragma once

#include "stdafx.h"

using namespace Microsoft::WRL;

class GraphicsPostProcesses
{
public:
	static GraphicsPostProcesses& getInstance();

	void initialize(const int32_t& resolutionX, const int32_t& resolutionY, const ID3D12Device*& device);
	void enableHDR();

private:
	GraphicsPostProcesses();
	~GraphicsPostProcesses() {}

	GraphicsPostProcesses(const GraphicsPostProcesses&) = delete;
	GraphicsPostProcesses(GraphicsPostProcesses&&) = delete;
	GraphicsPostProcesses& operator=(const GraphicsPostProcesses&) = delete;
	GraphicsPostProcesses& operator=(GraphicsPostProcesses&&) = delete;

	ComPtr<ID3D12PipelineState> hdrPipelineState;
};