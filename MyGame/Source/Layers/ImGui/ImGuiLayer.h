#pragma once

#include "../../Core/Layer.h"

#include "../../Events/AppEvent.h"
#include "../../Events/EventCodes/KeyCodes.h"
#include "../../Events/EventCodes/MouseCodes.h"

namespace MyGame
{
	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;
		virtual void OnImGuiRender() override;

		void Begin();
		void End();

		void SetLightMode();
		void SetDarkMode();

		void BlockEvents(bool block) { m_BlockEvents = block; }

	private:
		bool m_BlockEvents = true;
	};
}
