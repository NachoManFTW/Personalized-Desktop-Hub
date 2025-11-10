#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Window
{
private:
    GLFWwindow* m_Window; //Store GLFW window pointer

public:
    Window(int width, int height, const char* title);
    ~Window();

    void PollEvents(); //Handle input/events each frame
    void SwapBuffers(); //Swap the back and front buffers for rendering 
    bool ShouldClose() const; //Check if the window should be closed

    //Return raw GLFW pointer
    GLFWwindow* GetNativeWindow() const { return m_Window; }
};
