#include "VulkanContext.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "../Editor/Window.h"

#include <cstdio>
#include <cstring>
#include <cassert>

#if defined(_DEBUG) || defined(DEBUG)
static constexpr bool kDefaultValidation = true;
#else
static constexpr bool kDefaultValidation=false;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT severity,
    VkDebugUtilsMessageTypeFlagsEXT type,
    const VkDebugUtilsMessengerCallbackDataEXT* data,
    void* userData)
{
    (void)type;
    (void)userData;
    const char* sev =
        (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) ? "ERROR" : (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) ? "WARN " : (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) ? "INFO " : "VERB ";

    std::fprintf(stderr, "[Vulkan][%s] %s\n", sev, data->pMessage);
    return VK_FALSE;
}

VulkanContext::~VulkanContext()
{
    shutdown();
}

bool VulkanContext::init(const CreateInfo& createInfo)
{
    _validation = createInfo.enableValidation && kDefaultValidation;
   // 1) Instance
   if (!createInstance(createInfo))
    {
        std::fprintf(stderr, "[Vulkan] vkCreateInstance failed in init.\n");
        return false;
    }

    pfnCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT"));
    pfnDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT"));
    // 2) Debug messenger
    if (_validation)
    {
        if (!setupDebugMessenger())
        {
            std::fprintf(stderr, "[Vulkan] Failed to setup debug messenger.\n");
            return false;
        }
    }

    // 3) Surface 
    if (!createSurface(createInfo.window)) return false;

    // 4) GPU + queue families
    if (!pickPhysicalDevice()) return false;

    // 5) Device + Queues
    if (!createDeviceAndQueues()) return false;
    
    // Runtime info
    uint32_t apiVersion = 0;
    if (vkEnumerateInstanceVersion)
    {
        vkEnumerateInstanceVersion(&apiVersion);
        std::fprintf(stderr, "[Vulkan] Instance created. Runtime API %u.%u.%u\n",
                     VK_API_VERSION_MAJOR(apiVersion), VK_API_VERSION_MINOR(apiVersion), VK_API_VERSION_PATCH(apiVersion));
    }
    return true;
}


void VulkanContext::shutdown()
{
    if (_device) {
        vkDeviceWaitIdle(_device);
        vkDestroyDevice(_device, nullptr);
        _device = VK_NULL_HANDLE;
    }
    if (_surface) {
        vkDestroySurfaceKHR(_instance, _surface, nullptr);
        _surface = VK_NULL_HANDLE;
    }
    if (_validation && _debugMessenger && pfnDestroyDebugUtilsMessengerEXT)
    {
        pfnDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
        _debugMessenger = VK_NULL_HANDLE;
    }
    if (_instance)
    {
        vkDestroyInstance(_instance, nullptr);
        _instance = VK_NULL_HANDLE;
    }
}


bool VulkanContext::createInstance(const CreateInfo& ci)
{
    // 1) Validation layer support
    const char* validationLayer = "VK_LAYER_KHRONOS_validation";
    if (_validation && !checkValidationLayerSupport())
    {
        std::fprintf(stderr, "[Vulkan] Validation layer not available. Disabling.\n");
        _validation = false;
    }

    // 2) App info
    VkApplicationInfo app{};
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pApplicationName = ci.appName;
    app.applicationVersion = ci.appVersion;
    app.pEngineName = "SomineEngineV";
    app.engineVersion = VK_MAKE_VERSION(0, 1, 0);
    app.apiVersion = VK_API_VERSION_1_3;

    // 3) Extensions
    std::vector<const char*> extensions = getRequiredExtensions(_validation);

    // 4) Instance create info
    VkInstanceCreateInfo ici{};
    ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.pApplicationInfo = &app;
    ici.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    ici.ppEnabledExtensionNames = extensions.data();

    // 5) Validation layers
    const char* layers[] = {validationLayer};
    if (_validation)
    {
        ici.enabledLayerCount = 1;
        ici.ppEnabledLayerNames = layers;

        // pNext chain with debug messenger create info (so we catch messages during vkCreateInstance too)
        VkDebugUtilsMessengerCreateInfoEXT dbg{};
        dbg.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        dbg.messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        dbg.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        dbg.pfnUserCallback = DebugCallback;
        ici.pNext = &dbg;
    }

    VkResult res = vkCreateInstance(&ici, nullptr, &_instance);
    if (res != VK_SUCCESS)
    {
        std::fprintf(stderr, "[Vulkan] vkCreateInstance failed (%d)\n", res);
        return false;
    }
    return true;
}


bool VulkanContext::setupDebugMessenger()
{
    if (!pfnCreateDebugUtilsMessengerEXT) return false;

    VkDebugUtilsMessengerCreateInfoEXT ci{};
    ci.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    ci.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    ci.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    ci.pfnUserCallback = DebugCallback;

    VkResult res = pfnCreateDebugUtilsMessengerEXT(_instance, &ci, nullptr, &_debugMessenger);
    if (res != VK_SUCCESS)
    {
        std::fprintf(stderr, "[Vulkan] CreateDebugUtilsMessengerEXT failed (%d)\n", res);
        return false;
    }
    return true;
}

void VulkanContext::destroyDebugMessenger()
{
    if (_debugMessenger && pfnDestroyDebugUtilsMessengerEXT)
    {
        pfnDestroyDebugUtilsMessengerEXT(_instance, _debugMessenger, nullptr);
        _debugMessenger = VK_NULL_HANDLE;
    }
}

