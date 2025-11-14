#include "FileExplorerModule.h"

#include <windows.h>
#include <filesystem>

//--------------------
// Constructor / Destructor
//--------------------
FileExplorerModule::FileExplorerModule()
{

}

FileExplorerModule::~FileExplorerModule()
{

}


//--------------------
// Lifecycle
//--------------------
void FileExplorerModule::Init()
{
	LoadDrives();
	if (!m_Drives.empty())
		ChangeDirectory(m_Drives[0]);
}

void FileExplorerModule::Update(float dt)
{

}

void FileExplorerModule::Render()
{
	ImGui::Begin("File Explorer", nullptr, m_Flags);
	
	//Drive Selection
	if (ImGui::BeginCombo("Drive", m_CurrentPath.c_str()))
	{
		for (auto& drive : m_Drives)
		{
			bool selected = (drive == m_CurrentPath);
			if (ImGui::Selectable(drive.c_str(), selected))
				ChangeDirectory(drive);

			if (selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	//Search Bar
	//ImGui::InputText("Search", &m_SearchQuery, &m_SearchQuery);

	ImGui::Separator();

	//Directory Listings
	if (ImGui::BeginTable("FileTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("Name");
		ImGui::TableSetupColumn("Type");
		ImGui::TableSetupColumn("Size");
		ImGui::TableHeadersRow();

		//Parent directory
		if (m_CurrentPath != m_Drives[0])
		{
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			if (ImGui::Selectable("..", false))
				ChangeDirectory(std::filesystem::path(m_CurrentPath).parent_path().string());

			ImGui::TableSetColumnIndex(1);
			ImGui::Text("Directory");
			ImGui::TableSetColumnIndex(2);
			ImGui::Text("-");
		}


		//Entries
		for (auto& entry : m_Entries)
		{
			if (!m_SearchQuery.empty())
			{
				if (entry.ms_Name.find(m_SearchQuery) == std::string::npos)
					continue;
			}

			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			if (ImGui::Selectable(entry.ms_Name.c_str(), false))
			{
				if (entry.ms_IsDirectory)
					ChangeDirectory(m_CurrentPath + "\\" + entry.ms_Name);
			}

			ImGui::TableSetColumnIndex(1);
			ImGui::Text(entry.ms_IsDirectory ? "Directory" : "File");

			ImGui::TableSetColumnIndex(2);
			if (entry.ms_IsDirectory)
				ImGui::Text("-");
			else
				ImGui::Text("%llu bytes", entry.ms_Size);
		}
		ImGui::EndTable();
	}

	ImGui::End();

}

void FileExplorerModule::Shutdown()
{

}

//-------------------------
// Helpers
//-------------------------

void FileExplorerModule::LoadDrives()
{
	m_Drives.clear();

	DWORD mask = GetLogicalDrives();
	for (char letter = 'A'; letter <= 'Z'; ++letter)
	{
		if (mask & (1 << (letter - 'A')))
		{
			std::string drive = std::string(1, letter) + ":\\";
			m_Drives.push_back(drive);
		}
	}
}

void FileExplorerModule::LoadDirectory(const std::string& path)
{
	m_Entries.clear();

	for (auto& p : std::filesystem::directory_iterator(path))
	{
		FileEntry e;
		e.ms_Name = p.path().filename().string();
		e.ms_IsDirectory = p.is_directory();
		e.ms_Size = e.ms_IsDirectory ? 0 : p.file_size();

		m_Entries.push_back(e);
	}
}

void FileExplorerModule::ChangeDirectory(const std::string& path)
{
	m_CurrentPath = path;
	LoadDirectory(path);
}

