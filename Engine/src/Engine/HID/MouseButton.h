#pragma once

namespace Engine {

	enum class MouseButton
	{
		None = -1,
		Left = 0,
		Right = 1,
		Middle = 2,

		Max
	};

	enum class ButtonState
	{
		None = -1,
		Pressed,
		Hold,
		Released
	};

	enum class CursorMode
	{
		Normal = 0,
		Hidden = 1,
		Locked = 2
	};

}