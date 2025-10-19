#include "Window.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <cstdio>

static void glfw_error_callback(int error, const char* description)
{
	std::fprintf(stderr, "[GLFW] Error %d : %s \n", error, description);
}

Window::~Window()
{
	if (_handle)
	{
		glfwDestroyWindow(_handle);
		_handle = nullptr; 
	}

	glfwTerminate(); 
}

bool Window::create(const Desc& descriptor)
{
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
	{
		std::fprintf(stderr, "[GLFW] init failed \n");
		return false;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	_handle = glfwCreateWindow(descriptor.width, descriptor.height, descriptor.title.c_str(), nullptr, nullptr); 

	if (!_handle)
	{
		std::fprintf(stderr, "[GLFW] create window failed \n");
		glfwTerminate();
		return false;
	}
	_width = descriptor.width;
	_height = descriptor.height;

	glfwSetWindowUserPointer(_handle, this);
	glfwSetFramebufferSizeCallback(_handle, [](GLFWwindow* window, int width, int height) {
		auto* self = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
		self->_width = width;
		self->_height = height;
		});

	return true;
}

void Window::pollEvents()
{
	glfwPollEvents();
}

bool Window::shouldClose() const
{
	return _handle ? glfwWindowShouldClose(_handle) != 0 : true;
}
void Window::requestClose()
{
	if (_handle) glfwSetWindowShouldClose(_handle, 1);
}
void* Window::nativeHandle() const
{
	return reinterpret_cast<void*>(_handle);
}