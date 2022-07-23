#pragma once

// Graphics framework
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

// DirectX 12 API
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// DirectX 12 library linking
#pragma comment(lib, "dxgi")
#pragma comment(lib,"d3d12.lib")

#ifdef MYGAME_DEBUG
#define MYGAME_DIRECTX_DEBUG
#endif

#ifdef MYGAME_DIRECTX_DEBUG
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

namespace MyGame
{
	namespace DirectX
	{
		// DirectX data
		struct FrameContext
		{
			ID3D12CommandAllocator* CommandAllocator;
			UINT64                  FenceValue;
		};

		static int const                    NUM_FRAMES_IN_FLIGHT = 3;
		static FrameContext                 g_frameContext[NUM_FRAMES_IN_FLIGHT] = {};
		static UINT                         g_frameIndex = 0;

		static int const                    NUM_BACK_BUFFERS = 3;
		static ID3D12Device* g_pd3dDevice = NULL;
		static ID3D12DescriptorHeap* g_pd3dRtvDescHeap = NULL;
		static ID3D12DescriptorHeap* g_pd3dSrvDescHeap = NULL;
		static ID3D12CommandQueue* g_pd3dCommandQueue = NULL;
		static ID3D12GraphicsCommandList* g_pd3dCommandList = NULL;
		static ID3D12Fence* g_fence = NULL;
		static HANDLE                       g_fenceEvent = NULL;
		static UINT64                       g_fenceLastSignaledValue = 0;
		static IDXGISwapChain3* g_pSwapChain = NULL;
		static HANDLE                       g_hSwapChainWaitableObject = NULL;
		static ID3D12Resource* g_mainRenderTargetResource[NUM_BACK_BUFFERS] = {};
		static D3D12_CPU_DESCRIPTOR_HANDLE  g_mainRenderTargetDescriptor[NUM_BACK_BUFFERS] = {};

		// Forward declarations of helper functions
		bool CreateDeviceD3D(GLFWwindow*);
		void CleanupDeviceD3D();
		void CreateRenderTarget();
		void CleanupRenderTarget();
		void WaitForLastSubmittedFrame();
		FrameContext* WaitForNextFrameResources();
	}
}