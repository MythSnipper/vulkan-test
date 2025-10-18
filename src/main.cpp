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
    //handle to the presentation queue
    VkQueue presentQueue;
    //required device extensions
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME //for swapchain
    };
    //criterias needed to check if swap chain is supported with window surface
    struct SwapChainSupportDetails{
        VkSurfaceCapabilitiesKHR capabilities; //basic surface capabilities(min/max number of images in swap chain, min/max width of images)
        std::vector<VkSurfaceFormatKHR> formats; //pixel format, color space
        std::vector<VkPresentModeKHR> presentModes; //avilable presentation modes
    };





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

        //Check if device supports all extensions
        bool extensionsSupported = checkDeviceExtensionSupport(device);
        if(!extensionsSupported){
            return 0;
        }

        //Check if device supports swap chain
        bool swapChainAdequate = false;
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        //Adequate if formats and present modes are not empty
        //right now we require one format and one present mode
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        if(!swapChainAdequate){
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

        std::cout << "Finding queue families for device: \n";
        int i = 0;
        for(const auto& queueFamily : queueFamilies){
            //detect if the queue supports graphics commands
            if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
                indices.graphicsFamily = i;
            }
            //detect if the queue supports presenting to our window surface
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if(presentSupport){
                indices.presentFamily = i;
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
        //create info for queues we are using
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        //queue families indices
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        //assign priority to queue(0.0f - 1.0f)
        float queuePriority = 1.0f;
        //fill in the create infos
        for(uint32_t queueFamily : uniqueQueueFamilies){
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; //type
            queueCreateInfo.queueFamilyIndex = queueFamily; //index
            queueCreateInfo.queueCount = 1; //number of queues
            queueCreateInfo.pQueuePriorities = &queuePriority; //point to priority
            queueCreateInfos.push_back(queueCreateInfo); //add it to the vector
        }
        
        //create info for logical device
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data(); //points to queue create infos
        //number of queue create infos
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

        createInfo.pEnabledFeatures = &deviceFeatures;  //used device features

        //device features enabled
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()); //number of device extensions
        createInfo.ppEnabledExtensionNames = deviceExtensions.data(); //point to the vector

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
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

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
    //check if physical device supports required extensions
    bool checkDeviceExtensionSupport(VkPhysicalDevice device){
        //get extensions of the physical device
        //first get the count
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        //then store properties into a vector
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
        //convert deviceExtensions to a set of strings, which is the list of required extensions
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        //loop availableExtensions and remove from required list
        for(const auto& extension : availableExtensions){
            requiredExtensions.erase(extension.extensionName);

            //optimization: if it is empty already return true
            if(requiredExtensions.empty()){
                return true;
            }
        }

        //if the required list is empty then return true
        return requiredExtensions.empty();
    }
    //populates the SwapChainSupportDetails struct
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device){
        SwapChainSupportDetails details;
        //get physical device surface capabilities and store in the struct
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
        //query for supported surface formats
        //get count first
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        //then actually get the thing
        if(formatCount != 0){
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }
        //query for surface present modes
        //get count first
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        //then actually get the thing
        if(presentModeCount != 0){
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }
    //choose best swapchain surface format from available formats
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats){
        std::cout << "Choosing best swapchain surface format:\n\t";
        //find best format
        for(const auto& availableFormat : availableFormats){
            //best is 32bbp rgba, with SRGB support for colorspace
            if(availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR){
                std::cout << "Optimal format found!\n";
                return availableFormat;
            }
        }
        //just use the first one because lazy to rate each format and get the second best one
        std::cout << "Default format selected because optimal format not found!\n";
        return availableFormats[0];
    }
    //choose best swapchain surface present mode from available modes
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes){
        /*
        4 available present modes:
        
        VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application are transferred to the screen 
            right away, which may result in tearing.
        VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes an image from the front 
            of the queue when the display is refreshed and the program inserts rendered images at the back 
                of the queue. If the queue is full then the program has to wait. This is most similar to 
                    vertical sync as found in modern games. The moment that the display is refreshed is 
                        known as "vertical blank".
        VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs from the previous one if the application is 
            late and the queue was empty at the last vertical blank. Instead of waiting for the next vertical 
                blank, the image is transferred right away when it finally arrives. This may result in 
                    visible tearing.
        VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode. Instead of blocking the 
            application when the queue is full, the images that are already queued are simply replaced with 
                the newer ones. This mode can be used to render frames as fast as possible while still 
                    avoiding tearing, resulting in fewer latency issues than standard vertical sync. This is 
                        commonly known as "triple buffering", although the existence of three buffers alone 
                            does not necessarily mean that the framerate is unlocked.
        */
        std::cout << "Selecting swapchain presentation mode: \n";
        bool immediate = false; //proprity 4, may tear harder than niko
        bool fifo = false; //priority 2, vsync
        bool fifo_relaxed = false; //priority 3, may tear
        bool mailbox = false; //priority 1, triple buffering
        for(const auto& availablePresentMode : availablePresentModes){
            immediate = immediate || (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR);
            fifo = fifo || (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR);
            fifo_relaxed = fifo_relaxed || (availablePresentMode == VK_PRESENT_MODE_FIFO_RELAXED_KHR);
            mailbox = mailbox || (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR);
        }
        std::cout << "\t";
        if(mailbox){
            std::cout << "Mailbox(triple buffering) presentation mode\n";
            return VK_PRESENT_MODE_MAILBOX_KHR;
        }
        else if(fifo){
            std::cout << "FIFO(vsync) presentation mode\n";
            return VK_PRESENT_MODE_FIFO_KHR;
        }
        else if(fifo_relaxed){
            std::cout << "FIFO relaxed presentation mode(may cause screen tearing)\n";
            return VK_PRESENT_MODE_FIFO_RELAXED_KHR;
        }
        else{
            std::cout << "Immediate presentation mode(may cause screen tearing)\n";
            return VK_PRESENT_MODE_IMMEDIATE_KHR;
        }

    }
    //choose best swap extent(resolution of swapchain images
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities){
        if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()){
            return capabilities.currentExtent;
        }
        else{
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
    
            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };
            //clamp values to the min and max range of the implementation
            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    
            return actualExtent;
        }
    }
    //create swap chain
    void createSwapChain(){
        ///query swap chain support(it's a struct)
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
        //best of surface format, present mode, extent
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        //how many images to have in the swapchain
        //minimum is too shit so we add 1 to it to make it better
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
        //if max is not infinite(if it's infinite it's 0) and the count is less than max
        if(swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount){
            //take more resources, basically
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }
        std::cout << "Selected swapchain image count: " << imageCount << "\n";
        std::cout << "Max " << swapChainSupport.capabilities.maxImageCount;
        std::cout << "Min " << swapChainSupport.capabilities.minImageCount << "\n";


    }





    void initVulkan(){
        std::cout << ((enableValidationLayers) ? "Validation Layers Enabled\n" : "Validation Layers Disabled\n");
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();

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


