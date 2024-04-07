#include "Sego.hpp"
#include "EditorLayer.h"
#include "imgui.h"
#include <imgui_impl_vulkan.h>
#include <imgui_impl_sdl2.h>

#include "Core/Scene/SceneSerializer.hpp"

namespace Sego{


void EditorLayer::OnAttach(){
	
	m_Renderer = VulkanContext::Instance().GetRenderer();
	m_Cts = Vulkantool::createSample(vk::Filter::eLinear,vk::Filter::eLinear,1,
	vk::SamplerAddressMode::eRepeat,vk::SamplerAddressMode::eRepeat,vk::SamplerAddressMode::eRepeat);

	m_color_texture_set = ImGui_ImplVulkan_AddTexture(m_Cts,m_Renderer->GetColorImageView(),
	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	m_ActiveScene = std::make_shared<Scene>();
	//Entity
	auto redsquare = m_ActiveScene->CreateEntity("Rend Square Entity");
	redsquare.AddComponent<SpriteRendererComponent>(glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });

	auto greensquare = m_ActiveScene->CreateEntity("Green Square Entity");
	greensquare.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f });

	//Camera
	m_CameraEntity = m_ActiveScene->CreateEntity("Camera Entity");
	m_CameraEntity.AddComponent<CameraComponent>();

	//Vulkan Right hand coordinate system
	auto& translate = m_CameraEntity.GetComponent<TransformComponent>().Translation;
	translate.z = -6.0f;

	class CameraController : public ScriptableEntity
	{
	public:
		void OnCreate(){
			//GetComponent<TransformComponent>();
			std::cout << "CameraController::OnCrearte!"<<std::endl;
		}

		void OnDestroy(){

		}

		void OnUpdate(Timestep ts){
			auto& translation = GetComponent<TransformComponent>().Translation;
			float speed = 5.0f;
			
			if(Input::ISKeyPressed(KeySanCode::A))
				translation.x -= speed * ts;
			if(Input::ISKeyPressed(KeySanCode::D))
				translation.x += speed * ts;
			if(Input::ISKeyPressed(KeySanCode::W))
				translation.y += speed * ts;
			if(Input::ISKeyPressed(KeySanCode::S))
				translation.y -= speed * ts;

		}

	};

	m_CameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraController>();
	m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	SceneSerializer serializer(m_ActiveScene);
	serializer.Serialize("resources/scenes/Example.sego");

}

void EditorLayer::OnDetach(){
	ImGui_ImplVulkan_RemoveTexture(m_color_texture_set); //remove old texture
}

void EditorLayer::OnUpdate(Timestep ts){
	//Render
	m_Renderer->SetClearColor({0.1f,0.1f,0.1f,1.0f});
	m_ActiveScene->OnUpdate(ts);	

}

void EditorLayer::OnImGuiRender(){
// Note: Switch this to true to enable dockspace
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
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}
		style.WindowMinSize.x = minWinSizeX;

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
	//Scene Hierarchy
	m_SceneHierarchyPanel.OnImGuiRender();


	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Viewport");
	
	m_ViewportFocused = ImGui::IsWindowFocused();
	m_ViewportHovered = ImGui::IsWindowHovered();
	SegEngine::Instance().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);
	
	ImVec2 ViewportPanelSize = ImGui::GetContentRegionAvail();

	if(m_viewportsize != *((glm::vec2*)&ViewportPanelSize) && 
	ViewportPanelSize.x > 0 && ViewportPanelSize.y > 0){
		FramBufferResize(ViewportPanelSize.x,ViewportPanelSize.y);
		m_viewportsize = {ViewportPanelSize.x,ViewportPanelSize.y};
	}
	
	ImGui::Image(m_color_texture_set,ViewportPanelSize);
	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::End();


}

void EditorLayer::FramBufferResize(float w,float h){
	m_Renderer->resizeframbuffer(w,h);
	m_ActiveScene->OnViewportResize(w,h);

	if (m_color_texture_set != VK_NULL_HANDLE)
		ImGui_ImplVulkan_RemoveTexture(m_color_texture_set); //remove old texture
	
	m_color_texture_set = ImGui_ImplVulkan_AddTexture(m_Cts,m_Renderer->GetColorImageView(),
	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

}

void EditorLayer::OnEvent(Event &e){



}




}


