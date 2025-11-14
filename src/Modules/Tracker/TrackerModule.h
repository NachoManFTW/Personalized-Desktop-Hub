#pragma once
#include "Core/IModule.h"
#include "imgui.h"

#include <string>
#include <map>
#include <chrono>
#include <atomic>
#include <mutex>
#include <Windows.h>



class TrackerModule : public IModule
{
private:
	ImGuiWindowFlags m_Flags = ImGuiWindowFlags_None | ImGuiWindowFlags_MenuBar;

	//Windows api calls
	HWND m_ForegroundWindow = nullptr;
	HWND m_PreviousWindow = nullptr;
	DWORD m_ProcessId;
	HANDLE m_ProcessHandle = nullptr;

	//Tracker state
	std::atomic<bool> m_IsRunning = false;
	std::atomic<bool> m_IsSaving = false;
	bool m_ToggleTracking = false;
	bool m_IsVisible = false;


	//Current session tracking
	std::string m_AppKey;
	std::string m_CurrentApplication;
	std::chrono::high_resolution_clock::time_point m_StartTime;
	long m_TotalDuration = 0;

	//Storage containers
	std::map<std::string, long> m_ApplicationTotals;
	std::map<std::string, long> m_SessionDurations;


	//Logging
	std::string m_LogPath;
	std::mutex m_TrackerMutex;
	std::thread m_WorkerThread;

	//Styling
	ImGuiStyle style;

public:

	TrackerModule();
	~TrackerModule();

	//Main loop functions
	void Init() override;
	void Update(float dt) override;
	void Render() override;
	void Shutdown() override;


	const char* GetName() const override;

private:
	//Process helpers
	DWORD GetProcessId() const;
	HANDLE GetProcessHandle();
	std::string GetProcessPath() const;
	std::string GetExecutableName() const;


	//Window helpers
	void Refresh();
	void SynchronizeWindows(HWND currentWindow);
	std::string GetWindowTitle() const;

	//Time
	std::string Clock();
	void ResetTimer();
	long GetWindowDuration();
	long GetTotalDailyTime();

	//Logging
	void LoadFromJson();
	void SaveToJson();

	//Threads
	void StartWorkerThread();

	//Helpers
	std::string FormatTime(long seconds);
	std::vector<std::pair<std::string, long>> ConvertMapToVector();

	//Render Helpers
	void RenderTable();
	void RenderMenuBar();
};

