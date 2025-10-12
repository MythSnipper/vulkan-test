#include <main.hpp>

//creates VkDebugUtilsMessengerEXT object
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger){
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if(func != nullptr){
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else{
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}
//destroys VkDebugUtilsMessengerEXT object
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator){
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if(func != nullptr){
        func(instance, debugMessenger, pAllocator);
    }
}






class HelloTriangleApplication{
    public:
    void run(){
        std::cout << "Application started:\n";
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
        std::cout << "The End.\n";
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
    //handle for debug callback function
    VkDebugUtilsMessengerEXT debugMessenger;
    //handle for physical device
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    //struct to store all queue families we need
    struct QueueFamilyIndices{
        //using optional so the value of 0 and unavailable graphics family can be distinguished
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;


        //check if all queue families actually exist
        bool isComplete(){
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };
    //handle for logical device
    VkDevice device;
    //device features used
    VkPhysicalDeviceFeatures deviceFeatures{};
    //handle for the queue
    VkQueue graphicsQueue; 
    //window surface
    VkSurfaceKHR surface;









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
        
        //debug messenger create info
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        //if validation layers enabled, add validation layer info to create info
        if(enableValidationLayers){
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());//number of validation layers enabled
            createInfo.ppEnabledLayerNames = validationLayers.data();

            //fill debug messenger create info and make instance create info point to it
            //This will enable debugging in the createInstance and destroyInstance calls
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        }
        else{
            createInfo.enabledLayerCount = 0; //no validation layers enabled

            createInfo.pNext = nullptr; //do not point to debug messenger info
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
                std::cout << "Unsupported\n";
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
        
        //print
        std::cout << "Required extensions: \n";
        for(const char* extension : extensions){
            std::cout << "\t" << extension << "\n";
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
            std::cerr << "\tValidation layer: ";
            switch(messageType){
                case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
                    std::cerr << "GENERAL";
                    break;
                case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
                    std::cerr << "VALIDATION";
                    break;
                case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
                    std::cerr << "PERFORMANCE";
                    break;
                default:
                    std::cerr << "Unrecognized message type";
                    break;
            };
            std::cerr << "\n\t\t";
            std::cerr << pCallbackData->pMessage << "\n";
            std::cerr << "\t\tRelated num of objs: " << pCallbackData->objectCount << "\n";
        }
        else{
            std::cout << "$\n";
        }

        return VK_FALSE;
    }
    //sets up debug messenger by binding the callback function to vulkan
    void setupDebugMessenger(){
        if(!enableValidationLayers){
            return;
        }
        
        //use populate function to fill the create info
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);
        //use our function to create
        if(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS){
            throw std::runtime_error("failed to set up debug messenger!");
        }
        else{
            std::cout << "Debug messenger set up!\n";
        }


    }
    //fills info to create debug messenger thing
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo){
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT; //type
        //severity filters which types of messages the callback receives
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        //make more verbose(optional)
            //createInfo.messageSeverity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
        //type filters which message types the callback receives
        //all types are enabled
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        //pointer to callback function
        createInfo.pfnUserCallback = debugCallback;
        //whatever this pointer is will be passed to callback function
        createInfo.pUserData = nullptr; // Optional
    }
    //selects a viable physical graphics card
    void pickPhysicalDevice(){
        //query number of devices by passing nullptr
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        std::cout << deviceCount << " GPU" << ((deviceCount > 1) ? "s" : "") << " with vulkan support found!\n";
        //if no gpu found exit
        if(deviceCount == 0){
            throw std::runtime_error("Failed to find GPUs with Vulkan support!");
        }
        
        //actually get the devices information
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        std::cout << "Selecting GPU based on score: \n";
        //use ordered map to sort candidates by increasing score
        std::multimap<int, VkPhysicalDevice> candidates;
        //get devices scores and add to map
        for(const auto& device : devices){
            int score = rateDeviceSuitability(device);
            candidates.insert(std::make_pair(score, device));

            //check if best candidate is even supported
            //if supported, set physicalDevice to the best device's handle
            if(candidates.rbegin()->first > 0){
                physicalDevice = candidates.rbegin()->second;
            }
            else{
                throw std::runtime_error("failed to find a suitable GPU!");
            }
        }
        //print the selected device
        //query the device for basic device properties
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        std::cout << "Selected device: " << deviceProperties.deviceName;
        const char* deviceTypes[] = {"(other)", 
            "(integrated)",
            "(dedicated)",
            "(virtual)",
            "(CPU)"
        };
        if((uint32_t)(deviceProperties.deviceType) <= 4){
            std::cout << deviceTypes[(uint32_t)(deviceProperties.deviceType)];
        }
        else{
            //unknown device type???
            std::cout << "(" "???" ")";
        }
        std::cout << "\n";

    }
    //check if device is competent as well as how good the device is, 0 means unsupported, higher score is better
    int rateDeviceSuitability(VkPhysicalDevice device){
        //query the device for basic device properties
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        //for optional features like texture compression, 64 bit floats, and multi viewport rendering, query device features
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        int score = 0;
    
        //Discrete GPUs have a significant performance advantage
        if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU){
            score += 1000;
        }
    
        //Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;
    
        //Application can't function without geometry shaders
        if(!deviceFeatures.geometryShader){
            return 0;
        }

        //Check device queue capability
        QueueFamilyIndices indices = findQueueFamilies(device); 
        //Application can't function without graphics command queue
        if(!indices.isComplete()){
            return 0;
        }

        std::cout << "\t" << deviceProperties.deviceName << " | score: " << score << "\n";

        return score;
    }
    //find the device's queue family that supports sending graphics commands
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device){
        QueueFamilyIndices indices;
        //populate struct using information from device

        //get number of queue families
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        //actually get the properties from the queue families
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for(const auto& queueFamily : queueFamilies){
            //detect if the queue supports graphics commands
            if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
                indices.graphicsFamily = i;
            }
            //more checks if needed


            //early exit optimization if all required queue families are already found
            if(indices.isComplete()){
                break;
            }

            i++;
        }

        return indices;
    }
    //set up logical device from physical device
    void createLogicalDevice(){
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        //describes create info for queues used and number of queues used
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value(); //make this the indices
        queueCreateInfo.queueCount = 1; //number of queues
        //assign priority to queue(0.0f - 1.0f)z
        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        
        //create info for logical device
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo; //pointers to queue create info
        createInfo.queueCreateInfoCount = 1;

        createInfo.pEnabledFeatures = &deviceFeatures;  //and used device features

        //device extensions used
        createInfo.enabledExtensionCount = 0;

        //enabledLayerCount and ppEnabledLayerNames are ignored in newer vulkan implementations
        //because vulkan made instance and device specific layers the same
        //doing this for backwards compatibility
        if(enableValidationLayers){
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else{
            createInfo.enabledLayerCount = 0;
        }

        //create logical device
        if(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS){
            throw std::runtime_error("failed to create logical device!");
        }
        else{
            std::cout << "Created logical device!\n";
        }

        //retrieve handles for each queue family
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);




    }
    //create surface using GLFW
    void createSurface(){
        //create surface using GLFW call
        if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS){
            throw std::runtime_error("failed to create window surface!");
        }
        else{
            std::cout << "Created window surface using GLFW!\n";
        }


    }








    void initVulkan(){
        std::cout << ((enableValidationLayers) ? "Validation Layers Enabled\n" : "Validation Layers Disabled\n");
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();


    }
    void mainLoop(){
        while(!glfwWindowShouldClose(window)){
            glfwPollEvents();
        }
    }
    void cleanup(){
        std::cout << "Cleaning up...\n";
        
        //clean up logical device
        vkDestroyDevice(device, nullptr);
        
        //clean up window surface
        vkDestroySurfaceKHR(instance, surface, nullptr);

        //clean up debug messenger
        if(enableValidationLayers){
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }
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


