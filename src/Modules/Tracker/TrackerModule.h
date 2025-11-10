#pragma once
#include "Core/IModule.h"
#include <imgui.h>
#include <string>
#include <map>
#include <chrono>
#include <atomic>
#include <mutex>
#include <Windows.h>



class TrackerModule : public IModule
{
private:
	ImGuiWindowFlags m_Flags = ImGuiWindowFlags_None;

	//Windows api calls
	HWND m_ForegroundWindow;
	HWND m_PreviousWindow;
	DWORD m_ProcessId;
	HANDLE m_ProcessHandle;

	//Tracking state
	bool m_ToggleTracking;
	bool m_IsVisible;
	std::atomic<bool> m_IsRunning;
	std::atomic<bool> m_IsSaved;

	//Current app track
	std::string m_AppKey;
	std::string m_CurrentApplication;
	std::chrono::high_resolution_clock::time_point m_StartTime;
	long m_TotalDuration;

	//Logging
	std::string m_LogPath;
	std::mutex m_TrackerMutex;
	std::map<std::string, long> m_ApplicationList;

public:

	TrackerModule();
	~TrackerModule();

	//Main loop functions
	void Init() override;
	void Update(float dt) override;
	void Render() override;
	void Shutdown() override;


	const char* GetName() const override;

	//Getters for current active executable data
	DWORD GetProcessId() const;
	HANDLE GetProcessHandle();
	std::string GetProcessPath() const;
	std::string GetExecutableName() const;


	//Get current active window information and reset windows
	void Refresh();
	void SynchronizeWindows(HWND currentWindow);
	std::string GetWindowTitle() const;

	//Time related functions
	std::string Clock();
	void ResetTimer();
	long GetWindowDuration();

	//Saving functions
	void SaveToJson();
	void PeriodicSaver();
	void WindowChanges();
};

