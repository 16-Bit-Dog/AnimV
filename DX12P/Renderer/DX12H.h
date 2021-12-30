#pragma once
//finally I will try to use com_ptr's

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Windows Runtime Library. Needed for Microsoft::WRL::ComPtr<> template class.
#include <wrl.h>
using namespace Microsoft::WRL;

// DirectX 12 specific headers.
#include <d3d12.h>
#include <../Special_DX_Headers/d3dx12.h> //very good helper struct for DX12 - MSDN just refuses to teach raw stuff like D3D12_CPU_DESCRIPTOR_HANDLE prob due to it being a bad idea
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include <../imGUI/imgui.h>
#include <../imGUI/imgui_impl_glfw.h>
#include <../imGUI/imgui_impl_dx12.h>

using namespace DirectX;

struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT4 col;
};

void ThrowFailed(HRESULT v) {
    if (v != S_OK) {
        throw(v);
    }
}


struct MainDX12Objects {
   
    bool UseWarpDev = false;
   
    UINT Width;
    UINT Height;
    HWND hwnd;

    ComPtr<ID3D12DescriptorHeap> ImGUIHeap;

	static const UINT FrameCount = 2;

    struct RenderTargetObj {
        ComPtr<ID3D12Resource> m[FrameCount];
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle[FrameCount];

    };
    struct FrameContext
    {
        ID3D12CommandAllocator* commandAllocator;
        UINT64                  FenceValue;
    };

    //Pipeline Objects
    ComPtr<IDXGIFactory4> factory;
 
#ifdef _DEBUG
    ComPtr<ID3D12Debug> debugLayer;
#endif // DEBUG

    //for now heavily learned from MSDN docs
    HANDLE m_swapChainWaitableObject = NULL;
    D3D12_VIEWPORT m_viewport;
    D3D12_RECT m_scissorRect;
    ComPtr<ID3D12Device> m_device;
    RenderTargetObj m_renderTargets;
    ComPtr<IDXGISwapChain3> m_swapChain; //index 0 is main
    FrameContext FrameC[FrameCount];
    ComPtr<ID3D12CommandQueue> m_commandQueue;
    ComPtr<ID3D12RootSignature> m_rootSignature;
    ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
    ComPtr<ID3D12PipelineState> m_pipelineState;
    ComPtr<ID3D12GraphicsCommandList> m_commandList;
    UINT m_RTV_IS;
    UINT m_SRV_IS;
    UINT m_DSV_IS;
    UINT m_SAMP_IS;


    // App resources.
    ComPtr<ID3D12Resource> m_vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

    // Synchronization objects. - other than frame context
    UINT m_frameIndex;
    HANDLE m_fenceEvent;
    ComPtr<ID3D12Fence> m_fence;
    UINT64 m_lastFenceValue = 0;

    void CreateRenderTarget() {
        for (int n = 0; n < FrameCount; n++)
        {
            
            ThrowFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets.m[n])));
            
            m_device->CreateRenderTargetView(m_renderTargets.m[n].Get(), nullptr, m_renderTargets.rtvHandle[n]);
        }
    }

    void MakeImGUIHeap() {
        { //local scope creation
            D3D12_DESCRIPTOR_HEAP_DESC desc = {};
            desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
            desc.NumDescriptors = 1;
            desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            ThrowFailed(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&ImGUIHeap)));
                

        }
    }

    void SetupRendererDebugLayer() {

#ifdef _DEBUG
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer)))) {
            debugLayer->EnableDebugLayer();
        }
