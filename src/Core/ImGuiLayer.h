#pragma once
#include <GLFW/glfw3.h>
#include <imgui.h>

class ImGuiLayer
{
public:
    ImGuiLayer(GLFWwindow* window);
    ~ImGuiLayer();

    void Begin();
    void End();

};
