#include "Sego.hpp"
#include "EditorLayer.h"
#include "imgui.h"
#include <imgui_impl_vulkan.h>
#include <imgui_impl_sdl2.h>

#include "Core/Scene/SceneSerializer.hpp"
#include "platform/Utils/PlatformUtils.h"

#include <ImGuizmo.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Math/Math.h"


namespace Sego{

void EditorLayer::OnAttach(){
	
	m_Renderer = VulkanContext::Instance().GetRenderer();
	//Color
	m_Cts = Vulkantool::createSample(vk::Filter::eLinear,vk::Filter::eLinear,1,
	vk::SamplerAddressMode::eRepeat,vk::SamplerAddressMode::eRepeat,vk::SamplerAddressMode::eRepeat);

	m_color_texture_set = ImGui_ImplVulkan_AddTexture(m_Cts,m_Renderer->GetColorImageView(),
	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	//Depth
	m_Dts = Vulkantool::createSample(vk::Filter::eLinear,vk::Filter::eLinear,1,
	vk::SamplerAddressMode::eRepeat,vk::SamplerAddressMode::eRepeat,vk::SamplerAddressMode::eRepeat);

	m_depth_texture_set = ImGui_ImplVulkan_AddTexture(m_Dts,m_Renderer->GetDepthImageView(),
	VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);


	m_ActiveScene = std::make_shared<Scene>();
	
	m_EditorCamera = EditorCamera(30.0f,1.778f,0.1f,1000.0f);

#if 0
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
#endif

	m_SceneHierarchyPanel.SetContext(m_ActiveScene);
	
}

void EditorLayer::OnDetach(){
	ImGui_ImplVulkan_RemoveTexture(m_color_texture_set); //remove old texture
}

void EditorLayer::OnUpdate(Timestep ts){
	//Render
	m_Renderer->SetClearColor({0.1f,0.1f,0.1f,1.0f});
	if (m_ViewportFocused){
	}
	m_EditorCamera.OnUpdate(ts);
	m_ActiveScene->OnUpdateEditor(ts,m_EditorCamera);
	//m_ActiveScene->OnUpdateRuntime(ts);
	
	auto[mx,my] = ImGui::GetMousePos();
	mx -= m_ViewportBounds[0].x;
	my -= m_ViewportBounds[0].y;
	glm::vec2 viewportsize = m_ViewportBounds[1] - m_ViewportBounds[0];

	int mouseX = (int)mx;
	int mouseY = (int)my;

	if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportsize.x && mouseY < (int)viewportsize.y){
		int id = static_cast<int>(m_Renderer->ReadPixel(mouseX,mouseY));
		
		if (id < static_cast<int>(m_ActiveScene->GetRegistry().view<entt::entity>().size_hint())){
			m_HoveredEntity = id == -1 ? Entity() : Entity{(entt::entity)id,m_ActiveScene.get()};
		}
	}
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
				if (ImGui::MenuItem("New","Ctrl+N")){
					NewScene();
				}

				if (ImGui::MenuItem("Open...","Ctrl+O")){
						OpenScene();

				}
				if (ImGui::MenuItem("Save AS...","Ctrl+Shift+S")){
					SaveSceneAs();
				}

				if (ImGui::MenuItem("Exit")) SegEngine::Instance().Close();
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	//Scene Hierarchy
	m_SceneHierarchyPanel.OnImGuiRender();
	m_ContentBrowsPanel.OnImGuiRender();

	ImGui::Begin("Stats");
	std::string name = "None";
	if (m_HoveredEntity){
		name = m_HoveredEntity.GetComponent<TagComponent>().Tag;
	}
	ImGui::Text("Hovered Entity: %s",name.c_str());
	ImGui::End();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Viewport");
	auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
	auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
	auto viewportOffset = ImGui::GetWindowPos();
	m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
	m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

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

	// Gizmos
	Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
	if (selectedEntity && m_GizmoType != -1){
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		float windowWidth = (float)ImGui::GetWindowWidth();
		float windowHeight = (float)ImGui::GetWindowHeight();
		ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);
		
		//Camera
		//Runtime camera from entity
		/* auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
		const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
		const glm::mat4 cameraProjection = camera.GetProjection();
		glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());
 		*/

		//Editor Camera
		const glm::mat4& cameraProjection = m_EditorCamera.GetProjection();
		const glm::mat4& cameraView = m_EditorCamera.GetViewMatrix();
		

		//Entity transform
		auto& tc = selectedEntity.GetComponent<TransformComponent>();
		glm::mat4 transform = tc.GetTransform();

		// Snapping
		bool snap = Input::ISKeyPressed(KeySanCode::LCTRL);
		float snapValue = 0.5f;

		//Snap to 45 degrees for rotation
		if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
			snapValue = 45.0f;
		float snapValues[3] = {snapValue,snapValue,snapValue };


		ImGuizmo::Manipulate(glm::value_ptr(cameraView),glm::value_ptr(cameraProjection),
		(ImGuizmo::OPERATION)m_GizmoType,ImGuizmo::MODE::LOCAL,glm::value_ptr(transform),
		nullptr,snap ? snapValues : nullptr);

		if (ImGuizmo::IsUsing()){
			glm::vec3 translation,rotation,scale;
			Math::DecomposeTransform(transform,translation,rotation,scale);

			glm::vec3 deltaRotation =rotation- tc.Rotation;
			tc.Translation = translation;
			tc.Rotation += deltaRotation;
			tc.Scale = scale;
		}
		//glm::decompose

	}

	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::End();
}

