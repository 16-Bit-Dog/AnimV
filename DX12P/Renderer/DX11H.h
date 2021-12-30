#pragma once
//finally I will try to use com_ptr's


#define WIN32_LEAN_AND_MEAN
// DirectX 11 & windows specific headers.
#include <Windows.h>
#include <dwmapi.h>
#include <d3d11.h>
#include <d3d11_3.h>
#include <d3d11_4.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <dxgi1_2.h>
#include <wrl.h>
#include <AtlBase.h>
#include <atlconv.h>
using namespace Microsoft::WRL;


#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "dxguid.lib")


#include <map>
#include <string>
#include <../Special_DX_Headers/ffmpegVideoWork.h>
#include <../imGUI/imgui.h>
#include <../imGUI/imgui_impl_glfw.h>
#include <../imGUI/imgui_impl_dx11.h>
#include <../Special_DX_Headers/WICTextureLoader.h>
#include <../Renderer/DX11ShaderFunc/ShaderFuncs.h>



using namespace DirectX;

struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT4 col;
};

struct MainDX11Objects {

    int SampleSize = 3; //+- 3 frame + 1 current

    bool UseWarpDev = false;

    UINT Width;
    UINT Height;
    HWND hwnd;
    GLFWwindow* window;

    bool bFullScreen = false;

    DXGI_RATIONAL refreshRateStatic;

    //Pipeline Objects
    
    ComPtr<ID3D11Device5> dxDevice = 0;
    ComPtr<ID3D11DeviceContext4> dxDeviceContext = 0;
    IDXGISwapChain1* dxSwapChain = 0;

    D3D11_DEPTH_STENCIL_VIEW_DESC dxDepthStencilDesc{
    DXGI_FORMAT_D32_FLOAT,D3D11_DSV_DIMENSION_TEXTURE2D
    };

    ID3D11RenderTargetView* dxRenderTargetView = nullptr;
    ID3D11Texture2D* dxDepthStencilBuffer = nullptr;
    ID3D11DepthStencilView* dxDepthStencilView = nullptr;
    ID3D11DepthStencilState* dxDepthStencilStateDefault = nullptr;

    ID3D11RasterizerState* dxRasterizerStateF = nullptr;
    ID3D11RasterizerState* dxRasterizerStateW = nullptr;

    D3D11_VIEWPORT dxViewport;

    ComPtr<IDXGIFactory2> dxFactory;
    ComPtr<IDXGIAdapter> dxAdapter;
    ComPtr<IDXGIDevice> dxGIDevice;

    DXGI_SWAP_CHAIN_DESC1 swapChainDescW; //reuse for when recreating swap chain and parts to resize screen params
    DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainDescF;

    void ClearBuffer(XMFLOAT4 p, bool ClearDepth) {
        float ClearColor[4] = { float(p.x), float(p.y), float(p.z), float(p.w) };
        dxDeviceContext->ClearRenderTargetView(dxRenderTargetView, ClearColor);

        if (ClearDepth) dxDeviceContext->ClearDepthStencilView(dxDepthStencilView, D3D11_CLEAR_DEPTH, 1, 0);
    }
    void DrawLogic() {

        dxDeviceContext->OMSetRenderTargets(1, &dxRenderTargetView, NULL);
        ClearBuffer({ 0.0f,0.5f,0.0f,1.0f }, true);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();

        dxSwapChain->Present(1, 0); // Present with vsync

    }
    void StopStallAndCheckPic(int frame) {
        if (FFMPEG.ShowOutputPicture) {
            CopyOutputPicture(frame);
        }
        DrawLogic();
        
    }

    ID3D11Resource* GetResourceOfUAVSRV(ID3D11View* u) {
        ID3D11Resource* r = nullptr;
        u->GetResource(&r);
        return r;
    }

    void CopyOutputPicture(int frame) {

        ID3D11Resource* rtvR = nullptr;
        dxRenderTargetView->GetResource(&rtvR);

        ID3D11Resource* Pic = nullptr;
        if (FFMPEG.ComputePixelChangeFrequency) {
            Pic = GetResourceOfUAVSRV(PixelFMap[frame]);
        }
        else{
            Pic = GetResourceOfUAVSRV(PerFrameRMap[frame]);
        }
        

        dxDeviceContext->CopyResource(rtvR, Pic);

    }

