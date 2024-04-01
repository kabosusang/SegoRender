#pragma once
#include "Core/Event/Event.h"
#include "Core/Base/Timestep.hpp"

namespace Sego{

	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(Timestep ts) {}
		virtual void OnImGuiRender(){}
		virtual void OnEvent(Event& event) {}
		virtual void OnResize(){}

		const std::string& GetName() const { return m_DebugName; }
	protected:
		std::string m_DebugName;
		
	};

}