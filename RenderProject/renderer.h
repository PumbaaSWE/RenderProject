#ifndef RENDERER
#define RENDERER

#include "stuff.h"

#include "Swapchain.h"
#include "vk_init.h"
#include "Descriptors.h"

namespace tde{



    struct Vertex {
        vec3_t pos;
        vec3_t normal;

        static vkinit::VertexInputDescription GetVertexInputDescription();
    };

    struct UniformBufferObject {
        mat4_t model;
        mat4_t view;
        mat4_t proj;
    };


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

        VkExtent2D swapchainExtent{};
        //VkSwapchainKHR swapchain = VK_NULL_HANDLE;

        //std::vector<VkImage> swapchainImages;
        //VkFormat swapChainImageFormat = {};
        //VkExtent2D swapChainExtent = {};

        //Device* pDevice = nullptr;

        //std::vector<VkImageView> swapchainImageViews;
        Swapchain swapchain{};




        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline graphicsPipeline = VK_NULL_HANDLE;

      //  std::vector<VkFramebuffer> swapChainFramebuffers;
        VkCommandPool commandPool = VK_NULL_HANDLE;

        VkFence imFence;
        VkCommandBuffer imCommandBuffer;


        VkRenderPass renderPass = VK_NULL_HANDLE;

        //Unifor Buffer Objects, currently not in use
        std::vector<VkBuffer> uniformBuffers{};
        std::vector<VkDeviceMemory> uniformBuffersMemory{};
        std::vector<void*> uniformBuffersMapped{};


        DescriptorAllocator globalDescriptorAllocator;
        VkDescriptorSetLayout descriptorSetLayout{};

        //VkDescriptorSet _drawImageDescriptors;
        //VkDescriptorSetLayout _drawImageDescriptorLayout;

        VkDescriptorPool descriptorPool{};
        std::vector<VkDescriptorSet> descriptorSets{};

        std::vector<VkCommandBuffer> commandBuffers{};

        //sync objects
        //std::vector<VkSemaphore> imageAvailableSemaphores;
        //std::vector<VkSemaphore> renderFinishedSemaphores;
        //std::vector<VkFence> inFlightFences;

        //depth objects
        VkImage depthImage{};
        VkDeviceMemory depthImageMemory{};
        VkImageView depthImageView{};

        int currentFrame = 0;
        bool framebufferResized = false;
        int frameNumber = 0;
        FrameData frames[MAX_FRAMES_IN_FLIGHT];

        FrameData& get_current_frame() { return frames[frameNumber % MAX_FRAMES_IN_FLIGHT]; };

        VkPipelineLayout trianglePipelineLayout{};
        VkPipeline trianglePipeline{};

        DeletionQueue mainDeletionQueue{};


		Renderer();
		~Renderer();

		void Create(std::vector<void*> args, int width, int height);

        void BeginFrame();

        void SetViewport(int width, int height);

        VkCommandBuffer& GetCommandBuffer();

	private:
        bool resize_requested = false;
		void Init(int width, int height);
		void CreateSurfaceOnWindows(HWND hwnd, HINSTANCE hInstance);
        void CreateSwapchain(uint32_t width, uint32_t height);
        void ResizeSwapchain();
        void DestroySwapchain();
		void InitCommands();
		void InitSyncStructures();
        void CreateUniformBuffers();
		void InitDescriptors();
        void InitPipelines();
        void InitDefaultPipeline();
		void Destroy();



    public:

        uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
        void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, VkQueue queue);
        VkCommandBuffer  BeginSingleTimeCommands();
        void EndSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue);
        void CreateVertexBuffer(std::vector<Vertex>& vertices, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory);
        void CreateIndexBuffer(std::vector<uint16_t>& indices, VkBuffer& indexBuffer, VkDeviceMemory& indexBufferMemory);
        void ImmediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);
	};


}


#endif