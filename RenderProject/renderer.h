#ifndef RENDERER
#define RENDERER

#include "stuff.h"

namespace tde{
	constexpr int MAX_FRAMES_IN_FLIGHT = 2;

    struct FrameData {

        //VkCommandPool commandPool; //why is this here??
        VkCommandBuffer mainCommandBuffer;
        VkSemaphore imageAvailableSemaphore;
        VkSemaphore renderFinishedSemaphore;
        VkFence inFlightFence;
    };




	class Renderer
	{
#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif



	public:
		int width = 0;
		int height = 0;
		VkInstance instance = VK_NULL_HANDLE;// Vulkan library handle
		VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;// Vulkan debug output handle
		VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;// GPU chosen as the default device
		VkDevice device = VK_NULL_HANDLE; // Vulkan device for commands
		VkSurfaceKHR surface = VK_NULL_HANDLE;// Vulkan window surface

        VkQueue graphicsQueue = VK_NULL_HANDLE;  //where we put draw commands
        uint32_t graphicsQueueFamily;

        VkQueue presentQueue = VK_NULL_HANDLE;

        VkExtent2D swapchainExtent;
        VkSwapchainKHR swapchain = VK_NULL_HANDLE;

        std::vector<VkImage> swapchainImages;
        VkFormat swapChainImageFormat = {};
        VkExtent2D swapChainExtent = {};

        std::vector<VkImageView> swapchainImageViews;


        VkDescriptorSetLayout descriptorSetLayout;

        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline graphicsPipeline = VK_NULL_HANDLE;

      //  std::vector<VkFramebuffer> swapChainFramebuffers;
        VkCommandPool commandPool = VK_NULL_HANDLE;

        VkRenderPass renderPass = VK_NULL_HANDLE;

        //Unifor Buffer Objects, currently not in use
        std::vector<VkBuffer> uniformBuffers;
        std::vector<VkDeviceMemory> uniformBuffersMemory;
        std::vector<void*> uniformBuffersMapped;


        VkDescriptorPool descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;

        std::vector<VkCommandBuffer> commandBuffers;

        //sync objects
        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;

        //depth objects
        VkImage depthImage;
        VkDeviceMemory depthImageMemory;
        VkImageView depthImageView;

        int currentFrame = 0;
        bool framebufferResized = false;
        int frameNumber = 0;
        FrameData frames[MAX_FRAMES_IN_FLIGHT];

        FrameData& get_current_frame() { return frames[frameNumber % MAX_FRAMES_IN_FLIGHT]; };

		Renderer();
		~Renderer();

		void Create(std::vector<void*> args, int width, int height);

        void BeginFrame();

        void SetViewport(int width, int height);

	private:
        bool resize_requested = false;
		void Init(int width, int height);
		void CreateSurfaceOnWindows(HWND hwnd, HINSTANCE hInstance);
        void CreateSwapchain(uint32_t width, uint32_t height);
        void ResizeSwapchain();
        void DestroySwapchain();
		void InitCommands();
		void InitSyncStructures();
		void Destroy();

	};


}


#endif