#endif

    }
    void SetupDXAdapter() {
        ThrowFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));

        if (UseWarpDev) {
            ComPtr<IDXGIAdapter> adapter;

            ThrowFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&adapter)));
            ThrowFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)));
        }
        else {
            ComPtr<IDXGIAdapter1> Hadapter;

            ThrowFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&Hadapter)));
            ThrowFailed(D3D12CreateDevice(Hadapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&m_device)));
        }

    }

    void MakeNewWindowSwapChainAndAssociate(HWND sHwnd, UINT sWidth, UINT sHeight) {

        Width = sWidth;
        Height = sHeight;
        hwnd = sHwnd;

        //create command queue

        D3D12_COMMAND_QUEUE_DESC queueDesc = {};
        queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
        queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
        ThrowFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

        // Describe and create the swap chain.
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
        swapChainDesc.BufferCount = FrameCount;
        swapChainDesc.BufferDesc.Width = sWidth;
        swapChainDesc.BufferDesc.Height = sHeight;
        swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_FRAME_LATENCY_WAITABLE_OBJECT;
        swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.OutputWindow = sHwnd;
        swapChainDesc.SampleDesc.Count = 1;
        swapChainDesc.Windowed = TRUE;


        ComPtr<IDXGISwapChain> swapChain;
        ThrowFailed(factory->CreateSwapChain(
            m_commandQueue.Get(),
            &swapChainDesc,
            &swapChain
        ));

        ThrowFailed(swapChain.As(&m_swapChain));

        //ThrowFailed(factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER)); I want to allow toggle of fullscreen and windowed mode

        m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

        { //this is just an automatic scope*
            // Describe and create a render target view (RTV) descriptor heap.
            D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
            rtvHeapDesc.NumDescriptors = FrameCount;
            rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
            rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            ThrowFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
        }

        // Create frame resources.
        {
            CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

            // Create a RTV for each frame.
            for (UINT n = 0; n < FrameCount; n++)
            {
                m_renderTargets.rtvHandle[n] = rtvHandle;

                rtvHandle.ptr += m_RTV_IS;

            }
        }
        for (int i = 0; i < FrameCount; i++)
            ThrowFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&FrameC[i].commandAllocator)));

        ThrowFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, FrameC[0].commandAllocator, NULL, IID_PPV_ARGS(&m_commandList)));

        ThrowFailed(m_commandList->Close());

        ThrowFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
           
        m_fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_fenceEvent == NULL)
            throw("fence event not made");

        m_swapChain->SetMaximumFrameLatency(FrameCount);
        m_swapChainWaitableObject = m_swapChain->GetFrameLatencyWaitableObject();

        MakeImGUIHeap();
        CreateRenderTarget();
    }

    void RemoveWindowSwapChainAndAssociate() {

    }

    void PreloadAssets() {

    }

    void GetDescHandleIncrements() {
        m_RTV_IS = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        m_SRV_IS = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        m_DSV_IS = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
        m_SAMP_IS = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

    }

	void RendererStartUpLogic() {
        SetupRendererDebugLayer();
        SetupDXAdapter();
        GetDescHandleIncrements();
    }

    FrameContext* WaitForNextFrameResources() {
        UINT nextFrameIndex = m_frameIndex + 1;
        m_frameIndex = nextFrameIndex;

        HANDLE waitableObjects[] = { m_swapChainWaitableObject, NULL };
        DWORD numWaitableObjects = 1;

        FrameContext* frameC = &FrameC[nextFrameIndex % FrameCount];
        UINT64 fenceValue = frameC->FenceValue;
        if (fenceValue != 0) // means no fence was signaled
        {
            frameC->FenceValue = 0;
            m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
            waitableObjects[1] = m_fenceEvent;
            numWaitableObjects = 2;
        }

        WaitForMultipleObjects(numWaitableObjects, waitableObjects, TRUE, INFINITE);

        return frameC;
    }

    void SetupAndSendimGUIData() {

        FrameContext* frameC = WaitForNextFrameResources(); //TODO: this frame calc for backbuffer
        UINT backBufferIdx = m_swapChain->GetCurrentBackBufferIndex();
        frameC->commandAllocator->Reset();

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = m_renderTargets.m[backBufferIdx].Get();
        barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        m_commandList->Reset(frameC->commandAllocator, NULL);
        m_commandList->ResourceBarrier(1, &barrier);

        // Render Dear ImGui graphics
        XMFLOAT4 clear_color = { 0.5,1,0,1 };
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        m_commandList->ClearRenderTargetView(m_renderTargets.rtvHandle[backBufferIdx], clear_color_with_alpha, 0, NULL);
        m_commandList->OMSetRenderTargets(1, &m_renderTargets.rtvHandle[backBufferIdx], FALSE, nullptr);
        //m_commandList->SetDescriptorHeaps(1, &ImGUIHeap);
        ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        m_commandList->ResourceBarrier(1, &barrier);
        ThrowFailed(m_commandList->Close());

        ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
        m_commandQueue->ExecuteCommandLists(1, ppCommandLists);

        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault(NULL, m_commandList.Get());

        m_swapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync

        UINT64 fenceValue = m_lastFenceValue + 1;
        m_commandQueue->Signal(m_fence.Get(), fenceValue);
        m_lastFenceValue = fenceValue;
        FrameC->FenceValue = fenceValue;

    }
}DXM;