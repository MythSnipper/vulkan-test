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
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;
        
    }

    void initVulkan(){
        createInstance();
    }
    void mainLoop(){
        while(!glfwWindowShouldClose(window)){
            glfwPollEvents();
        }
    }
    void cleanup(){
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