#pragma once
#include "pch.h"

#include "Core/Event/Event.h"
const int DFWIDTH = 1280;
const int DFHEIGHT = 800;

namespace Sego{
    struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::string& title = "Sego Engine",
			        uint32_t width = DFWIDTH,
			        uint32_t height = DFHEIGHT)
			: Title(title), Width(width), Height(height)
		{
		}
	};

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() = default;

		virtual void OnUpdate() = 0;
		virtual void PollEvent() =0;
		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual bool IsWindowResize() = 0;

		virtual void* GetNativeWindow() const = 0;
		static Window* Create(const WindowProps& props = WindowProps());
	};



}