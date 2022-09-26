#pragma once

#include "Engine/Core/Timestep.h"
#include "Engine/Core/Events/Event.h"

namespace Engine {

	class Layer
	{
	public:
		Layer(const std::string& name = "Layer")
			: m_Name(name) {}
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const;

	protected:
		std::string m_Name;
	};

	const std::string& Layer::GetName() const
	{
		return m_Name;
	}

}