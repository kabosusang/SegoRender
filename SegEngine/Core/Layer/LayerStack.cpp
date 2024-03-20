#include "pch.h"
#include "LayerStack.h"

namespace Sego{

    LayerStack::LayerStack()
    {
    }

    LayerStack::~LayerStack()
    {
        for (Layer* layer : layers_)
            delete layer;
    }

    void LayerStack::PushLayer(Layer* layer)
    {
        LayerInsert_ = layers_.emplace(LayerInsert_, layer);
    }

    void LayerStack::PushOverlay(Layer* overlay)
    {
        layers_.emplace_back(overlay);
    }

    void LayerStack::PopLayer(Layer* layer)
    {
        auto it = std::find(layers_.begin(), layers_.end(), layer);
        if (it != layers_.end())
        {
            layers_.erase(it);
            LayerInsert_--;
        }
    }

    void LayerStack::PopOverlay(Layer* overlay)
    {
        auto it = std::find(layers_.begin(), layers_.end(), overlay);
        if (it != layers_.end())
            layers_.erase(it);
    }








}