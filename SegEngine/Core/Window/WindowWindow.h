#pragma once

#include "Core/Window/Window.h"
#include "SDL.h"
#include "SDL_vulkan.h"
#include "Renderer/GraphicsContext.hpp"


namespace Sego{
class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps& props);
		virtual ~WindowsWindow();

		void OnUpdate() override;
		void PollEvent() override;
		unsigned int GetWidth() const override { return m_Data.Width; }
		unsigned int GetHeight() const override { return m_Data.Height; }

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		
		virtual void* GetNativeWindow() const { return window_; }

	private:
		virtual void Init(const WindowProps& props);
		virtual void Shutdown();
	private:
		 //SDL window
        SDL_Window* window_;
		GraphicsContext* context_;

		struct WindowData
		{
			std::string Title;
			unsigned int Width, Height;
			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
	};


}
