#pragma once
//finally I will try to use com_ptr's

//TODO: fix memory leak I have when cleaning images from map?!?
//TODO: add live change programming and "show output" - call it "pass1""pass2" as folders, and it pulls from these to compile result video
//also have "display video <-- split video has same formating for name as ffmpeg"

#define WIN32_LEAN_AND_MEAN
const int ZERO_FILE_COUNT = 8;
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
#include <objidl.h>
#include <wincodec.h>
#include <algorithm>
//#include <gdiplus.h>
//using namespace Gdiplus;
//#pragma comment (lib,"Gdiplus.lib")

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
#include <../Special_DX_Headers/textureCap/ScreenGrab11.h>

#include <../Renderer/DX11ShaderFunc/ShaderFuncs.h>



using namespace DirectX;

struct Vertex {
	XMFLOAT3 pos;
	XMFLOAT4 col;
};

struct MainDX11Objects {
    bool ClearRTV = true;

    int CurrentFrameRate = 24;

    int TargetFrameRate = 48;

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
    IDXGISwapChain1* dxSwapChain = nullptr;

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
    
    void addZeroes(std::string* s, int val) {
        for (int i = 0; i < val; i++) {
            *s += "0";
        }
    }

    void ClearBuffer(XMFLOAT4 p, bool ClearDepth) {
        float ClearColor[4] = { float(p.x), float(p.y), float(p.z), float(p.w) };
        dxDeviceContext->ClearRenderTargetView(dxRenderTargetView, ClearColor);

        if (ClearDepth) dxDeviceContext->ClearDepthStencilView(dxDepthStencilView, D3D11_CLEAR_DEPTH, 1, 0);
    }
    void DrawLogic(bool sync) {

        if (NewImGUIDat) {
            ImGui::Render();
        }
        dxDeviceContext->OMSetRenderTargets(1, &dxRenderTargetView, NULL);
        if (ClearRTV == true) {
            ClearBuffer({ 0.0f,0.5f,0.0f,1.0f }, true);
        }
        ClearRTV = true;

        if (NewImGUIDat) {
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        dxSwapChain->Present(sync, 0); // Present with vsync

        NewImGUIDat = false;
    }
    void StopStallAndCheckPic(int frame, ID3D11View* v) {
        if (FFMPEG.ShowOutputPicture) {
            CopyOutputPicture(frame, v);
        }
        glfwPollEvents();
        DrawLogic(false);
        
    }

    ID3D11Resource* GetResourceOfUAVSRV(ID3D11View* u) {
        ID3D11Resource* r = nullptr;
        u->GetResource(&r);
        return r;
    }
    void CleanDir(std::string* Fpath) {
        if (fs::exists(*Fpath)) {
            fs::remove_all(*Fpath);
        }
        if (fs::is_directory(*Fpath) == false) {
            fs::create_directory(*Fpath);
        }

    }
    void CopyOutputPicture(int frame, ID3D11View* v) {

        ID3D11Resource* rtvR = nullptr;
        dxRenderTargetView->GetResource(&rtvR);

        ComPtr<ID3D11Resource> Pic;
        Pic = GetResourceOfUAVSRV(v);
        D3D11_TEXTURE2D_DESC d;
        ComPtr<ID3D11Texture2D> tex;
        Pic.As(&tex);

        SafeRelease(Pic);

        tex->GetDesc(&d);

        //dxDeviceContext->CopyResource(rtvR, Pic);
        dxDeviceContext->ResolveSubresource(rtvR,0,Pic.Get(),0,d.Format); //ignore mip copy
        

        ClearRTV = false;
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
        
        ID3D11Texture2D* backBuffer = nullptr;
        if (dxSwapChain != nullptr) {
            dxSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
        }
        if (backBuffer != nullptr) {
            SafeRelease(backBuffer);
        }
        if (dxRenderTargetView != nullptr) {
            SafeRelease(dxRenderTargetView);
        }
        if ((dxDepthStencilBuffer) == nullptr) {
            SafeRelease(dxDepthStencilBuffer);
        }

        if (dxSwapChain != nullptr) {
            dxDeviceContext->ClearState();
            dxDeviceContext->Flush();

            dxSwapChain->ResizeBuffers(0, Width, Height, format, swapChainDescW.Flags);
        }
        else {
            ThrowFailed(dxFactory->CreateSwapChainForHwnd(dxDevice.Get(), hwnd, &swapChainDescW, &swapChainDescF, NULL, &dxSwapChain));
        }

        backBuffer;
        dxSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
    
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
    std::vector<std::map<int, ID3D11UnorderedAccessView*>> PixelFMap;
    

    int BLOCK_SIZE = 16;
    
    bool NewImGUIDat = false;

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
    
    struct ConstantFrameData {
        float CurrentFrameRatio;
        float pad1 = 0.0f;
        float pad2 = 0.0f;
        float pad3 = 0.0f;
    }CDataSFrame;

    ID3D11Buffer* Constants;

    ID3D11Buffer* ConstantsFrameDat;

    ID3D11ComputeShader* PixelFrequencyCalc;

    ID3D11ComputeShader* FinalComputeLogic1;

    ID3D11ComputeShader* FinalComputeLogicSoftBody1;

    void SaveUAVTexToFile(std::map<int, ID3D11UnorderedAccessView*>* m, std::string* Fpath) {

        //start file saving
        for (auto& x : *m) {
            std::string TrailZeros = "";
            addZeroes(&TrailZeros, (ZERO_FILE_COUNT - std::to_string(x.first).length()));
            TrailZeros += std::to_string(x.first) + ".png";
            std::string FName = (*Fpath) + TrailZeros;

            CA2W ca2w(FName.c_str());
            LPWSTR ws = LPWSTR(ca2w);

            ID3D11Resource* r;
            r = GetResourceOfUAVSRV(x.second);

            ThrowFailed(SaveWICTextureToFile(dxDeviceContext.Get(), r, GUID_ContainerFormatPng, ws));
            SafeRelease(r);
        }

    }

    void CreatePixelFrequencyCalcShader() {
        if (PixelFrequencyCalc != nullptr) SafeRelease(PixelFrequencyCalc);

        const std::string s = 
            "#define BLOCK_SIZE " + std::to_string(BLOCK_SIZE) + "\n"
            
            "cbuffer ConstData : register(b0){\n"
            "int DepthSizeX;\n"
            "int DepthSizeY;\n"
            "uint pad4;\n"
            "uint pad1;\n"
            "uint3 numTG;\n"
            "uint pad2;\n"
            "uint3 TG;\n"
            "uint pad3;\n"
            "}\n"

            "RWTexture2D<float4> OutT : register(u0);\n"
            "#define SampleSize "+ std::to_string(SampleSize*2+1)+"\n"
            "Texture2D tex[SampleSize] : register(t0); \n"//compare texture is 0, rest is extra 


            "struct ComputeShaderInput\n"
            "{\n"
            "uint3 groupID : SV_GroupID;           // 3D index of the thread group in the dispatch.\n"
            "uint3 groupThreadID : SV_GroupThreadID;     // 3D index of local thread ID in a thread group.\n"
            "uint3 dispatchThreadID : SV_DispatchThreadID;  // 3D index of global thread ID in the dispatch.\n"
            "uint  groupIndex : SV_GroupIndex;        // Flattened local index of the thread within a thread group.\n"
            "};\n"

            "[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]\n"
            "void CS_main(ComputeShaderInput IN){\n"
            "int2 texCoord = IN.dispatchThreadID.xy;"
            "OutT[IN.dispatchThreadID.xy] = tex[0].Load(int3(texCoord,0));"
            "[unroll]"
            "for(int i = 1; i < SampleSize; i++){\n"
            "OutT[IN.dispatchThreadID.xy] -= tex[i].Load(int3(texCoord,0))/SampleSize;\n"
            "\n"
            "}\n"
            "OutT[IN.dispatchThreadID.xy]=OutT[IN.dispatchThreadID.xy]/SampleSize;\n"
            "\n"
            "\n"
            "}\n"            
            ;

        PixelFrequencyCalc = LoadShader<ID3D11ComputeShader>(&s, "CS_main", "latest", dxDevice.Get());

    }
    void CreateFinalComputeLogicSoftBody1Shader() {
        if (FinalComputeLogicSoftBody1 != nullptr) SafeRelease(FinalComputeLogicSoftBody1);

        const std::string s =
            "#define BLOCK_SIZE " + std::to_string(BLOCK_SIZE) + "\n"

            "cbuffer ConstDataFrame : register(b0){\n"
            "float FrameRatio;\n"
            "}\n"

            "RWTexture2D<float4> OutT : register(u0);\n"
            "#define SampleSize " + std::to_string(SampleSize * 2 + 1) + "\n"
            "Texture2D tex[SampleSize] : register(t0); \n"//compare texture is 0, rest is extra 
            "Texture2D texFZero[SampleSize] : register(t" + std::to_string(SampleSize * 2 + 1) + ");\n"

            "struct ComputeShaderInput\n"
            "{\n"
            "uint3 groupID : SV_GroupID;           // 3D index of the thread group in the dispatch.\n"
            "uint3 groupThreadID : SV_GroupThreadID;     // 3D index of local thread ID in a thread group.\n"
            "uint3 dispatchThreadID : SV_DispatchThreadID;  // 3D index of global thread ID in the dispatch.\n"
            "uint  groupIndex : SV_GroupIndex;        // Flattened local index of the thread within a thread group.\n"
            "};\n"

            "[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]\n"
            "void CS_main(ComputeShaderInput IN){\n"
            //uniform branch is cheap - im not worried here
            "int2 texC = IN.dispatchThreadID.xy;\n"
            "float Fr = FrameRatio;"
            "\n"
            "OutT[texC] = tex[0].Load(int3(texC,0))*Fr+tex[2].Load(int3(texC,0))*(1-Fr); \n"
            "\n"
            "\n"
            "}\n"
            ;

        FinalComputeLogicSoftBody1 = LoadShader<ID3D11ComputeShader>(&s, "CS_main", "latest", dxDevice.Get());
    }
    void CreateFinalComputeLogic1Shader() {
        if (FinalComputeLogic1 != nullptr) SafeRelease(FinalComputeLogic1);

        const std::string s =
            "#define BLOCK_SIZE " + std::to_string(BLOCK_SIZE) + "\n"

            "cbuffer ConstDataFrame : register(b0){\n"
            "float FrameRatio;\n"
            "}\n"

            "RWTexture2D<float4> OutT : register(u0);\n"
            "#define SampleSize " + std::to_string(SampleSize * 2 + 1) + "\n"
            "Texture2D tex[SampleSize] : register(t0); \n"//compare texture is 0, rest is extra 
            
            "struct ComputeShaderInput\n"
            "{\n"
            "uint3 groupID : SV_GroupID;           // 3D index of the thread group in the dispatch.\n"
            "uint3 groupThreadID : SV_GroupThreadID;     // 3D index of local thread ID in a thread group.\n"
            "uint3 dispatchThreadID : SV_DispatchThreadID;  // 3D index of global thread ID in the dispatch.\n"
            "uint  groupIndex : SV_GroupIndex;        // Flattened local index of the thread within a thread group.\n"
            "};\n"

            "[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]\n"
            "void CS_main(ComputeShaderInput IN){\n"
            //uniform branch is cheap - im not worried here
            "int2 texC = IN.dispatchThreadID.xy;\n"
            "float Fr = FrameRatio;"
            "\n"
            "OutT[texC] = tex[0].Load(int3(texC,0))*Fr+tex[2].Load(int3(texC,0))*(1-Fr); \n"
            "\n"
            "\n"
            "}\n"
            ;

        FinalComputeLogic1 = LoadShader<ID3D11ComputeShader>(&s, "CS_main", "latest", dxDevice.Get());
    }

    void RunSoftBodyComputePass(ID3D11UnorderedAccessView* uav, std::map<int, std::vector<ID3D11ShaderResourceView*>>* srv, bool HasSetVar = false) {

        dxDeviceContext->CSSetUnorderedAccessViews(0, 1, &uav, 0);

        if (HasSetVar = false) {
            dxDeviceContext->CSSetShader(FinalComputeLogicSoftBody1, 0, 0);

            for (int i = 0; i < srv->size(); i++) {
                dxDeviceContext->CSSetShaderResources((*srv)[i].size() * i, (*srv)[i].size(), (*srv)[i].data());
            }
        }

        dxDeviceContext->CSSetConstantBuffers(0, 1, &ConstantsFrameDat);

        dxDeviceContext->Dispatch(CDataS.numThreadGroups[0], CDataS.numThreadGroups[1], CDataS.numThreadGroups[2]);

        srv->clear();

        ID3D11UnorderedAccessView* uavt = nullptr;

        dxDeviceContext->CSSetUnorderedAccessViews(0, 1, &uavt, 0);
    }
    void RunImageComputePass(ID3D11UnorderedAccessView* uav, std::map<int,std::vector<ID3D11ShaderResourceView*>>* srv, bool HasSetVar = false) {
        
        dxDeviceContext->CSSetUnorderedAccessViews(0, 1, &uav, 0);
        
        if (HasSetVar == false) {
            dxDeviceContext->CSSetShader(FinalComputeLogic1, 0, 0);

            for (int i = 0; i < srv->size(); i++) {
                dxDeviceContext->CSSetShaderResources((*srv)[i].size() * i, (*srv)[i].size(), (*srv)[i].data());
            }
        }

        dxDeviceContext->CSSetConstantBuffers(0, 1, &ConstantsFrameDat);

        dxDeviceContext->Dispatch(CDataS.numThreadGroups[0], CDataS.numThreadGroups[1], CDataS.numThreadGroups[2]);

        srv->clear();

        ID3D11UnorderedAccessView* uavt = nullptr;

        dxDeviceContext->CSSetUnorderedAccessViews(0, 1, &uavt, 0);

    }
    void RunPixelFrequency(ID3D11UnorderedAccessView* uav, std::vector<ID3D11ShaderResourceView*>* srv) {
        dxDeviceContext->CSSetUnorderedAccessViews(0,1,&uav,0);
        
        dxDeviceContext->CSSetShader(PixelFrequencyCalc,0,0);

        dxDeviceContext->CSSetShaderResources(0, srv->size(), srv->data());

        dxDeviceContext->CSSetConstantBuffers(0, 1, &Constants);

        dxDeviceContext->Dispatch(CDataS.numThreadGroups[0], CDataS.numThreadGroups[1], CDataS.numThreadGroups[2]);

        srv->clear();

        ID3D11UnorderedAccessView* uavt = nullptr;

        dxDeviceContext->CSSetUnorderedAccessViews(0, 1, &uavt, 0);

    }

    void CreateConstantBuf() {
        D3D11_BUFFER_DESC bufDesc;
        ZeroMemory(&bufDesc, sizeof(bufDesc));
        bufDesc.Usage = D3D11_USAGE_DEFAULT;
        bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufDesc.ByteWidth = sizeof(ConstantsDataStruct);
        bufDesc.StructureByteStride = sizeof(UINT);
        dxDevice->CreateBuffer(&bufDesc, nullptr, &Constants);

        bufDesc.ByteWidth = sizeof(ConstantFrameData);

        dxDevice->CreateBuffer(&bufDesc, nullptr, &ConstantsFrameDat);


    }
    void UpdateConstantBuf() {
        dxDeviceContext->UpdateSubresource(Constants, 0, nullptr, &CDataS, 0, 0);
    }
    void UpdateFrameConstantBuf(float dat) { // how close from frame 0.0f-1.0f [[1] is before--[0] is after], and 1.001f-2.0f[[0] is before-- [2] is after] 
        CDataSFrame.CurrentFrameRatio = dat;
        dxDeviceContext->UpdateSubresource(ConstantsFrameDat, 0, nullptr, &CDataSFrame, 0, 0);
    }

    void ResizeWindowAndViewport(float NewWidth, float NewHeight, DXGI_FORMAT format) {
        glfwSetWindowSize(window, NewWidth, NewHeight);
        Width = NewWidth;
        Height = NewHeight;
        CreateSwapChainAndAssociate(format);
        DrawLogic(true);
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
        
        if(FFMPEG.CompileLazyIntrop)
            CreateFinalComputeLogic1Shader();
        else if (FFMPEG.CompileSoftBodyIntrop) {
            CreateFinalComputeLogicSoftBody1Shader();
        }
        else {
            CreateFinalComputeLogic1Shader();
        }

    }

    bool AddMainResourceToMapFromFile(std::string* psp, int val, std::map<int, ID3D11ShaderResourceView*>* PerFrameRMap) {

        std::string TrailZeros = "";
        addZeroes(&TrailZeros, (ZERO_FILE_COUNT - std::to_string(val).length()));
        TrailZeros += std::to_string(val) + ".png";

        std::string workS = *psp + TrailZeros;
        if (fs::exists(workS)) {
            GetTextureFromPathIntoMap(&workS, val, PerFrameRMap);

            val += 1;
            TrailZeros = "";
            addZeroes(&TrailZeros, (ZERO_FILE_COUNT - std::to_string(val).length()));
            TrailZeros += std::to_string(val) + ".png";

            return true;
        }
        return false;
    }

    bool CheckForMusic(std::string* fsM) {
                
        return fs::exists(*fsM + FFMPEG.musicName);

    }

    int GetSplitPicCount(std::string* fsS) {
        int i = 0;
        for (auto& p : fs::directory_iterator(*fsS)) {
            if(p.path().string().find(".png") != std::string::npos)
            i++;
        }
        return i;
    }

    void RunFinalImageCreationPassFrame(int i, std::map<int, std::map<int, ID3D11ShaderResourceView*>>* srvM, int& SplitPicCount, int& RealFrameCount, float& fRat) { //0 is reg, remember

        //[0] = current [1] = before [2] = after

        int total = SampleSize * 2 + 1;
        int cur = i;
        int bottom = i - SampleSize;
        int top = i + SampleSize;
        int filled = 1;

        std::map<int, std::vector<ID3D11ShaderResourceView*>> srv; //the srvs + 1 for main image, 0 is reg, rest are in order map stuff from folders
        for (int x = 0; x < srvM->size(); x++) {
            srv[x].resize(total);
        }
        for (int x = 0; x < srvM->size(); x++) {
            srv[x][0] = (*srvM)[x][cur];
        }
        if (bottom < 1) {
            int SetBottom = cur;
            for (int y = SampleSize; y > -SampleSize; y--) {
                if (SetBottom - y > 0) {
                    bottom = cur - y;
                    top = cur + (SampleSize - y) + SampleSize;
                    y = -SampleSize;
                }
            }
        }
        else if ((*srvM)[0].count(top) == 0) { // unbound data, so I need to map count - unless I pre pass max size - won't run for pre cache, so only checks in CPU cache around 7-10 items, so this is very fast despite looking to have a poor worst case' worst case will never be reached
            int SetTop = cur;
            for (int y = SampleSize; y > -SampleSize; y--) {
                if ((*srvM)[0].count(SetTop + y) > 0) {
                    top = cur + y;
                    bottom = cur - (SampleSize - y) - SampleSize;
                    y = -SampleSize;
                }
            }
        }


        std::map<int,std::map<int, ID3D11ShaderResourceView*>> InUseData; //remove not needed data
        
        for (auto& x : (*srvM)) {
            InUseData[x.first][cur] = x.second[cur];
        }

        int Prefilled = filled;

        for (auto& y : (*srvM)) {
            filled = Prefilled;
            for (int x = bottom; x < top + 1; x++) {
                if (x != cur) {

                    InUseData[y.first][x] = y.second[x];

                    srv[y.first][filled] = y.second[x];
                    filled += 1;
                }
            }
        }
     //   if (!FFMPEG.CacheVideoImages) {
            //clean not used data for non cached work
        
        for (auto& y : (*srvM)) {
            std::vector<int> ToErase;
            for (auto& x : y.second) {
                if (InUseData[y.first].count(x.first) == 0) {
                    ID3D11Resource* r = GetResourceOfUAVSRV(x.second);
                    SafeRelease(r);
                    SafeRelease(x.second);
                    ToErase.push_back(x.first);
                }
            }
            for (auto& x : ToErase) {
                y.second.erase(x); //if I erase in iterator it crashes due to refrencing a non existant object in a logical loop, so I cache all "to delete" object
            }
        }

    //    }
        /*
        *SplitPicCount/CurrentFrameRate = seconds
        
        *SplitPicCount/TargetFrameRate = 

        120 = total frames
        90/30 = frame rate, goal is to make 90 -> 30 same time

        120/30 = 4 //oldseconds good

        120/90 = 1.333 //target seconds wrong

        4/1.333 * current frames = new frame count

        360/90 = 4 seconds
        
        
        
        */
        int CapVal = std::floor((float(i) + 1) * fRat);
        bool HasSetVar = false;
        for (int x = std::floor(float(i) * fRat); x < CapVal; x++) { //new frames

            UpdateFrameConstantBuf((CapVal + 1 - x) / fRat - 1); // 0.0-1.0f is [1]-> [0]    1.01f-2.0f [0]->[2]
            //pass 1
            MakeEmptyUAVfromSRVIntoMap(&PixelFMap[0], (*srvM)[0][i], x);

            if (FFMPEG.CompileLazyIntrop){
                RunImageComputePass(PixelFMap[0][x], &srv, HasSetVar);
            }
            else if (FFMPEG.CompileSoftBodyIntrop) {
                RunSoftBodyComputePass(PixelFMap[0][x], &srv, HasSetVar);
            }
            else {
                RunImageComputePass(PixelFMap[0][x], &srv, HasSetVar);
            }
            HasSetVar = true;
            //


            StopStallAndCheckPic(x, PixelFMap[0][x]);
        }
        for (int iter = 0; iter < PixelFMap.size(); iter++) {
            std::string FPath = FFMPEG.filePathNameStore + FFMPEG.EndProduct;

            if (i == 1)CleanDir(&FPath);

            SaveUAVTexToFile(&PixelFMap[iter], &FPath);
            
            CleanCacheResourceMap(&PixelFMap[iter]);
        }


        i += 1;

    }

    void RunFinalImageCreationPass(std::vector<int>* validFolder, std::string* fsS, int& SplitPicCount, int& RealFrameCount) {
        std::string FinalImageSplitF = FFMPEG.filePathNameStore + FFMPEG.EndProduct;
        CleanDir(&FinalImageSplitF);

        int OldSampleSize = SampleSize;
        SampleSize = 1; // 1 beofre and 1 infront, and current in middle - 2 if start or end, so start and end is messy looks

        int i = 1;
        int LastTmp = 0;
        int tmp = i;
         //pack all data into this map
        std::map<int, std::map<int, ID3D11ShaderResourceView*>> srvM;

        std::map<int, std::string> fsSU;
        fsSU[0] = *fsS;

        for (int& x : *validFolder) {
            fsSU[x + 1] = FFMPEG.filePathNameStore + std::to_string(x) + "\\";
        }

        while (tmp < SampleSize * 2 + 2) {
            for (int& x : *validFolder) {
                AddMainResourceToMapFromFile(&fsSU[x + 1], tmp, &srvM[x + 1]);
            }
            AddMainResourceToMapFromFile(&fsSU[0], tmp, &srvM[0]); //for frame 1
            tmp++;
        }

        CreateConstantBuf();
        MakeShadersAndConstantsData(srvM[0][1]);

        PixelFMap.resize(1);

        float FrameRatio = RealFrameCount / SplitPicCount;

        RunFinalImageCreationPassFrame(i, &srvM, SplitPicCount, RealFrameCount, FrameRatio);

        i += 1;
        bool h = false;
        while (i != tmp) {
            if (i > SampleSize + 1) {
                for (auto x : srvM) {
                    h = AddMainResourceToMapFromFile(&fsSU[x.first], tmp, &srvM[x.first]);
                }
            }
            if (h) {
                tmp += 1;
            }
            h = false;

            RunFinalImageCreationPassFrame(i, &srvM, SplitPicCount, RealFrameCount, FrameRatio);

            i += 1;
        }


        for (auto& x : srvM) {
            CleanCacheResourceMap(&x.second);
        }

        srvM.clear();
        
        CleanCacheResourceMap(&PerFrameRMap);

        //BuildVideo
        std::string MovieOutDirL = FFMPEG.filePathNameStore + FFMPEG.MovieOutDir;
        CleanDir(&MovieOutDirL);

        if (!FFMPEG.DontBuildVideo) {
            std::string qs = R"(")";
            std::string BuildVideo = qs + qs + FFMPEG.filePathNameffmpeg + qs + " -framerate " + std::to_string(TargetFrameRate) + " -i " + qs + FFMPEG.filePathNameStore + FFMPEG.EndProduct + "\%" + std::to_string(ZERO_FILE_COUNT) + "d.png" + qs + " -i " + qs + FFMPEG.filePathNameStore + FFMPEG.musicSplit + FFMPEG.musicName + qs + " -c:a copy -shortest -c:v libx264 -pix_fmt yuv420p " + qs + MovieOutDirL + FFMPEG.MovieOutName + qs+qs;

            system(BuildVideo.c_str());
        }
        //

        SampleSize = OldSampleSize;
        //TODO: use music and new folder with data to construct video with FFMPEG;
    }

    bool VideoCreationPass() {
        std::string fsS = FFMPEG.filePathNameStore + FFMPEG.imageSplit;
        std::string fsM = FFMPEG.filePathNameStore + FFMPEG.musicSplit;

        bool IsMusic = CheckForMusic(&fsM);

        std::vector<int> validFolder;
        validFolder.resize(FFMPEG.ComputePassCount);
        std::fill(validFolder.begin(), validFolder.end(), -1);

        int SplitPicCount = GetSplitPicCount(&fsS);

        for (int i = 0; i < validFolder.size(); i++) {
            std::string FPath = FFMPEG.filePathNameStore + std::to_string(i) + "\\";
            if (GetSplitPicCount(&FPath) == SplitPicCount) validFolder[i] = i;
        }
        for (int i = 0; i < validFolder.size(); i++) {
            if (validFolder[i] == -1) {
                validFolder.erase(validFolder.begin()+i);
                i--;
            }
        }

        validFolder.clear();

        if (validFolder.size() != FFMPEG.ComputePassCount && !FFMPEG.CompileLazyIntrop) return false; //end early due to issue with files

        int RealFrameCount = std::floor((float(SplitPicCount) / float(CurrentFrameRate)) / (float(SplitPicCount) / float(TargetFrameRate))) * SplitPicCount;

        RunFinalImageCreationPass(&validFolder, &fsS, SplitPicCount, RealFrameCount);


        //BUILD MOVIE NOW

        return true;
    }

    void CheckIfINeedToSplitAgain() {
        std::string FPath = FFMPEG.filePathNameStore + FFMPEG.imageSplit;
        if (fs::is_directory(FFMPEG.filePathNameStore) && !std::filesystem::is_empty(FPath)) {

        }
        else {
            FFMPEG.SplitVideo(&FFMPEG.filePathNameStore, &FFMPEG.filePathName, &FFMPEG.filePathNameffmpeg);
        }
    }

    void ComputePassLogicFrame(int i) {

        
        int total = SampleSize * 2 + 1;
        int cur = i;
        int bottom = i - SampleSize;
        int top = i + SampleSize;
        int filled = 1;

        std::vector<ID3D11ShaderResourceView*> srv(total); //the 10 srvs - 0 is curr frame
        srv[0] = PerFrameRMap[cur];
        if (bottom < 1) {
            int SetBottom = cur;
            for (int y = SampleSize; y>-SampleSize; y--) {
                if (SetBottom - y > 0) {
                    bottom = cur - y;
                    top = cur + (SampleSize - y) + SampleSize;
                    y = -SampleSize;
                }
            }
        }
        else if (PerFrameRMap.count(top)==0) { // unbound data, so I need to map count - unless I pre pass max size - won't run for pre cache, so only checks in CPU cache around 7-10 items, so this is very fast despite looking to have a poor worst case' worst case will never be reached
            int SetTop = cur;
            for (int y = SampleSize; y > -SampleSize; y--) {
                if (PerFrameRMap.count(SetTop + y)>0) {
                    top = cur + y;
                    bottom = cur - (SampleSize - y) - SampleSize;
                    y = -SampleSize;
                }
            }
        }


        std::map<int, ID3D11ShaderResourceView*> InUseData; //remove not needed data
      
        if(!FFMPEG.CacheVideoImages)
            InUseData[cur] = PerFrameRMap[cur];

        for (int x = bottom; x < top+1; x++) {
            if (x != cur) {
                
                if (!FFMPEG.CacheVideoImages)
                    InUseData[x] = PerFrameRMap[x];

                srv[filled] = PerFrameRMap[x];
                filled += 1;
            }
        }

        if (!FFMPEG.CacheVideoImages) {
            //clean not used data for non cached work
            std::vector<int> ToErase;
            for (auto& x : PerFrameRMap) {
                if (InUseData.count(x.first) == 0) {
                    ID3D11Resource* r = GetResourceOfUAVSRV(x.second);
                    SafeRelease(r);
                    SafeRelease(x.second);
                    ToErase.push_back(x.first);
                }
            }
            for (auto& x : ToErase) {
                PerFrameRMap.erase(x); //if I erase in iterator it crashes due to refrencing a non existant object in a logical loop, so I cache all "to delete" object
            }

        }



        if (FFMPEG.ComputePixelChangeFrequency) {
            MakeEmptyUAVfromSRVIntoMap(&PixelFMap[0], PerFrameRMap[i], i);

            RunPixelFrequency(PixelFMap[0][i], &srv);
        }
        //
        if (FFMPEG.ComputePixelChangeFrequency) {
            StopStallAndCheckPic(i, PixelFMap[FFMPEG.NumToShow][i]);
        }
        else{
            StopStallAndCheckPic(i, PerFrameRMap[i]);
        }

        for (int iter = 0; iter < PixelFMap.size(); iter++) {
            if (FFMPEG.SaveTex) {
                std::string FPath = FFMPEG.filePathNameStore + std::to_string(iter) + "\\";
                
                if(i == 1)CleanDir(&FPath);
                
                SaveUAVTexToFile(&PixelFMap[iter], &FPath);
            }
            CleanCacheResourceMap(&PixelFMap[iter]);
        }


        i += 1;
    }

    void CleanCacheResourceMap(std::map<int, ID3D11ShaderResourceView*>* m) {
        for (auto& i : (*m) ) {
            ID3D11Resource* TexO = nullptr;
            i.second->GetResource(&TexO);
            SafeRelease(i.second);
            SafeRelease(TexO);
        }
        m->clear();
    }
    void CleanCacheResourceMap(std::map<int, ID3D11UnorderedAccessView*>* m) {
        for (auto& i : (*m)) {
            ID3D11Resource* TexO = nullptr;
            i.second->GetResource(&TexO);
            SafeRelease(i.second);
            SafeRelease(TexO);
        }
        m->clear();
    }

    void StartComputePass(){
        CheckIfINeedToSplitAgain();

        std::string psp = FFMPEG.filePathNameStore + FFMPEG.imageSplit;

        if (FFMPEG.CacheVideoImages) {
            int val = 1;
            while (AddMainResourceToMapFromFile(&psp, val, &PerFrameRMap)) {
                val += 1;
            }
        }
        else {
            int val = 1;
            AddMainResourceToMapFromFile(&psp, val, &PerFrameRMap);
        }
        
        CreateConstantBuf();
        MakeShadersAndConstantsData(PerFrameRMap[1]);

        PixelFMap.resize(FFMPEG.ComputePassCount);

        int i = 1;
        if (FFMPEG.CacheVideoImages) {
            while (i < PerFrameRMap.size() + 1) { //first few start and end frames are not very accurate due to more forward data-
                //t0 is promised to be main frame, t1-4 is promised to be less than (normally) main frame, and t5-9 is more than main frame
                ComputePassLogicFrame(i);
                i += 1;
            }
        }
        else {
            int LastTmp = 0;
            int tmp = i;
            while (tmp < SampleSize * 2 + 2) {
                AddMainResourceToMapFromFile(&psp, tmp, &PerFrameRMap); //for frame 1
                tmp++;
            }

            ComputePassLogicFrame(i);
            i += 1;
            bool h = false;
            while (i!=tmp) {
                if (i > SampleSize + 1) {
                    h = AddMainResourceToMapFromFile(&psp, tmp, &PerFrameRMap);
                }
                if (h) {
                    tmp += 1;
                }
                h = false;

                ComputePassLogicFrame(i);
                
                i += 1;
            }
        }
        
        for (int y = 0; y < PixelFMap.size();y++) {
            CleanCacheResourceMap(&PixelFMap[y]);
        }
        CleanCacheResourceMap(&PerFrameRMap);
        SafeRelease(Constants);
        Constants = nullptr;

        ResizeWindowAndViewport(1000, 1000, DXGI_FORMAT_R8G8B8A8_UNORM);
    }
    

    void MakeEmptyUAVfromSRVIntoMap(std::map<int, ID3D11UnorderedAccessView*>* m, ID3D11ShaderResourceView* srv, int frame) {
        if (m->count(frame) > 0) {
            ID3D11Resource* TexO = nullptr;
            (*m)[frame]->GetResource(&TexO);
            SafeRelease(TexO);
        }
        (*m)[frame] = MakeEmptyUAVfromSRV(srv);

        FLOAT fl[4] = { 0.0f,0.0f,0.0f,1.0f };

        dxDeviceContext->ClearUnorderedAccessViewFloat((*m)[frame], fl);
    }

    ID3D11UnorderedAccessView* MakeEmptyUAVfromSRV(ID3D11ShaderResourceView* srv) {
        D3D11_TEXTURE2D_DESC d;
        ComPtr<ID3D11Resource> r;
        srv->GetResource(&r);
        ComPtr<ID3D11Texture2D> tex;
        r.As(&tex);

        tex->GetDesc(&d);
        
        ID3D11Texture2D* texUAV;

        d.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

        dxDevice->CreateTexture2D(&d, NULL, &texUAV);

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavs;
        uavs.Format = d.Format;
        uavs.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uavs.Texture2D.MipSlice = 0;

        ID3D11UnorderedAccessView* uav;

        dxDevice->CreateUnorderedAccessView(texUAV,&uavs,&uav);

        SafeRelease(texUAV);

        return uav;
    }

    void GetTextureFromPathIntoMap(std::string* path, int frame, std::map<int, ID3D11ShaderResourceView*>* m) {
        std::pair<ID3D11ShaderResourceView*, HRESULT> srv = GetTextureFromPath(path);
        /*if (PerFrameRMap.count(frame) > 0) {
            ID3D11Resource* TexO = nullptr;
            PerFrameRMap[frame]->GetResource(&TexO);
            SafeRelease(TexO);
        } //should never hit this event, so I commented out
        */

        if (srv.second == S_OK) {
            (*m)[frame] = srv.first;
        }
        else {

        }
    }

    std::pair<ID3D11ShaderResourceView*,HRESULT> GetTextureFromPath(std::string* path) {
        ID3D11Resource* tex = nullptr;
        ID3D11ShaderResourceView* srv = nullptr;

        CA2W ca2w(path->c_str());
        LPWSTR ws = LPWSTR(ca2w);
        
        HRESULT d = CreateWICTextureFromFile(dxDevice.Get(), dxDeviceContext.Get(),
            ws, &tex, &srv, 0);//TODO: delete all mip's
        
        SafeRelease(tex);

        return { srv, d };

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