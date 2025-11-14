#include "Window.h"
#include <stdexcept>

Window::Window(int width, int height, const char* title) 
{
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW");

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    m_Window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!m_Window)
        throw std::runtime_error("Failed to create GLFW window");

    glfwMakeContextCurrent(m_Window);
}

Window::~Window() 
{
    if (m_Window) glfwDestroyWindow(m_Window);
    glfwTerminate();
}

void Window::PollEvents() 
{ 
    glfwPollEvents(); 
}

void Window::SwapBuffers() 
{ 
    glfwSwapBuffers(m_Window); 
}

bool Window::ShouldClose() const 
{ 
    return glfwWindowShouldClose(m_Window); 
}

