workspace "DesktopHub"
	architecture "x64"
	configurations { "Debug", "Release", "Dist" }
	startproject "Core"

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


project "DesktopHub"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ( "%{wks.location}/bin/Intermediates-" .. outputdir .. "/%{prj.name}")

	files {
	"src/**.h", 
	"src/**.cpp",

	--ImGui 
	"ThirdParty/imgui/*.cpp",
	"ThirdParty/imgui/*.h",
	"ThirdParty/imgui/backends/imgui_impl_glfw.cpp",
	"ThirdParty/imgui/backends/imgui_impl_opengl3.cpp",

	--GLFW
	"ThirdParty/glfw/src/*.c",

	--GLAD
	"ThirdParty/glad/src/*.c",

	--Nlohmann Json
	"ThirdParty/json/*.hpp"
	}

	includedirs 
	{
		"src/",
		"ThirdParty/imgui",
		"ThirdParty/imgui,backends",
		"ThirdParty/glfw/include",
		"ThirdParty/glad/include",
		"ThirdParty/json"
	}

	filter "system:windows"
		systemversion "latest"
		defines { "_GLFW_WIN32", "PLATFORM_WINDOWS" }
		links { "opengl32", "gdi32", "user32", "shell32" }
		
	filter "system:linux"
		defines { "_GLFW_X11", "PLATFORM_LINUX"}
		links { "GL", "pthread", "dl" }

	filter "system:macosx"
		defines { "PLATFORM_MAC" }
		links { "Cocoa.framework", "IOKit.framework", "CoreVideo.framework" }

	filter "configurations:Debug"
		defines { "DEBUG" }
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines { "NDEBUG" }
		runtime "Release"
		optimize "On"
		symbols "On"

	filter "configurations:Dist"
		defines { "DIST" }
		runtime "Release"
		optimize "On"
		symbols "Off"



