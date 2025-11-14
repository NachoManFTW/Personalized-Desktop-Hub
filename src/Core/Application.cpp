//Declaration
#include "Application.h"



//Inlcudes
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

//--------------------
// Application Lifecycle
//--------------------
void Application::Run()
{
    ModuleInit();

    while (!m_Window.ShouldClose()) 
    {
        m_Window.PollEvents();

        ClearScreen();

        m_ImGui.Begin();
        {
            ModuleRender();

        }
        m_ImGui.End();
        m_Window.SwapBuffers();
    }

    ModuleShutdown();
}

void Application::Shutdown()
{

}


//--------------------
// Module Lifecycle
//--------------------
void Application::ModuleInit()
{
    for (auto& module : m_Modules)
        module->Init();
}

void Application::ModuleUpdate(float dt)
{
    for (auto& module : m_Modules)
        module->Update(dt); 
}

void Application::ModuleRender()
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

void Application::ModuleShutdown()
{
    for (auto& module : m_Modules)
        module->Shutdown();
}



void Application::ClearScreen()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}






