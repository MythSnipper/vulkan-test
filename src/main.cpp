#include <main.hpp>


class HelloTriangleApplication{
    public:
    void run(){
        initWindow(); //create GLFW window
        initVulkan();
        mainLoop();
        cleanup();
    }

    private:

    //GLFW window information
    GLFWwindow* window;
    const uint32_t GLFW_WINDOW_WIDTH = 800;
    const uint32_t GLFW_WINDOW_HEIGHT = 600;
    const char* GLFW_WINDOW_TITLE = "vulkan test nuck";

    //enable validation layers unless NO DEBUG
    #ifdef NDEBUG
        const bool enableValidationLayers = false;
    #else
        const bool enableValidationLayers = true;
    #endif

    //validation layers used
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    //Vulkan stuff
    //handle to Vulkan instance
    VkInstance instance;
    

    //creates GLFW window
    void initWindow(){
        //detect display server protocol if not running on windows
        #ifndef _WIN32
            std::cout << "GLFW platform: " << ((glfwGetPlatform()) ? "wayland" : "X11") << "\n";
        #endif
        //init GLFW library
        if(!glfwInit()){
            throw std::runtime_error("failed to initialize GLFW");
        }
        //hint and create window, no api for vulkan
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(GLFW_WINDOW_WIDTH, GLFW_WINDOW_HEIGHT, GLFW_WINDOW_TITLE, nullptr, nullptr);
        if(!window){
            throw std::runtime_error("GLFW Failed to create window");
        }
    }
    //creates vulkan instance
    void createInstance(){
        //check if validation layers are supported
        if(enableValidationLayers && !checkValidationLayerSupport()){
            throw std::runtime_error("validation layers requested, but not available!");
        }
        //print supported vulkan version
        uint32_t instanceVersion = 0;
        vkEnumerateInstanceVersion(&instanceVersion);
        std::cout << "Supported Vulkan version: "
          << VK_VERSION_MAJOR(instanceVersion) << "."
          << VK_VERSION_MINOR(instanceVersion) << "."
          << VK_VERSION_PATCH(instanceVersion) << "\n";

        //info about our app provided to vulkan
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; ///type
        appInfo.pApplicationName = "Hello Triangle"; 
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pNext = nullptr; //can point to extension information

        //not optional struct, tells vulkan which global extensions and validation layers are used
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        //get required extensions
        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        
        //include validation layer names in struct
        if(enableValidationLayers){
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());//number of validation layers enabled
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else{
            createInfo.enabledLayerCount = 0; //no validation layers enabled
        }

        //create instance
        if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS){
            throw std::runtime_error("failed to create instance!");
        }

        //check for extension support
        //query number of extensions
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        //holds the extension details
        std::vector<VkExtensionProperties> extensionDetails(extensionCount);
        
        //query extension details
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionDetails.data());
        
        //list available extensions
        std::cout << "available extensions:\n";

        for(const auto& extension : extensionDetails){
            std::cout << '\t' << extension.extensionName << '\n';
        }

        
    }
    //checks if validation layers are supported
    bool checkValidationLayerSupport(){
        std::cout << "Checking validation layers support:\n";
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        //check if all layers in validationLayers exist in availableLayers
        for(const char* layerName : validationLayers){
            std::cout << "\tChecking if layer " << layerName << " is available: ";
            bool layerFound = false;

            for(const auto& layerProperties : availableLayers){
                if(strcmp(layerName, layerProperties.layerName) == 0){
                    std::cout << "Yes\n";
                    layerFound = true;
                    break;
                }
            }

            if(!layerFound){
                std::cout << "No\n";
                return false;
            }
        }

        return true;
    }
    //gets list of required extensions based on if validation layers are enabled or not
    std::vector<const char*> getRequiredExtensions(){
        //must require GLFW extensions, they interface with the window system
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        //debug messenger extension
        if(enableValidationLayers){
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
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

        //clean up vulkan instance
        vkDestroyInstance(instance, nullptr);

        //clean up GLFW window and GLFW
        glfwDestroyWindow(window);
        glfwTerminate();
    }


    
};

int main(){
    //detect if we are running on windows or linux
    #ifdef _WIN32
        std::cout << "RUNNING ON WINDOWS\n";
    #else
        std::cout << "RUNNING ON LINUX\n";
    #endif

    //run vulkan app
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


