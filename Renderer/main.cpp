#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <vector>


class Application
{
public:

    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    const uint32_t _width = 720;
    const uint32_t _height = 480;
    GLFWwindow* _window;
    VkInstance _instance;
    
    void initWindow()
    {
        if (!glfwInit())
            return;
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        _window = glfwCreateWindow(_width, _height, "Somine Engine V", nullptr, nullptr);
    }

    void initVulkan()
    {
        createInstance();   
    }
    void mainLoop()
    {
        while (!glfwWindowShouldClose(_window))
        {
            glfwPollEvents();
        }
    }

    void createInstance()
    {
        VkApplicationInfo appInfo {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Somine Engine V";
        appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        ci.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> requiredExtensions;
        for (uint32_t i = 0; i < glfwExtensionCount; i++)
            requiredExtensions.emplace_back(glfwExtensions[i]);

        requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

        ci.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
        ci.enabledExtensionCount = (uint32_t) requiredExtensions.size(); 
        ci.ppEnabledExtensionNames = requiredExtensions.data();
        ci.enabledLayerCount =0;

        if(vkCreateInstance(&ci, nullptr, &_instance) != VK_SUCCESS)
            throw::std::runtime_error("Failed to create vk instance.");

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

        std::cout << "Available extensions(" << extensionCount << "):\n";
        for(const auto& extension: extensions)
            std::cout << "\t" << extension.extensionName << "\n";
    }
    void cleanup()
    {
        vkDestroyInstance(_instance,  nullptr);
        glfwDestroyWindow(_window);
        glfwTerminate();
        
    }
};


int main()
{
    Application application;
    try
    {
        application.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
