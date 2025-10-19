#pragma once
#include "Window.h"
#include <cstdint>
#include <memory>
#include "VulkanContext.h"

class Application
{
public:
	Application() = default;
	~Application();           

	bool init();
	void run(); 
	void shutdown();
private: 
	Window _window;
    std::unique_ptr<VulkanContext> _vk;
};

