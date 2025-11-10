#include "TrackerModule.h"
#include "json.hpp"
#include <fstream>
#include <filesystem>
#include <iostream>
#include <thread>


TrackerModule::TrackerModule()
{
	this->m_ForegroundWindow = GetForegroundWindow();
	this->m_PreviousWindow = nullptr;

	this->m_ProcessId = GetProcessId();
	this->m_ProcessHandle = GetProcessHandle();

	this->m_TotalDuration = 0;
	this->m_AppKey = GetProcessPath();
	this->m_IsRunning = false;
	this->m_IsSaved = false;

	this->m_ApplicationList = {};
}

TrackerModule::~TrackerModule()
{
	
}

//, ran once
void TrackerModule::Init()
{
	WindowChanges();
	//PeriodicSaver();

}

//, ran every frame
void TrackerModule::Update(float dt)
{

}

//Draw to screen, ran every frame 
void TrackerModule::Render()
{

	ImGui::Begin("Tracker", nullptr, m_Flags);
	{
		ImGui::Text(("Current Application: " + m_CurrentApplication).c_str());
	}
	ImGui::End();
}

//Clear resources allocated to object, ran once
void TrackerModule::Shutdown()
{
	SaveToJson();
	m_IsRunning = false;
	if (m_ProcessHandle)
		CloseHandle(m_ProcessHandle);
}

//Return the module name
const char* TrackerModule::GetName() const
{
	return "Tracker";
}


//***************************
//Current executable data 
//***************************
//Return the handle to the current process(active window)
HANDLE TrackerModule::GetProcessHandle()
{
	return OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, m_ProcessId);
}

//Get the process id for the current active window
DWORD TrackerModule::GetProcessId() const
{
	DWORD pid = 0;
	GetWindowThreadProcessId(m_ForegroundWindow, &pid);
	return pid;
}

//Return the path to the current active window
std::string TrackerModule::GetProcessPath() const
{
	char buffer[MAX_PATH];
	DWORD size = MAX_PATH;
	if (QueryFullProcessImageNameA(m_ProcessHandle, 0, buffer, &size))
		return std::string(buffer);
	return "Unknown";
}

//Return the current executable name
//If it can't return the path to the current executable
std::string TrackerModule::GetExecutableName() const
{
	std::string path = GetProcessPath();
	size_t pos = path.find_last_of("\\/");
	return (pos != std::string::npos) ? path.substr(pos + 1) : path;
}


//***************************
//Current Window data 
//***************************
//Get the current foreground window and that windows process id
//If it has a process id, close that handle and create a new handle
void TrackerModule::Refresh()
{
	m_ForegroundWindow = GetForegroundWindow();
	m_ProcessId = GetProcessId();

	if (m_ProcessHandle)
		CloseHandle(m_ProcessHandle);
	m_ProcessHandle = GetProcessHandle();
}

//Set previous window to current active window
//Get current active window's executable name
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


//***************************
//Saving 
//***************************
//Logs to a json file via nlohmann library 
void TrackerModule::SaveToJson()
{
	//Get path to solution directory
	std::filesystem::path solutionRoot = std::filesystem::current_path();
	std::string fileName = "usage_" + Clock() + ".json";
	std::cout << solutionRoot << std::endl;

	//Path to logs folder
	std::filesystem::path logPath = solutionRoot / "Logs" / (fileName);
	this->m_LogPath = logPath.string();
	nlohmann::json jsonLogFile;


	//Create logs directory if none exists
	if (!std::filesystem::exists((solutionRoot / "Logs")))
		std::filesystem::create_directory(solutionRoot / "Logs");


	//Load existing file if exists
	if (std::filesystem::exists(logPath))
	{
		std::ifstream inFile(logPath);
		try
		{
			inFile >> jsonLogFile;
		}
		catch (const nlohmann::json::parse_error& e)
		{
			std::cerr << "JSON PARSE ERROR:" << e.what() << std::endl;
			jsonLogFile = nlohmann::json::object();
		}
		inFile.close();
	}
	else //Create one if it log file doesn't exist
	{
		std::ofstream outFile(logPath);
		outFile << jsonLogFile;
		outFile.close();
	}
	
	//Loop through objects in json file, if they exists, update total duration
	//if they dont exists, create a new entry
	for (const auto& [m_AppKey, m_Duration] : m_ApplicationList)
	{
		if (jsonLogFile.contains(m_AppKey))
			jsonLogFile[m_AppKey]["Total Duration"] = jsonLogFile[m_AppKey]["Total Duration"].get<long>() + m_Duration;
		else
			jsonLogFile[m_AppKey] = { {"Application", m_AppKey }, { "Total Duration", m_Duration }};
	}


	//Dump to output file, then clear the current json file, and close the output file
	std::ofstream outFile(logPath);
	m_ApplicationList.clear();
	outFile << jsonLogFile.dump(4);
	jsonLogFile.clear();
	outFile.close();
	std::cout << "Saved to JSON" << std::endl;

}

//Automatic saver, 15 second timer
void TrackerModule::PeriodicSaver()
{
	std::thread periodicSave([this]()
	{
		while (m_IsRunning)
		{
			std::this_thread::sleep_for(std::chrono::seconds(15));
			std::lock_guard<std::mutex> lock(m_TrackerMutex);
			
			SaveToJson();
			ResetTimer();
		}
	});
	periodicSave.detach();
}

void TrackerModule::WindowChanges()
{
	this->m_IsRunning = true;
	this->m_PreviousWindow = GetForegroundWindow();
	this->m_CurrentApplication = GetExecutableName();
	this->m_StartTime = std::chrono::high_resolution_clock::now();

	std::thread WindowChangeTrack([this]()
	{
		while (this->m_IsRunning)
		{
			this->m_ForegroundWindow = GetForegroundWindow();
			if (this->m_ForegroundWindow != this->m_PreviousWindow && m_ForegroundWindow != nullptr)
			{
				std::string previousApp = m_CurrentApplication;
				long duration = GetWindowDuration();
				m_AppKey = GetExecutableName();

				if (!previousApp.empty())
				{
					std::lock_guard<std::mutex> lock(m_TrackerMutex);
					m_ApplicationList[previousApp] += duration;
				}

				Refresh();
				SynchronizeWindows(this->m_ForegroundWindow);
				ResetTimer();
				SaveToJson();
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	});
	WindowChangeTrack.detach();
}
//***************************
//Timing 
//***************************

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

//Resets the start time 
void TrackerModule::ResetTimer()
{
	this->m_StartTime = std::chrono::high_resolution_clock::now();
}

long TrackerModule::GetWindowDuration()
{
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::seconds>(endTime - m_StartTime).count();
	this->m_TotalDuration = duration;
	return this->m_TotalDuration;
}


