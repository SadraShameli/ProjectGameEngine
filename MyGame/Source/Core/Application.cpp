#include "CommonHeaders.h"

#include "Application.h"
#include "Log.h"

#include "../Renderer/Renderer.h"
#include "../Events/AppEvent.h"

#include "../Debugs/DebugHelpers.h"
#include "../Debugs/Instrumentor.h"

namespace MyGame
{
	void Application::Init()
	{
		MYGAME_PROFILE_FUNCTION();

		m_Window = Window::Create(WindowProps());
		m_Window->SetEventCallback(MYGAME_BIND_EVENT_FN(Application::OnEvent));

		Renderer::Init();

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	}

	void Application::Destroy()
	{
		MYGAME_PROFILE_FUNCTION();

		//Renderer::Shutdown();
	}

	void Application::PushLayer(Layer* layer)
	{
		MYGAME_PROFILE_FUNCTION();

		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		MYGAME_PROFILE_FUNCTION();

		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}

	void Application::OnEvent(Event& e)
	{
		MYGAME_PROFILE_FUNCTION();

		MYGAME_INFO_EVENTS(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowCloseEvent>(MYGAME_BIND_EVENT_FN(Application::OnWindowClose));
		dispatcher.Dispatch<WindowResizeEvent>(MYGAME_BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack.end(); --it != m_LayerStack.begin();)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}
	}

	void Application::Run()
	{
		MYGAME_PROFILE_FUNCTION();

		while (m_Running)
		{
			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrameTime;
			m_LastFrameTime = time;

			if (!m_Minimized)
			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(timestep);

				m_ImGuiLayer->Begin();
				for (Layer* layer : m_LayerStack)
					layer->OnImGuiRender();
				m_ImGuiLayer->End();
			}

			m_Window->OnUpdate();
		}
	}

	bool Application::OnWindowClose(WindowCloseEvent& e)
	{
		m_Running = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		MYGAME_PROFILE_FUNCTION();

		if (e.GetWidth() == 0 || e.GetHeight() == 0)
		{
			m_Minimized = true;
			return false;
		}

		m_Minimized = false;
		Renderer::OnWindowResize(e.GetWidth(), e.GetHeight());

		return false;
	}

	void Application::Close() { m_Running = false; }

	Application application;
}

int main()
{
	MyGame::Log::Init();
	MYGAME_INFO("Welcome to MyGame!");

	// Application lifetime
	MyGame::application.Init();
	MyGame::application.Run();
	MyGame::application.Destroy();

	return 0;
}