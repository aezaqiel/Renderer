#include "Core/Logger.hpp"
#include "Core/Application.hpp"

int main()
{
    Renderer::Logger::Init();

    {
        Renderer::Application app;
        app.Run();
    }

    Renderer::Logger::Shutdown();
}
