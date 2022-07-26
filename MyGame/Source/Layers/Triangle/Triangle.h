#pragma once

#include "../../Core/Layer.h"

#include "../../Events/AppEvent.h"
#include "../../Events/EventCodes/KeyCodes.h"
#include "../../Events/EventCodes/MouseCodes.h"

#include "../../Core/Time.h"

namespace MyGame
{
	class TriangleLayer : public Layer
	{
	public:
		TriangleLayer();
		~TriangleLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(Timestep) override;
		virtual void OnEvent(Event& e) override;

		void BlockEvents(bool block) { m_BlockEvents = block; }

	private:
		bool m_BlockEvents = true;
	};
}