bool VulkanContext::checkValidationLayerSupport() const
{
    uint32_t count = 0;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> props(count);
    vkEnumerateInstanceLayerProperties(&count, props.data());

    for (const auto& p : props)
    {
        if (std::strcmp(p.layerName, "VK_LAYER_KHRONOS_validation") == 0)
            return true;
    }
    return false;
}

std::vector<const char*> VulkanContext::getRequiredExtensions(bool enableValidation) const
{
    uint32_t glfwCount = 0;
    const char** glfwExt = glfwGetRequiredInstanceExtensions(&glfwCount);

    std::vector<const char*> exts;
    exts.reserve(glfwCount + 1);
    for (uint32_t i = 0; i < glfwCount; ++i) exts.push_back(glfwExt[i]);

    // For debug messenger
    if (enableValidation)
    {
        exts.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // "VK_EXT_debug_utils"
    }
    return exts;
}

// ---------- Surface + GPU + Device/Queues ----------

bool VulkanContext::createSurface(const Window* window)
{
    if (!window) { std::fprintf(stderr, "[Vulkan] No Window for surface.\n"); return false; }
    GLFWwindow* glf_wwindow = reinterpret_cast<GLFWwindow*>(window->nativeHandle());
    VkResult r = glfwCreateWindowSurface(_instance, glf_wwindow, nullptr, &_surface);
    if (r != VK_SUCCESS) {
        std::fprintf(stderr, "[Vulkan] glfwCreateWindowSurface failed (%d)\n", r);
        return false;
    }
    return true;
}

bool VulkanContext::deviceSupportsRequiredExtensions(VkPhysicalDevice gpu)
{
    // For now only swapchain
    const char* required[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    uint32_t count = 0;
    vkEnumerateDeviceExtensionProperties(gpu, nullptr, &count, nullptr);
    std::vector<VkExtensionProperties> props(count);
    vkEnumerateDeviceExtensionProperties(gpu, nullptr, &count, props.data());

    for (const char* need : required) {
        bool found = false;
        for (const auto& p : props)
            if (std::strcmp(p.extensionName, need) == 0) { found = true; break; }
        if (!found) return false;
    }
    return true;
}

bool VulkanContext::queryQueueFamilies(VkPhysicalDevice gpu, VkSurfaceKHR surf,
                                       uint32_t& outGfx, uint32_t& outPresent)
{
    outGfx = outPresent = VK_QUEUE_FAMILY_IGNORED;

    uint32_t n = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &n, nullptr);
    std::vector<VkQueueFamilyProperties> q(n);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &n, q.data());

    for (uint32_t i = 0; i < n; ++i) {
        const bool isGfx = (q[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0;
        VkBool32 present = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(gpu, i, surf, &present);
        if (isGfx   && outGfx    == VK_QUEUE_FAMILY_IGNORED) outGfx    = i;
        if (present && outPresent== VK_QUEUE_FAMILY_IGNORED) outPresent= i;
    }
    return outGfx != VK_QUEUE_FAMILY_IGNORED &&
           outPresent != VK_QUEUE_FAMILY_IGNORED;
}

bool VulkanContext::pickPhysicalDevice()
{
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(_instance, &count, nullptr);
    if (count == 0) { std::fprintf(stderr, "[Vulkan] No physical devices.\n"); return false; }
    std::vector<VkPhysicalDevice> gpus(count);
    vkEnumeratePhysicalDevices(_instance, &count, gpus.data());

    for (auto gpu : gpus) {
        if (!deviceSupportsRequiredExtensions(gpu)) continue;

        uint32_t gfx = 0, present = 0;
        if (queryQueueFamilies(gpu, _surface, gfx, present)) {
            _gpu = gpu;
            _gfxFamily = gfx;
            _presentFamily = present;
            return true;
        }
    }
    std::fprintf(stderr, "[Vulkan] No suitable GPU (needs graphics+present+swapchain).\n");
    return false;
}

bool VulkanContext::createDeviceAndQueues()
{
    float prio = 1.0f;

    // Unique families
    std::vector<uint32_t> families;
    families.push_back(_gfxFamily);
    if (_presentFamily != _gfxFamily) families.push_back(_presentFamily);

    std::vector<VkDeviceQueueCreateInfo> qcis;
    qcis.reserve(families.size());
    for (uint32_t fam : families) {
        VkDeviceQueueCreateInfo qci{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        qci.queueFamilyIndex = fam;
        qci.queueCount       = 1;
        qci.pQueuePriorities = &prio;
        qcis.push_back(qci);
    }

    const char* devExts[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }; // Needed for swapchain

    VkPhysicalDeviceFeatures feats{}; // None for now 
    VkDeviceCreateInfo dci{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    dci.queueCreateInfoCount    = (uint32_t)qcis.size();
    dci.pQueueCreateInfos       = qcis.data();
    dci.enabledExtensionCount   = 1;
    dci.ppEnabledExtensionNames = devExts;
    dci.pEnabledFeatures        = &feats;

    VkResult r = vkCreateDevice(_gpu, &dci, nullptr, &_device);
    if (r != VK_SUCCESS) {
        std::fprintf(stderr, "[Vulkan] vkCreateDevice failed (%d)\n", r);
        return false;
    }

    vkGetDeviceQueue(_device, _gfxFamily,     0, &_gfxQueue);
    vkGetDeviceQueue(_device, _presentFamily, 0, &_presentQueue);
    return true;
}