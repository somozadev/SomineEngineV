#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

class Window;

class VulkanContext
{
public:
    struct CreateInfo
    {
        const Window* window = nullptr;
        bool enableValidation = true; //set to false for release
        const char* appName = "SomineEngineV";
        uint32_t appVersion = VK_MAKE_VERSION(0, 1, 0);
    };

    VulkanContext() = default;
    ~VulkanContext();

    bool init(const CreateInfo& createInfo);
    void shutdown();
    VkInstance instance() const { return _instance; }
    VkSurfaceKHR surface() const { return _surface; }
    VkPhysicalDevice physicalDevice() const { return _gpu; }
    VkDevice device() const { return _device; }
    VkQueue graphicsQueue() const { return _gfxQueue; }
    VkQueue presentQueue() const { return _presentQueue; }
    uint32_t graphicsFamily() const { return _gfxFamily; }
    uint32_t presentFamily() const { return _presentFamily; }

private:
    bool createInstance(const CreateInfo& createInfo);
    bool setupDebugMessenger();
    void destroyDebugMessenger();

    bool createSurface(const Window* window);
    bool pickPhysicalDevice();
    bool createDeviceAndQueues();

    bool checkValidationLayerSupport() const;
    std::vector<const char*> getRequiredExtensions(bool enableValidation) const;

    static bool deviceSupportsRequiredExtensions(VkPhysicalDevice gpu);
    static bool queryQueueFamilies(VkPhysicalDevice gpu, VkSurfaceKHR surf,
                                   uint32_t& outGfx, uint32_t& outPresent);

private:
    VkInstance _instance = VK_NULL_HANDLE;

    VkDebugUtilsMessengerEXT _debugMessenger = VK_NULL_HANDLE;

    PFN_vkCreateDebugUtilsMessengerEXT pfnCreateDebugUtilsMessengerEXT = nullptr;
    PFN_vkDestroyDebugUtilsMessengerEXT pfnDestroyDebugUtilsMessengerEXT = nullptr;
    bool _validation = true;


    VkSurfaceKHR _surface = VK_NULL_HANDLE;
    VkPhysicalDevice _gpu = VK_NULL_HANDLE;
    uint32_t _gfxFamily = VK_QUEUE_FAMILY_IGNORED;
    uint32_t _presentFamily = VK_QUEUE_FAMILY_IGNORED;
    VkDevice _device = VK_NULL_HANDLE;
    VkQueue _gfxQueue = VK_NULL_HANDLE;
    VkQueue _presentQueue = VK_NULL_HANDLE;
};
