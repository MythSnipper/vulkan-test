#include <main.hpp>


class HelloTriangleApplication{
    public:
    void run(){
        std::cout << "Application started:\n";
        initWindow();
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
    

    //minimum message severity(changes which error messages are displayed)
    /*
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: Diagnostic message
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT: Informational message like the creation of a resource
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: Message about behavior that is not necessarily an error, but very likely a bug in your application
    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT: Message about behavior that is invalid and may cause crashes
    */
    const static VkDebugUtilsMessageSeverityFlagBitsEXT minimumDebugMessageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;

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
        else{
            std::cout << "GLFW initialized!\n";
        }
        //hint and create window, no api for vulkan
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(GLFW_WINDOW_WIDTH, GLFW_WINDOW_HEIGHT, GLFW_WINDOW_TITLE, nullptr, nullptr);
        if(!window){
            throw std::runtime_error("GLFW Failed to create window");
        }
        else{
            std::cout << "GLFW window created!\n";
        }
    }
    //creates vulkan instance
    void createInstance(){
        //check if validation layers are supported
        if(enableValidationLayers && !checkValidationLayerSupport()){
            throw std::runtime_error("Validation layers requested, but not available!");
        }
        else{
            std::cout << "Validation layers supported and available!\n";
        }
        //print system's supported vulkan version
        uint32_t instanceVersion = 0;
        vkEnumerateInstanceVersion(&instanceVersion);
        std::cout << "System supported Vulkan version: "
          << VK_VERSION_MAJOR(instanceVersion) << "."
          << VK_VERSION_MINOR(instanceVersion) << "."
          << VK_VERSION_PATCH(instanceVersion) << "\n";

        //info about our app vulkan needs to optimize driver
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; ///type
        appInfo.pApplicationName = "Hello Triangle"; 
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0); //app's version
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
        appInfo.pNext = nullptr; //can point to extension information

        //not optional struct, tells vulkan which global extensions and validation layers are used
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; //type
        createInfo.pApplicationInfo = &appInfo; //point to app info

        //get required extensions
        auto extensions = getRequiredExtensions(); //list of required extension names
        //add to create info
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        
        //if validation layers enabled, add validation layer info to create info
        if(enableValidationLayers){
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());//number of validation layers enabled
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else{
            createInfo.enabledLayerCount = 0; //no validation layers enabled
        }

        //create vulkan instance using the create info
        if(vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS){
            throw std::runtime_error("failed to create instance!");
        }
        else{
            std::cout << "Created vulkan instance!\n";
        }

        //check for extension support
        //get number of extensions available
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        //holds the extension details
        std::vector<VkExtensionProperties> extensionDetails(extensionCount);
        
        //query extension details
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensionDetails.data());
        
        //list available extensions
        std::cout << "Available/supported extensions:\n";
        //print available extension names
        for(const auto& extension : extensionDetails){
            std::cout << '\t' << extension.extensionName << '\n';
        }

    }
    //checks if validation layers are supported
    bool checkValidationLayerSupport(){
        std::cout << "Checking validation layers support:\n";
        uint32_t layerCount;
        //get layer count
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        //get properties of all the layers
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        //check if all layers in validationLayers exist in availableLayers
        std::cout << "Checking if validation layers are available:\n";
        for(const char* layerName : validationLayers){
            std::cout << "\t" << layerName << ": ";
            bool layerFound = false;

            for(const auto& layerProperties : availableLayers){
                if(strcmp(layerName, layerProperties.layerName) == 0){
                    std::cout << "Supported\n";
                    layerFound = true;
                    break;
                }
            }

            if(!layerFound){
                std::cout << "Nuh uh\n";
                std::cout << "Validation layer support check failed!\n";
                return false;
            }
        }
        std::cout << "Validation layers supported!\n";
        return true;
    }
    //gets list of required extensions by GLFW and optionally validation layers
    std::vector<const char*> getRequiredExtensions(){
        //must require GLFW extensions, they interface with the window system
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        //stores the required extension names
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        //debug messenger extension
        if(enableValidationLayers){
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }
    //debug callback function for vulkan to call
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData){
        /*
            pCallbackData->pMessage: null terminated error message
            pCallbackData->pObjects:
        */
        if(messageSeverity >= minimumDebugMessageSeverity){
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        }

        return VK_FALSE;
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
        std::cout << "Cleaning up...\n";

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


