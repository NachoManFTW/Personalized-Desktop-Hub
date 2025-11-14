#pragma once
#include "Core/IModule.h"
#include "imgui.h"

#include <windows.h>
#include <iomanip>
#include <vector>
#include <string>


class VirtualMemoryMapModule : public IModule
{
private:
	//UI / Config
	ImGuiWindowFlags m_Flags = ImGuiWindowFlags_None;
	int m_MaxSampleBytes = 16;
	float m_AutoRefreshSeconds = 10.0f;



	struct ms_MemRegion
	{
		void* ms_Base;
		SIZE_T ms_Size;
		DWORD ms_State;
		DWORD ms_Protect;
		std::string ms_SampleHex;
	};

	//Cached Data
	std::vector<ms_MemRegion> m_MemRegions;
	double m_LastRefreshTime = 0.0;
	
public:
	//Constructor/Deconstructor
	VirtualMemoryMapModule() = default;
	~VirtualMemoryMapModule() override = default;

	//Lifecycle
	void Init() override;
	void Update(float dt) override;
	void Render() override;
	void Shutdown() override;

	const char* GetName() const override { return "Memory Map"; }

private:
	//Helpers
	void BuildMemoryMap();
	std::string SampleBytesAsHex(void* addr, SIZE_T regionSize, SIZE_T maxSample);
	std::string ProtectToString(DWORD protect);
	const char* MemStateToString(DWORD state);
	std::string FormatSize(SIZE_T bytes);
	
};