void EditorLayer::FramBufferResize(float w,float h){
	uint32_t new_width = static_cast<uint32_t>(w);
	uint32_t new_height = static_cast<uint32_t>(h);

	m_Renderer->resizeframbuffer(new_width,new_height);
	m_ActiveScene->OnViewportResize(new_width,new_height);
	m_EditorCamera.SetViewportSize(w,h);

	//Color
	if (m_color_texture_set != VK_NULL_HANDLE)
		ImGui_ImplVulkan_RemoveTexture(m_color_texture_set); //remove old texture
	
	m_color_texture_set = ImGui_ImplVulkan_AddTexture(m_Cts,m_Renderer->GetColorImageView(),
	VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	//Depth
	if (m_depth_texture_set != VK_NULL_HANDLE)
	ImGui_ImplVulkan_RemoveTexture(m_depth_texture_set); //remove old texture

	m_depth_texture_set = ImGui_ImplVulkan_AddTexture(m_Dts,m_Renderer->GetDepthImageView(),
	VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

}

void EditorLayer::OnEvent(Event &e){
	m_EditorCamera.OnEvent(e); //EditorCamera Pull Events

	EventDispatcher dispatcher(e);
	dispatcher.Dispatch<KeyPressedEvent>(SG_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
	dispatcher.Dispatch<MouseButtonPressedEvent>(SG_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
}

bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e){
	if (e.GetMouseButton() == Mouse::ButtonLeft && !ImGuizmo::IsOver() && !Input::ISKeyPressed(KeySanCode::LALT)){

		if (m_ViewportHovered){
			m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
		}
	}
	return false;
}

bool EditorLayer::OnKeyPressed(KeyPressedEvent& e){
	
	if (e.IsRepeat() > 0)
		return false;
	bool control = Input::ISKeyPressed(KeySanCode::LCTRL) || Input::ISKeyPressed(KeySanCode::RCTRL);
	bool shit = Input::ISKeyPressed(KeySanCode::LSHIFT) || Input::ISKeyPressed(KeySanCode::RSHIFT);

	switch (e.GetKeyCode()){
		case KeySanCode::N:{
			if (control)
				NewScene();

			break;
		}
		case KeySanCode::O:{
			if (control)
				OpenScene();

			break;
		}
		case KeySanCode::S:{
			if (control && shit)
				SaveSceneAs();

			break;
		}


		//Gizmos
		case KeySanCode::Q :
			m_GizmoType = -1;
			break;
		case KeySanCode::W :
			m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;
		case KeySanCode::E :
			m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			break;
		case KeySanCode::R :
			m_GizmoType = ImGuizmo::OPERATION::SCALE;
			break;

	}



	return true;
}

void EditorLayer::NewScene(){
	m_ActiveScene = std::make_shared<Scene>();
	m_ActiveScene->OnViewportResize(m_viewportsize.x,m_viewportsize.y);
	m_SceneHierarchyPanel.SetContext(m_ActiveScene);

}
void EditorLayer::OpenScene(){
	std::string filepath = FileDialogs::OpenFile("Sego Scene(*.Sego)\0*.sego\0");
	if(!filepath.empty()){
		m_ActiveScene = std::make_shared<Scene>();
		m_ActiveScene->OnViewportResize(m_viewportsize.x,m_viewportsize.y);
		m_SceneHierarchyPanel.SetContext(m_ActiveScene);

		SceneSerializer serializer(m_ActiveScene);
		serializer.Deserialize(filepath);
	}
}
void EditorLayer::SaveSceneAs(){
	std::string filepath = FileDialogs::SaveFile("Sego Scene(*.Sego)\0*.sego\0");
	if(!filepath.empty()){
		SceneSerializer serializer(m_ActiveScene);
		serializer.Serialize(filepath);
	}
}


}
