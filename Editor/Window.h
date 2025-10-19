#pragma once
#include <cstdint>
#include <string>

struct GLFWwindow;

class Window
{
public: 
	struct Desc {
		int width = 1280; 
		int height = 720; 
		std::string title = "SomineEngineV"; 
	};

	Window() = default;
	~Window();

	bool create(const Desc& descriptor); 
	void pollEvents(); 
	bool shouldClose() const;
	void requestClose(); 
	void* nativeHandle() const; //for Vulkan surfate based on platform 
	int width() const { return _width; }
	int height() const { return _height; }

private: 
	GLFWwindow* _handle = nullptr; 
	int _width = 0; 
	int _height = 0;
};

