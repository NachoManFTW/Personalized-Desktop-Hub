#pragma once

#include "Core/Window.h"
#include "Core/ImGuiLayer.h"
#include "Core/IModule.h"

#include <memory>
#include <vector>

class Application 
{
private:
    Window m_Window;
    ImGuiLayer m_ImGui;
    std::vector<std::shared_ptr<IModule>> m_Modules;
    
    void ModuleInit();
    void ModuleUpdate(float dt);
    void ModuleRender();
    void ModuleShutdown();

    void ClearScreen();




public:
    Application(int width, int height, const char* title);
    ~Application();

    void Run();
    void Shutdown();

    template<typename T, typename... Args>
    void AddModule(Args&&... args)
    {
        m_Modules.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }



};
