#include "Application.h"
#include "VulkanContext.h"
#include <chrono>
#include <thread>
#include <cstdio>

Application::~Application() = default;

bool Application::init()
{
	Window::Desc windowDescriptor; 
	windowDescriptor.width = 1280;
	windowDescriptor.height = 720;
	windowDescriptor.title = "Somine Engine V";
	if (!_window.create(windowDescriptor))
	{
		std::fprintf(stderr, "[App] Window creation failed \n");
		return false;
	}

	_vk = std::make_unique<VulkanContext>();
	VulkanContext::CreateInfo ci;
	ci.window = &_window;
	ci.enableValidation = true; 
	ci.appName = "SomineEngineV";
	if (!_vk->init(ci)) {
		std::fprintf(stderr, "[App] VulkanContext init failed\n");
		return false;
	}
	return true;
}
void Application::run()
{
	using clock = std::chrono::steady_clock;
	auto last = clock::now(); 

	while (!_window.shouldClose())
	{
		auto now = clock::now();
		float dt = std::chrono::duration<float>(now - last).count();
		last = now; 

		//scene update, renderer begin frame, editor draw ui , renderer end frame ... 
		_window.pollEvents();

		//throttle while nothing's drawing
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

}
void Application::shutdown()
{
	if(_vk)
	{
		_vk->shutdown();
		_vk.reset();
	}
}