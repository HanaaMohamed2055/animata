#pragma once

class Application
{
private:
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

public:
    inline Application() {}
    inline virtual ~Application() {}

    inline virtual void Initialize() {}
    inline virtual void Update(float deltaTime) {}
    inline virtual void Render(float aspectRatio) {}
    inline virtual void Shutdown() {}
};