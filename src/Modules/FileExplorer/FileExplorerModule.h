#pragma once
#include "Core/IModule.h"
#include "imgui.h"

#include <string>
#include <vector>

class FileExplorerModule : public IModule
{
private:
	ImGuiWindowFlags m_Flags = ImGuiWindowFlags_None;

	//Current directory context
	std::string m_CurrentPath;
	std::string m_SearchQuery;

	//Directory contents
	struct FileEntry
	{
		std::string ms_Name;
		bool ms_IsDirectory;
		uint64_t ms_Size;
	};

	std::vector<FileEntry> m_Entries;

	//Drives
	std::vector<std::string> m_Drives;

public:

	//Constructors
	FileExplorerModule();
	~FileExplorerModule();

	//Lifecycle
	void Init() override;
	void Update(float dt) override;
	void Render() override;
	void Shutdown() override;

	const char* GetName() const override { return "File Explorer"; }


private:

	void LoadDrives();
	void LoadDirectory(const std::string& path);
	void ChangeDirectory(const std::string& path);
};

