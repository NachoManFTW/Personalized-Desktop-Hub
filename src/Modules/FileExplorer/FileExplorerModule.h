#pragma once
#include "Core/IModule.h"
#include <imgui.h>

class FileExplorerModule : public IModule
{
private:
	ImGuiWindowFlags flags = ImGuiWindowFlags_None;

public:
	void Init() override {};
	void Update(float dt) override {};
	void Shutdown() override {};

	void Render() override
	{
		ImGui::Begin("File Explorer", nullptr, flags);
		ImGui::Text("File Explorer system is active.");
		ImGui::End();
	}

	const char* GetName() const override { return "File Explorer"; }
};

