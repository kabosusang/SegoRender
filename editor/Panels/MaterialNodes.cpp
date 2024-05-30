#include "MaterialNodes.hpp"
#include <imgui.h>


namespace Sego{
    enum class NodesType{
        BRDF,
        OutPut,
        Add
    };



    const int hardcoded_node_id = 1;
    void MaterialNodes::Init()
    {
        ImNodes::SetNodeGridSpacePos(1, ImVec2(200.0f, 200.0f));
    }
    
    void MaterialNodes::OnImGuiRender()
    {
        ImGui::Begin("node editor");
        //ColorSetting
      

        ImNodes::BeginNodeEditor();
        ImNodes::BeginNode(1);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("simple node :)");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginInputAttribute(2);
        ImGui::Text("input");
        ImNodes::EndInputAttribute();

        ImNodes::BeginOutputAttribute(3);
        ImGui::Indent(40);
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();

        ImNodes::MiniMap();
        ImNodes::EndNodeEditor();

        ImGui::End();
    }

}