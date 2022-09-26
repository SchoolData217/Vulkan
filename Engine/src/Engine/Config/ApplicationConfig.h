#pragma once

namespace Engine {

	struct ApplicationConfig
	{
		const char* Name = "Application";
		unsigned int WindowWidth = 1920, WindowHeight = 1080;
		bool VSync = true;
		bool Fullscreen = false;
		bool Maximized = false;
		bool WindowDecorated = true;
		bool Resizable = true;
		bool EnableImGui = true;
	};

}