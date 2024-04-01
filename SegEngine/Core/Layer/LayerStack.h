#pragma once

#include <vector>
#include "Layer.h"

namespace Sego{

    class LayerStack{

    public:
        LayerStack();
        ~LayerStack();

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);
        void PopLayer(Layer* layer);
        void PopOverlay(Layer* overlay);

        std::vector<Layer*>::iterator begin() { return layers_.begin(); }
        std::vector<Layer*>::iterator end() { return layers_.end(); }
        std::vector<Layer*>::reverse_iterator rbegin() { return layers_.rbegin(); }
        std::vector<Layer*>::reverse_iterator rend() { return layers_.rend(); }

        



    private:
        std::vector<Layer*> layers_;
        uint32_t LayerInsertIndex_ = 0;

    };






}