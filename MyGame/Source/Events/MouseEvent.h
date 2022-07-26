#pragma once

#include "../Events/Event.h"
#include "../Events/EventCodes/MouseCodes.h"

namespace MyGame
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(const float x, const float y) : m_MouseX(x), m_MouseY(y) {}

		float GetX() const { return m_MouseX; }
		float GetY() const { return m_MouseY; }

		std::string ToString() const override { return "MouseMovedEvent: " + std::to_string(m_MouseX) + ", " + std::to_string(m_MouseY); }

		EVENT_CLASS_TYPE(MouseMoved) EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_MouseX, m_MouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(const float xOffset, const float yOffset) : m_XOffset(xOffset), m_YOffset(yOffset) {}

		float GetXOffset() const { return m_XOffset; }
		float GetYOffset() const { return m_YOffset; }

		std::string ToString() const override { return   "MouseScrolledEvent: " + std::to_string(GetXOffset()) + ", " + std::to_string(GetYOffset()); }

		EVENT_CLASS_TYPE(MouseScrolled)	EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput)

	private:
		float m_XOffset, m_YOffset;
	};

	class MouseButtonEvent : public Event
	{
	public:
		int GetMouseButton() const { return m_Button; }

		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryInput | EventCategoryMouseButton)

	protected:
		MouseButtonEvent(const int button) : m_Button(button) {}
		int m_Button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(const int button) : MouseButtonEvent(button) {}

		std::string ToString() const override { return "MouseButtonPressedEvent: " + std::to_string(m_Button); }

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(const int button) : MouseButtonEvent(button) {}

		std::string ToString() const override { return "MouseButtonReleasedEvent: " + std::to_string(m_Button); }

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}