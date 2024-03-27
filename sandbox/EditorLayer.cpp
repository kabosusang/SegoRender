#include "EditorLayer.h"
#include "imgui.h"
#include <imgui_impl_vulkan.h>
#include <imgui_impl_sdl2.h>

namespace Sego{


void EditorLayer::OnAttach(){
	m_Cts = Vulkantool::createSample(vk::Filter::eLinear,vk::Filter::eLinear,1,
	vk::SamplerAddressMode::eRepeat,vk::SamplerAddressMode::eRepeat,vk::SamplerAddressMode::eRepeat);

	m_color_texture_set = ImGui_ImplVulkan_AddTexture(m_Cts,VulkanRhi::Instance().getColorImageView(),
	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void EditorLayer::OnDetach(){

}

void EditorLayer::OnUpdate(){

}

void EditorLayer::OnImGuiRender(){
    static bool dockingEnabled = true;
	if (dockingEnabled)
	{
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive, 
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise 
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows, 
				// which we can't undo at the moment without finer window depth/z control.
				//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

				if (ImGui::MenuItem("Exit")) SegEngine::Instance().Close();
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

        ImGui::Begin("Viewport");
        ImVec2 ViewportPanelSize = ImGui::GetContentRegionAvail();
		m_viewportsize = {ViewportPanelSize.x,ViewportPanelSize.y};
		if(m_viewportsize != *((glm::vec2*)&ViewportPanelSize)){
			VulkanRhi::Instance().resizeframbuffer(ViewportPanelSize.x,ViewportPanelSize.y);
			m_viewportsize = {ViewportPanelSize.x,ViewportPanelSize.y};
			m_color_texture_set = ImGui_ImplVulkan_AddTexture(m_Cts,VulkanRhi::Instance().getColorImageView(),
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
        ImGui::Image(m_color_texture_set,ViewportPanelSize);
		ImGui::End();

		ImGui::End();
   
	}
	else
	{
		ImGui::Begin("Hello, world!");
        ImGui::Text("This is some useful text.");
        ImGui::End();
	}
}

void EditorLayer::OnResize(){

}

void EditorLayer::OnEvent(Event &e){

}




}


