#include "TrackerModule.h"
#include "json.hpp"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <thread>
#include <vector>


//--------------------
// Constructor / Destructor
//--------------------
TrackerModule::TrackerModule()
{
	this->m_ForegroundWindow = GetForegroundWindow();
	m_ProcessId = GetProcessId();
	m_ProcessHandle = GetProcessHandle();
	m_StartTime = std::chrono::high_resolution_clock::now();
}

TrackerModule::~TrackerModule()
{

}


//--------------------
// Lifecycle
//--------------------
void TrackerModule::Init()
{
	LoadFromJson();
	StartWorkerThread();
}

void TrackerModule::Update(float dt)
{
}

void TrackerModule::Render()
{
	ImGui::Begin("Tracker", nullptr, m_Flags);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 10.0f));
	ImGui::Text(("Current Application: " + m_CurrentApplication).c_str());
	RenderMenuBar();
	RenderTable();

	ImGui::PopStyleVar();
	ImGui::End();
}

void TrackerModule::Shutdown()
{
	m_IsRunning = false;
	if (m_WorkerThread.joinable())
		m_WorkerThread.join();
	//if (m_ProcessHandle)
		//CloseHandle(m_ProcessHandle);
	SaveToJson();
}

const char* TrackerModule::GetName() const
{
	return "Tracker";
}


//--------------------
// Process helpers
//--------------------
HANDLE TrackerModule::GetProcessHandle()
{
	return OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, m_ProcessId);
}

DWORD TrackerModule::GetProcessId() const
{
	DWORD pid = 0;
	GetWindowThreadProcessId(m_ForegroundWindow, &pid);
	return pid;
}

std::string TrackerModule::GetProcessPath() const
{
	char buffer[MAX_PATH];
	DWORD size = MAX_PATH;
	if (QueryFullProcessImageNameA(m_ProcessHandle, 0, buffer, &size))
		return std::string(buffer);
	return "Unknown";
}

std::string TrackerModule::GetExecutableName() const
{
	std::string path = GetProcessPath();
	size_t pos = path.find_last_of("\\/");
	return (pos != std::string::npos) ? path.substr(pos + 1) : path;
}

//--------------------
// Window tracking
//--------------------
void TrackerModule::Refresh()
{
	m_ForegroundWindow = GetForegroundWindow();
	m_ProcessId = GetProcessId();

	if (m_ProcessHandle)
		CloseHandle(m_ProcessHandle);
	m_ProcessHandle = GetProcessHandle();
}

void TrackerModule::SynchronizeWindows(HWND currentWindow)
{
	m_PreviousWindow = currentWindow;
	m_CurrentApplication = GetExecutableName();
}

std::string TrackerModule::GetWindowTitle() const
{
	char title[256];
	GetWindowTextA(m_ForegroundWindow, title, sizeof(title));
	return std::string(title);
}


//--------------------
// Time + Utility
//--------------------
std::string TrackerModule::Clock()
{
	auto todayDate = std::chrono::system_clock::now();
	std::time_t now_c = std::chrono::system_clock::to_time_t(todayDate);
	tm local_time_struct;
	localtime_s(&local_time_struct, &now_c);

	char buffer[80];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d", &local_time_struct);
	return std::string(buffer);
}

void TrackerModule::ResetTimer()
{
	this->m_StartTime = std::chrono::high_resolution_clock::now();
}

long TrackerModule::GetWindowDuration()
{
	auto endTime = std::chrono::high_resolution_clock::now();;
	return std::chrono::duration_cast<std::chrono::seconds>(endTime - m_StartTime).count();
}

long TrackerModule::GetTotalDailyTime()
{
	float totalTime = 0;
	for (auto& [app, duration] : m_ApplicationTotals)
		totalTime += duration;

	return totalTime;
}


//--------------------
// Json Handling
//--------------------
void TrackerModule::LoadFromJson()
{
	std::filesystem::path soluitionRoot = std::filesystem::current_path();
	std::filesystem::path logDir = soluitionRoot / "Logs";

	if (!std::filesystem::exists(logDir))
		std::filesystem::create_directory(logDir);

	std::filesystem::path logPath = logDir / ("usage_" + Clock() + ".json");
	m_LogPath = logPath.string();

	/*
	if (!std::filesystem::exists(logPath))
	{
		std::cout << "Couldn't return file location\n";
		return;
	}
	*/

	nlohmann::json data;

	if (!std::filesystem::exists(logDir))
	{
		std::ofstream createFile(logPath);
		createFile << "{}";
		createFile.close();
		return;
	}

	std::ifstream inFile(logPath);
	try 
	{ 
		inFile >> data; 
	} 
	catch (nlohmann::json::parse_error& e)
	{
		std::cerr << "JSON parse error: " << e.what() << "\n";
		return;
	}

	for (auto& [app, obj] : data.items())
	{
		if (obj.contains("Total Duration"))
		m_ApplicationTotals[app] = obj["Total Duration"].get<long>();
	}
	std::cout << "Loaded " << m_ApplicationTotals.size() << " entries from " << logPath << "\n";
}

