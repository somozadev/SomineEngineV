#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // GLFW was originally designed to create an OpenGL context,
    // so we need to tell it not to create one
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Create a window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan Window", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Create Vulkan instance (not shown)
    VkInstance instance = VK_NULL_HANDLE;
    // ... create instance ...

    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::cout << extensionCount << " extensions supported \n";
    glm::mat4 matrix; 
    glm::vec4 vec; 
    auto test = matrix * vec; 

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Render with Vulkan (not shown)
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}