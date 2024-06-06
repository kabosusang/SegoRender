#pragma once
#include "imnodes.h"
#include "example/graph.h"

namespace Sego{
enum class NodeType
{
    add,
    multiply,
    output,
    sine,
    time,
    value
};

struct IMNode
{
    NodeType type;
    float    value;

    explicit IMNode(const NodeType t) : type(t), value(0.f) {}

    IMNode(const NodeType t, const float v) : type(t), value(v) {}
};

    class MaterialNodes{
    public:
        MaterialNodes() : graph_(), nodes_(), root_node_id_(-1),
          minimap_location_(ImNodesMiniMapLocation_BottomRight)
        {
        }

        void Init();
        void OnImGuiRender();   

    private:
        enum class UiNodeType
        {
            add,
            multiply,
            output,
            sine,
            time,
            BRDF
        };

        struct UiNode
        {
            UiNodeType type;
            // The identifying id of the ui node. For add, multiply, sine, and time
            // this is the "operation" node id. The additional input nodes are
            // stored in the structs.
            int id;

            union
            {
                struct
                {
                    int lhs, rhs;
                } add;

                struct
                {
                    int lhs, rhs;
                } multiply;

                struct
                {
                    int r, g, b;
                } output;

                struct
                {
                    int input;
                } sine;

                
            } ui;
        };

        example::Graph<IMNode>            graph_;
        std::vector<UiNode>    nodes_;
        int                    root_node_id_;
        ImNodesMiniMapLocation minimap_location_;
    };



}
