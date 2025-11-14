#include <imgui.h>
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "ImGuiLayer.h" 

#include <wtypes.h>


ImGuiLayer::ImGuiLayer(GLFWwindow* window) 
{
    //Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();


    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::GetStyle().WindowPadding = ImVec2(0.0f, 0.0f);

    //Setup style
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

ImGuiLayer::~ImGuiLayer() 
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiLayer::Begin()
{

    //Start new frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuiWindowFlags window_flags = 
        ImGuiWindowFlags_NoDocking | 
        ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus | 
        ImGuiWindowFlags_NoBackground;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

    ImGui::Begin("Dockspace", nullptr, window_flags);

    RenderMainMenubar();
    DraggableWindow();


    //Create dockspace
    ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
}

void ImGuiLayer::End()
{

    ImGui::End();

    ImGui::PopStyleVar(1);
    //Render everything that was drawn
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup);
    }
    
}


//--------------------
// Window Events
//--------------------
void ImGuiLayer::DraggableWindow()
{
    static bool dragging = false;
    static POINT clickOffset = { 0, 0 };
    HWND hwnd = GetActiveWindow();

    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 winPos = ImGui::GetWindowPos();
    bool isInTitleBar = (mousePos.y > winPos.y && mousePos.y < winPos.y + 15);

    if (isInTitleBar && ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemHovered())
    {
        dragging = true;

        POINT cursorPos;
        GetCursorPos(&cursorPos);

        RECT windowRect;
        GetWindowRect(hwnd, &windowRect);

        clickOffset.x = cursorPos.x - windowRect.left;
        clickOffset.y = cursorPos.y - windowRect.top;

        SetCapture(hwnd);
    }
    else if (!ImGui::IsMouseDown(0) && dragging)
    {
        dragging = false;
        ReleaseCapture();
    }

    if (dragging)
    {
        POINT cursorPos;
        GetCursorPos(&cursorPos);

        // Get screen work area (monitor minus taskbar)
        RECT screenRect;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &screenRect, 0);

        RECT windowRect;
        GetWindowRect(hwnd, &windowRect);
        int windowWidth = windowRect.right - windowRect.left;
        int windowHeight = windowRect.bottom - windowRect.top;

        // Desired new position
        int newX = cursorPos.x - clickOffset.x;
        int newY = cursorPos.y - clickOffset.y;

        SetWindowPos(hwnd, nullptr, newX, newY, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
}


//--------------------
// Rendering
//--------------------

void ImGuiLayer::RenderMainMenubar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Modules"))
        {
            ImGui::EndMenu();
        }

        if (ImGui::Button("-"))
        {
            HWND hwnd = GetActiveWindow();
            ShowWindow(hwnd, SW_MINIMIZE);
        }
        if (ImGui::Button("Exit"))
        {
            exit(EXIT_SUCCESS);
        }
        ImGui::EndMainMenuBar();
    }
}
