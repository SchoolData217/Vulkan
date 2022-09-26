include "Dependencies.lua"

workspace "Solusion"
	architecture "x64"
	
	configurations 
	{ 
		"Debug", 
        "Release",
    }

    flags { "MultiProcessorCompile" }

	startproject "Sandbox"
    
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
include "Engine/vendor/GLFW"
include "Engine/vendor/ImGui"
group ""

project "Engine"
    location "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "epch.h"
    pchsource "Engine/src/epch.cpp"

	files 
	{ 
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.c", 
		"%{prj.name}/src/**.hpp", 
		"%{prj.name}/src/**.cpp",
        "%{IncludeDir.stb}/**.h",
		"%{IncludeDir.stb}/**.cpp",

		"%{IncludeDir.ImGuizmo}/ImGuizmo.h",
		"%{IncludeDir.ImGuizmo}/ImGuizmo.cpp",
    }

    includedirs
	{
		"%{prj.name}/src",
        "%{prj.name}/vendor",

        "%{IncludeDir.VulkanSDK}",

        "%{IncludeDir.GLFW}",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.stb}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.entt}",
    }
    
    links 
	{
        "%{Library.Vulkan}",

        "GLFW",
        "ImGui",
    }

    filter "files:Engine/vendor/ImGuizmo/**.cpp"
        flags { "NoPCH" }

	filter "system:windows"
        systemversion "latest"
        
		defines 
		{ 
            "HZ_PLATFORM_WINDOWS",
		}

    filter "configurations:Debug"
        defines {"DEBUG", "_DEBUG", "HZ_DEBUG"}
        symbols "on"
                
    filter "configurations:Release"
        defines {"NDEBUG", "_NDEBUG", "HZ_RELEASE"}
        symbols "off" 
        optimize "on"

project "Sandbox"
    location "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"
    
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	links 
	{ 
        "Engine"
    }
    
	files 
	{ 
		"%{prj.name}/src/**.h", 
		"%{prj.name}/src/**.c", 
		"%{prj.name}/src/**.hpp", 
		"%{prj.name}/src/**.cpp" 
	}
    
	includedirs 
	{
        "%{prj.name}/src",
        "Engine/src",
        "Engine/vendor",
        "%{IncludeDir.ImGui}",
        "%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.glm}",
        "%{IncludeDir.entt}",
    }

	filter "system:windows"
        systemversion "latest"
                
		defines 
		{ 
            "HZ_PLATFORM_WINDOWS"
		}
    
    filter "configurations:Debug"
        defines {"DEBUG", "_DEBUG"}
        symbols "on"

		links
		{
            --"%{Library.assimp_Debug}",
		}

        postbuildcommands 
	    {
		    --'{COPY} "%{Binaries.assimp_Debug}" "%{cfg.targetdir}"',
	    }
                
    filter "configurations:Release"
        defines {"NDEBUG", "_NDEBUG"}
        symbols "off"
        optimize "on"

		links
		{
            --"%{Library.assimp_Release}",
		}

        postbuildcommands 
	    {
		    --'{COPY} "%{Binaries.assimp_Release}" "%{cfg.targetdir}"',
            '{COPY} "Resources" "%{cfg.targetdir}/Resources"',
	    }