#include "Application.h"

#include <imgui.h>
#include <iostream>
#include <stdexcept>
#include <unordered_map>


Application::Application(int width, int height, const char* title) : 
    m_Window(width, height, title),
    m_ImGui(m_Window.GetNativeWindow())
{
    // Initialize OpenGL loader (glad)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("Failed to initialize OpenGL context");
}

Application::~Application() 
{

}

//Run initialze functions on all modules
void Application::RunInit()
{
    for (auto& module : m_Modules)
        module->Init();
}
//Run update functions on all modules

void Application::RunUpdate(float dt)
{
    for (auto& module : m_Modules)
        module->Update(dt); 
}
//Run shutdown functions on all modules

void Application::RunShutdown()
{
    for (auto& module : m_Modules)
        module->Shutdown();
}

//Ability to toggle view of all functions
void Application::ToggleModules()
{
    static std::unordered_map<std::string, bool> visibility;
    ImGui::Begin("Modules");
    float dt = 0;

    for (auto& module : m_Modules)
    {
        const char* name = module->GetName();
        bool& visible = visibility[name];
        ImGui::Checkbox(name, &visible);
        if (visible)
        {
            module->Update(dt);
            module->Render();
        }
    }
    ImGui::End();
}

void Application::ClearScreen()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}


void Application::Run() 
{

    RunInit();


    while (!m_Window.ShouldClose()) {
        m_Window.PollEvents();

        ClearScreen();
       
        m_ImGui.Begin();
        {
            ToggleModules();

        }
        m_ImGui.End();
        m_Window.SwapBuffers();
    }

    RunShutdown();
}