    void CreateSwapChainAndAssociate(DXGI_FORMAT format) {
        swapChainDescW.Width = Width;
        swapChainDescW.Height = Height;
        swapChainDescW.BufferCount = 2;
        swapChainDescW.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDescW.Format = format;
        swapChainDescW.SampleDesc.Count = 1;
        swapChainDescW.SampleDesc.Quality = 0;
        swapChainDescW.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDescW.Scaling = DXGI_SCALING_NONE;

        swapChainDescF.Scaling = DXGI_MODE_SCALING_STRETCHED;
        swapChainDescF.RefreshRate = refreshRateStatic;
        swapChainDescF.Windowed = !bFullScreen;

        dxFactory->CreateSwapChainForHwnd(dxDevice.Get(), hwnd, &swapChainDescW, &swapChainDescF, NULL, &dxSwapChain);

        ID3D11Texture2D* backBuffer;
        dxSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);

        if (dxRenderTargetView != nullptr) {
            SafeRelease(dxRenderTargetView);
        }

        dxDevice->CreateRenderTargetView(
            backBuffer,
            nullptr,
            &dxRenderTargetView);

        SafeRelease(backBuffer);

        D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
        ZeroMemory(&depthStencilBufferDesc, sizeof(D3D11_TEXTURE2D_DESC));

        depthStencilBufferDesc.ArraySize = 1;
        depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
        depthStencilBufferDesc.CPUAccessFlags = 0;
        depthStencilBufferDesc.Format = DXGI_FORMAT_R32_TYPELESS;
        depthStencilBufferDesc.Width = Width;
        depthStencilBufferDesc.Height = Height;
        depthStencilBufferDesc.MipLevels = 1;

        depthStencilBufferDesc.SampleDesc.Count = 1;
        depthStencilBufferDesc.SampleDesc.Quality = 0;

        depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;

        dxDevice->CreateTexture2D(
            &depthStencilBufferDesc,
            nullptr,
            &dxDepthStencilBuffer);

        dxDevice->CreateDepthStencilView(
            dxDepthStencilBuffer,
            &dxDepthStencilDesc,
            &dxDepthStencilView);