void TrackerModule::SaveToJson()
{
	if (m_IsSaving.exchange(true)) 
		return; //Prevent concurrent saves
	
	std::map<std::string, long> snapshot;
	{
		std::lock_guard<std::mutex> lock(m_TrackerMutex);
		snapshot = m_SessionDurations;
	}

	//Daily rollover handling
	std::filesystem::path solutionRoot = std::filesystem::current_path();
	std::filesystem::path logDir = solutionRoot / "Logs";
	std::string currentDate = Clock();
	std::filesystem::path newPath = logDir / ("usage_" + currentDate + ".json");

	if (m_LogPath != newPath.string())
	{
		std::cout << "New day detected. Rolling over to. " << newPath << "\n";
		m_LogPath = newPath.string();
		LoadFromJson();
	}

	if (!std::filesystem::exists(logDir))
		std::filesystem::create_directory(logDir);

	//Load old totals
	nlohmann::json jsonData;
	if (std::filesystem::exists(m_LogPath))
	{
		std::ifstream inFile(m_LogPath);
		try
		{
			inFile >> jsonData;
		}
		catch (...)
		{
			jsonData = {};
		}
	}

	//Merge snapshot into totals
	{
		for (const auto& [app, duration] : snapshot)
		{
			m_ApplicationTotals[app] += duration;
			jsonData[app] = { {"Application", app}, {"Total Duration", m_ApplicationTotals[app]} };
		}
	}

	//Save updated file
	{
		std::ofstream outFile(m_LogPath, std::ios::trunc);
		outFile << jsonData.dump(4);
		outFile.close();
	}

	//Reset session
	{
		std::lock_guard<std::mutex> lock(m_TrackerMutex);
		m_SessionDurations.clear();	
	}

	m_IsSaving = false;
	std::cout << "Saved to " << m_LogPath << "\n";
}


//--------------------
// Threads
//--------------------
void TrackerModule::StartWorkerThread()
{
	m_IsRunning = true;
	m_WorkerThread = std::thread([this]()
	{
		auto lastSaveTime = std::chrono::steady_clock::now();
		auto lastDayCheck = std::chrono::system_clock::now();
		auto currentDate = Clock();

		m_PreviousWindow = GetForegroundWindow();
		m_CurrentApplication = GetExecutableName();
		ResetTimer();

		while (m_IsRunning)
		{
			m_ForegroundWindow = GetForegroundWindow(); 

			if (m_ForegroundWindow != m_PreviousWindow && m_ForegroundWindow != nullptr)
			{
				std::lock_guard<std::mutex> lock(m_TrackerMutex);

				long duration = GetWindowDuration();
				m_SessionDurations[m_CurrentApplication] += duration;
				
				Refresh();
				SynchronizeWindows(m_ForegroundWindow);
				ResetTimer();
			}

			
			if (duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - lastSaveTime).count() >= 5)
			{
				SaveToJson();
				lastSaveTime = std::chrono::steady_clock::now();
			}
			
			if (Clock() != currentDate)
			{
				std::lock_guard<std::mutex> lock(m_TrackerMutex);
				SaveToJson();
				m_ApplicationTotals.clear();
				currentDate = Clock();
				std::cout << "New log day started: " << currentDate << std::endl;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	});
	m_WorkerThread.detach();
}


//--------------------
// Render Helpers
//--------------------
void TrackerModule::RenderTable()
{
	std::vector<std::pair<std::string, long>> sortedApps = ConvertMapToVector();

	std::sort(sortedApps.begin(), sortedApps.end(), [](const auto& a, const auto& b) { return a.second > b.second; });

	if (ImGui::BeginTable("Apps", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("Application");
		ImGui::TableSetupColumn("Duration (s)");
		ImGui::TableHeadersRow();

		for (const auto& [app, duration] : sortedApps)
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(app.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%ld", duration);
			ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(0.1f, 0.1f, 0.1f, 0.5f)));

		}
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);

		ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImVec4(0.4f, 0.1f, 0.1f, 0.3f)));

		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.3f, 1.0f)); //Bright Green
		ImGui::TextUnformatted("Total Time (HH:MM:SS)");

		ImGui::TableSetColumnIndex(1);
		ImGui::TextUnformatted(FormatTime(GetTotalDailyTime()).c_str());
		ImGui::PopStyleColor();
		ImGui::EndTable();
	}
}

void TrackerModule::RenderMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			ImGui::EndMenu();
		}
		
		if (ImGui::BeginMenu("Settings"))
		{
			ImGui::EndMenu();
	
		}

		ImGui::EndMenuBar();
	}
	
	
}


//--------------------
// Helpers
//--------------------
std::string TrackerModule::FormatTime(long seconds)
{
	int hours = seconds / 3600;
	int minutes = (seconds % 3600) / 60;
	int secs = seconds % 60;

	char buffer[16];
	snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", hours, minutes, secs);
	return buffer;
}

std::vector<std::pair<std::string, long>> TrackerModule::ConvertMapToVector()
{
	std::lock_guard<std::mutex> lock(m_TrackerMutex);
	std::vector<std::pair<std::string, long>> sortedApps(m_ApplicationTotals.begin(), m_ApplicationTotals.end());
	return sortedApps;
}
