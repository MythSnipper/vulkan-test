#include <main.hpp>

class HelloTriangleApplication{
    public:
    void run(){
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

    private:

    GLFWwindow* window;
    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    VkInstance instance;

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif

    void initWindow(){
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan test nuck", nullptr, nullptr);
        if(!window){
            std::cout << "GLFW Failed to create window\n";
        }
    }
    void createInstance(){
        if(enableValidationLayers && !checkValidationLayerSupport()){
            throw std::runtime_error("Validation layer requested, but not available");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        
        if(enableValidationLayers){
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else{
            createInfo.enabledLayerCount = 0;
        }

        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        if(result != VK_SUCCESS){
            throw std::runtime_error("FAILED TO CREATE INSTANCE");
        }

        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        
        std::cout << "Available extensions:\n";
        for(const auto& extension : extensions){
            std::cout << "\t" << extension.extensionName << "\n";
        }
    }
    bool checkValidationLayerSupport(){
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        
        for(const char* layerName : validationLayers){
            bool layerFound = false;
            for(const auto& layerProperties : availableLayers){
                std::cout << "Comparing: " << layerName << " : " << layerProperties.layerName << "\n";
                if(strcmp(layerName, layerProperties.layerName) == 0){
                    layerFound = true;
                    break;
                }   
            }
            if(!layerFound){
                return false;
            }
        }
        return true;
    }


    void initVulkan(){
        std::cout << ((enableValidationLayers) ? "Validation Layers Enabled\n" : "Validation Layers Disabled\n");
        createInstance();
    }
    void mainLoop(){
        while(!glfwWindowShouldClose(window)){
            glfwPollEvents();
        }
    }
    void cleanup(){
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);
        glfwTerminate();
    }


    
};

int main(){
    #ifdef _WIN32
        std::cout << "RUNNING ON WINDOWS\n";
    #else
        std::cout << "RUNNING ON LINUX\n";
    #endif

    
    HelloTriangleApplication app;

    try{
        app.run();
    }
    catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}