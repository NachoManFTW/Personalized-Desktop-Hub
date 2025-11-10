#pragma once
#include "Core/IModule.h"
#include <imgui.h>

class ConsoleModule : public IModule
{
private:
	ImGuiWindowFlags flags = ImGuiWindowFlags_None;

public:
	void Init() override {};
	void Update(float dt) override {};
	void Shutdown() override {};

	void Render() override
	{
		ImGui::Begin("Console", nullptr, flags);
		ImGui::Text("Console system is active");
		ImGui::End();
	}

	const char* GetName() const override { return "Console"; }
};

