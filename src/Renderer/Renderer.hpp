#pragma once

#include "Core/Window.hpp"

namespace Renderer {

    class Renderer
    {
    public:
        Renderer(const std::shared_ptr<Window>& window);
        ~Renderer();

        void Resize();

        void BeginFrame();
        void EndFrame();

        void Draw();

    private:
        std::shared_ptr<Window> m_Window;
    };

}