        dxViewport.Width = Width;
        dxViewport.Height = Height;
        dxViewport.TopLeftX = 0.0f;
        dxViewport.TopLeftY = 0.0f;
        dxViewport.MinDepth = 0.0f;
        dxViewport.MaxDepth = 1.0f;
    }

    void MakeNewWindowSwapChainAndAssociate(GLFWwindow* windowW, HWND sHwnd, UINT sWidth, UINT sHeight) {
        window = windowW;
        Width = sWidth;
        Height = sHeight;
        hwnd = sHwnd;

        DEVMODEA WinMonitorInfo;

        EnumDisplaySettingsA(NULL, ENUM_CURRENT_SETTINGS, &WinMonitorInfo);

        refreshRateStatic.Numerator = WinMonitorInfo.dmDisplayFrequency;
        refreshRateStatic.Denominator = 1;

        CreateSwapChainAndAssociate(DXGI_FORMAT_R8G8B8A8_UNORM);

        D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
        ZeroMemory(&depthStencilStateDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));


        depthStencilStateDesc.DepthEnable = FALSE; //this is a bad idea...
        depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
        depthStencilStateDesc.StencilEnable = FALSE;


        dxDevice->CreateDepthStencilState(&depthStencilStateDesc, &dxDepthStencilStateDefault);

        D3D11_RASTERIZER_DESC rasterizerDesc;
        ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));

        rasterizerDesc.AntialiasedLineEnable = FALSE;
        rasterizerDesc.CullMode = D3D11_CULL_FRONT;
        rasterizerDesc.DepthBias = 0;
        rasterizerDesc.DepthBiasClamp = 0.0f;
        rasterizerDesc.DepthClipEnable = false;
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.MultisampleEnable = FALSE;
        rasterizerDesc.ScissorEnable = FALSE;
        rasterizerDesc.SlopeScaledDepthBias = 0.0f;

        dxDevice->CreateRasterizerState(
            &rasterizerDesc,
            &dxRasterizerStateF);

        rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;

        dxDevice->CreateRasterizerState(
            &rasterizerDesc,
            &dxRasterizerStateW);
    }


    std::map<int, ID3D11ShaderResourceView*> PerFrameRMap;
    std::map<int, ID3D11UnorderedAccessView*> PixelFMap;
    

    int BLOCK_SIZE = 16;

    struct ConstantsDataStruct {
        int DepthSizeX;
        int DepthSizeY;
        UINT pad4 = 1;
        UINT pad1 = 1;
        
        UINT numThreadGroups[3];
        UINT pad2 = 1;

        UINT numThreads[3];
        UINT pad3 = 1;

    }CDataS;
    
    ID3D11Buffer* Constants;

    ID3D11ComputeShader* PixelFrequencyCalc;




    void CreatePixelFrequencyCalcShader() {
        const std::string s = 
            "#define BLOCK_SIZE " + std::to_string(BLOCK_SIZE) + "\n"
            
            "cbuffer ConstData : register(b0){\n"
            "int DepthSizeX;\n"
            "int DepthSizeY;\n"
            "UINT pad4;\n"
            "UINT pad1;\n"
            "UINT[3] numTG;\n"
            "UINT pad2;\n"
            "UINT[3] TG;\n"
            "UINT pad3;\n"
            "}\n"

            "RWTexture2D<float4> OutTexture : register(u0);\n"
            
            "Texture2D tex0 : register(t0); \n"//compare texture is 0 
            "Texture2D tex1 : register(t1); \n"
            "Texture2D tex2 : register(t2); \n"
            "Texture2D tex3 : register(t3); \n"
            "Texture2D tex4 : register(t4); \n"
            "Texture2D tex5 : register(t5); \n"
            "Texture2D tex6 : register(t6); \n"
            "Texture2D tex7 : register(t7); \n"
            "Texture2D tex8 : register(t8); \n"
            "Texture2D tex9 : register(t9); \n"
            "Texture2D tex10 : register(t10); \n"

            "struct ComputeShaderInput\n"
            "{\n"
            "uint3 groupID : SV_GroupID;           // 3D index of the thread group in the dispatch.\n"
            "uint3 groupThreadID : SV_GroupThreadID;     // 3D index of local thread ID in a thread group.\n"
            "uint3 dispatchThreadID : SV_DispatchThreadID;  // 3D index of global thread ID in the dispatch.\n"
            "uint  groupIndex : SV_GroupIndex;        // Flattened local index of the thread within a thread group.\n"
            "};\n"

            "[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]\n"
            "void CS_main(ComputeShaderInput IN){\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "}\n"
            
            
            ;

        PixelFrequencyCalc = LoadShader<ID3D11ComputeShader>(&s, "CS_main", "latest", dxDevice.Get());

    }
    void RunPixelFrequency(ID3D11UnorderedAccessView* uav, ID3D11ShaderResourceView* srv[10]) {
        dxDeviceContext->CSSetUnorderedAccessViews(0,1,&uav,0);
        
        dxDeviceContext->CSSetShaderResources(0, 10, srv);

        dxDeviceContext->CSSetConstantBuffers(0, 1, &Constants);

        dxDeviceContext->Dispatch(CDataS.numThreadGroups[0], CDataS.numThreadGroups[1], CDataS.numThreadGroups[2]);

    }

    void CreateConstantBuf() {
        D3D11_BUFFER_DESC bufDesc;
        ZeroMemory(&bufDesc, sizeof(bufDesc));
        bufDesc.Usage = D3D11_USAGE_DEFAULT;
        bufDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        bufDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
        bufDesc.CPUAccessFlags = 0;
        bufDesc.ByteWidth = sizeof(ConstantsDataStruct);
        bufDesc.StructureByteStride = sizeof(UINT);
        dxDevice->CreateBuffer(&bufDesc, nullptr, &Constants);
    }
    void UpdateConstantBuf() {
        dxDeviceContext->UpdateSubresource(Constants, 0, nullptr, &CDataS, 0, 0);
    }
    void ResizeWindowAndViewport(float NewWidth, float NewHeight, DXGI_FORMAT format) {
        glfwSetWindowSize(window, NewWidth, NewHeight);
        Width = NewWidth;
        Height = NewHeight;
        CreateSwapChainAndAssociate(format);

    }
    void MakeShadersAndConstantsData(ID3D11ShaderResourceView* srv) {
        D3D11_TEXTURE2D_DESC d;
        ComPtr<ID3D11Resource> r;
        srv->GetResource(&r);
        ComPtr<ID3D11Texture2D> tex;
        r.As(&tex);

        tex->GetDesc(&d);
        
        CDataS.DepthSizeX = d.Width;
        CDataS.DepthSizeY = d.Height;

        ResizeWindowAndViewport(d.Width, d.Height, d.Format);

        UINT xG = floor(float(CDataS.DepthSizeX) / BLOCK_SIZE);
        UINT yG = floor(float(CDataS.DepthSizeY) / BLOCK_SIZE);

        CDataS.numThreadGroups[0] = xG;
        CDataS.numThreadGroups[1] = yG;
        CDataS.numThreadGroups[2] = 1;
        

        CDataS.numThreads[0] = xG * BLOCK_SIZE;
        CDataS.numThreads[1] = yG * BLOCK_SIZE;
        CDataS.numThreads[2] = 1;
        
        UpdateConstantBuf();

        CreatePixelFrequencyCalcShader();

    }

    void addZeroes(std::string* s, int val) {
        for (int i = 0; i < val; i++) {
            *s += "0";
        }
    }

    bool AddMainResourceToMapFromFile(std::string* psp, int val) {

        std::string TrailZeros = "";
        addZeroes(&TrailZeros, (8 - std::to_string(val).length()));
        TrailZeros += std::to_string(val) + ".png";

        std::string workS = *psp + TrailZeros;
        if (fs::exists(workS)) {
            GetTextureFromPathIntoMap(&workS, val);

            val += 1;
            TrailZeros = "";
            addZeroes(&TrailZeros, (8 - std::to_string(val).length()));
            TrailZeros += std::to_string(val) + ".png";

            return true;
        }
        return false;
    }

    void ComputePassLogicFrame(int i) {
        int total = SampleSize * 2 + 1;
        int cur = i;
        int bottom = i - 3;
        int top = i + 3;
        int filled = 1;

        std::vector<ID3D11ShaderResourceView*> srv(total); //the 10 srvs - 0 is curr frame
        srv[0] = PerFrameRMap[cur];
        if (bottom < 0) {
            bottom = cur + 1;
            top = bottom + SampleSize * 2;
        }
        if (top > PerFrameRMap.size()) {

            top = cur - 1;
            bottom = top - SampleSize * 2;
        }


        std::map<int, ID3D11ShaderResourceView*> InUseData; //remove not needed data
        if(!FFMPEG.CacheVideoImages)
        InUseData[cur] = PerFrameRMap[cur];

        for (int x = bottom; x < top; x++) {
            if (x != cur) {
                
                if (!FFMPEG.CacheVideoImages)
                    InUseData[cur] = PerFrameRMap[x];

                srv[filled] = PerFrameRMap[x];
                filled += 1;
            }
        }

        if (!FFMPEG.CacheVideoImages) {
            //clean not used data for non cached work
            for (auto i : PerFrameRMap) {
                if (InUseData.count(i.first) == 0) {
                    ID3D11Resource* r = GetResourceOfUAVSRV(i.second);
                    SafeRelease(r);
                    PerFrameRMap.erase(i.first);
                }
            }

        }



        if (FFMPEG.ComputePixelChangeFrequency) {
            MakeEmptyUAVfromSRVIntoMap(&PixelFMap, PerFrameRMap[i], i);

            RunPixelFrequency(PixelFMap[i], srv.data());
        }
        //

        StopStallAndCheckPic(i);

        i += 1;
    }

    void CleanCacheResourceMap(std::map<int, ID3D11ShaderResourceView*>* m) {
        for (auto i : (*m) ) {
            ID3D11Resource* TexO = nullptr;
            i.second->GetResource(&TexO);
            SafeRelease(TexO);
        }
        PerFrameRMap.clear();
    }

    void StartComputePass(){
        std::string psp = FFMPEG.filePathNameStore + FFMPEG.imageSplit;

        if (FFMPEG.CacheVideoImages) {
            int val = 1;
            while (AddMainResourceToMapFromFile(&psp, val)) {
                val += 1;
            }
        }
        else {
            int val = 1;
            AddMainResourceToMapFromFile(&psp, val);
        }
        
        CreateConstantBuf();
        MakeShadersAndConstantsData(PerFrameRMap[1]);

        int i = 1;
        if (FFMPEG.CacheVideoImages) {
            while (i < PerFrameRMap.size() + 1) { //first few start and end frames are not very accurate due to more forward data-
                //t0 is promised to be main frame, t1-4 is promised to be less than (normally) main frame, and t5-9 is more than main frame
                ComputePassLogicFrame(i);
                i += 1;
            }
        }
        else {
            ComputePassLogicFrame(i); //for frame 1
            i += 1;
            while (AddMainResourceToMapFromFile(&psp, i)) {
                ComputePassLogicFrame(i);
                i += 1;
            }
        }
        
        CleanCacheResourceMap(&PerFrameRMap);
        SafeRelease(Constants);
        Constants = nullptr;
        
    }
    

    void MakeEmptyUAVfromSRVIntoMap(std::map<int, ID3D11UnorderedAccessView*>* m, ID3D11ShaderResourceView* srv, int frame) {
        if (m->count(frame) > 0) {
            ID3D11Resource* TexO = nullptr;
            (*m)[frame]->GetResource(&TexO);
            SafeRelease(TexO);
        }
        (*m)[frame] = MakeEmptyUAVfromSRV(srv);
    }

    ID3D11UnorderedAccessView* MakeEmptyUAVfromSRV(ID3D11ShaderResourceView* srv) {
        D3D11_TEXTURE2D_DESC d;
        ComPtr<ID3D11Resource> r;
        srv->GetResource(&r);
        ComPtr<ID3D11Texture2D> tex;
        r.As(&tex);

        tex->GetDesc(&d);
        
        ID3D11Texture2D* texUAV;

        dxDevice->CreateTexture2D(&d, NULL, &texUAV);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavs;
        uavs.Format = d.Format;
        uavs.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
            
        ID3D11UnorderedAccessView* uav;

        dxDevice->CreateUnorderedAccessView(texUAV,&uavs,&uav);

        return uav;
    }

    void GetTextureFromPathIntoMap(std::string* path, int frame) {
        ID3D11ShaderResourceView* srv = GetTextureFromPath(path);
        if (PerFrameRMap.count(frame) > 0) {
            ID3D11Resource* TexO = nullptr;
            PerFrameRMap[frame]->GetResource(&TexO);
            SafeRelease(TexO);
        }
        PerFrameRMap[frame] = srv;
    }

    ID3D11ShaderResourceView* GetTextureFromPath(std::string* path) {
        ID3D11Resource* tex = nullptr;
        ID3D11ShaderResourceView* srv = nullptr;

        CA2W ca2w(path->c_str());
        LPWSTR ws = LPWSTR(ca2w);
        
        CreateWICTextureFromFile(dxDevice.Get(), dxDeviceContext.Get(),
            ws, &tex, &srv, UINTMAX_MAX);
               
        return srv;

    }

    void ImGUIInit() {
        ImGui_ImplDX11_Init(dxDevice.Get(), dxDeviceContext.Get());
    }

    void ImGUINewFrameLogic(){
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void NewDX11Object() {
        UINT createDeviceFlags = 0; //D3D11_CREATE_DEVICE_BGRA_SUPPORT should also be here if need be
#if _DEBUG
        createDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

        D3D_FEATURE_LEVEL featureLevels[] =
        {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };

        ComPtr<ID3D11Device> dev;
        ComPtr<ID3D11DeviceContext> devC;
        D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE, //you always can use the software accelerator  :)
            nullptr,
            createDeviceFlags,
            featureLevels,
            _countof(featureLevels),
            D3D11_SDK_VERSION,
            &dev,
            nullptr,
            &devC);

        ThrowFailed(dev.As(&dxDevice));
        ThrowFailed(devC.As(&dxDeviceContext));

        dxDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxGIDevice);



        dxGIDevice->GetAdapter(&dxAdapter);

        dxAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxFactory);
    }
    
   
    void RendererStartUpLogic() {
        NewDX11Object();

    }


};
MainDX11Objects DXM;