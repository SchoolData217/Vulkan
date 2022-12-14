VULKAN_SDK = os.getenv("VULKAN_SDK")

IncludeDir = {}
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

IncludeDir["GLFW"] = "%{wks.location}/Engine/vendor/GLFW/include"
IncludeDir["ImGui"] = "%{wks.location}/Engine/vendor/imgui"
IncludeDir["ImGuizmo"] = "%{wks.location}/Engine/vendor/ImGuizmo"
IncludeDir["glm"] = "%{wks.location}/Engine/vendor/glm"
IncludeDir["stb"] = "%{wks.location}/Engine/vendor/stb/include"
IncludeDir["assimp"] = "%{wks.location}/Engine/vendor/assimp/include"
IncludeDir["entt"] = "%{wks.location}/Engine/vendor/entt/include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"

Library["assimp_Debug"] = "%{wks.location}/Engine/vendor/assimp/lib/Debug/assimp-vc141-mtd.lib"
Library["assimp_Release"] = "%{wks.location}/Engine/vendor/assimp/lib/Release/assimp-vc141-mt.lib"

Binaries = {}
Binaries["assimp_Debug"] = "%{wks.location}/Engine/vendor/assimp/lib/Debug/assimp-vc141-mtd.dll"
Binaries["assimp_Release"] = "%{wks.location}/Engine/vendor/assimp/lib/Release/assimp-vc141-mt.dll"