#pragma once
//finally I will try to use com_ptr's

#define UNNegative 10000 //assume 10000 pixel video max

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

using namespace DirectX;

void ThrowFailed(HRESULT v) {
    if (v != S_OK) {
        throw(v);
    }
}
template<typename T>
inline void SafeReleaseAll(T& ptr)
{
    if (ptr !=  nullptr)
    {
        
#ifdef _DEBUG
        long v = ptr->Release();
        if (v != 0)
        {
            std::cout << "uh oh, refrence of: " + std::to_string(v) + "\n";
            while (ptr->Release());
        }


#else
        while(ptr->Release());
#endif
        ptr = nullptr;
    }
}
template<typename T>
inline void SafeRelease(T& ptr)
{
    if (ptr != nullptr)
    {

#ifdef _DEBUG
        long v = ptr->Release();
        if (v != 0)
        {
   //         std::cout << "uh oh, refrence of: " + std::to_string(v) + "\n";
        }


#else
        ptr->Release();
#endif
        ptr = nullptr;
    }
}



ID3D11InputLayout* dxInputLayoutQuad;

template< class ShaderClass >  //done here so you can do GetLatestProfile<ID3D11PixelShader>(); or related to make your own shaders and such - I implemented the hull shaders and such here so cohesiveness is kept to a maximum
std::string GetLatestProfile(ID3D11Device* dxDevice); //template to get shader level to compile with

template<>
std::string GetLatestProfile<ID3D11VertexShader>(ID3D11Device* dxDevice)
{
    // Query the current feature level:
    D3D_FEATURE_LEVEL featureLevel = dxDevice->GetFeatureLevel();

    switch (featureLevel) // later if needed I will add a dx12 feature level... 
    {
    case D3D_FEATURE_LEVEL_11_1:
    case D3D_FEATURE_LEVEL_11_0:
    {
        return "vs_5_0";
    }
    break;
    case D3D_FEATURE_LEVEL_10_1:
    {
        return "vs_4_1";
    }
    break;
    case D3D_FEATURE_LEVEL_10_0:
    {
        return "vs_4_0";
    }
    break;
    case D3D_FEATURE_LEVEL_9_3:
    {
        return "vs_4_0_level_9_3";
    }
    break;
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
    {
        return "vs_4_0_level_9_1";
    }
    break;
    }
    return "";
}
////////////////////
template<>
std::string GetLatestProfile<ID3D11PixelShader>(ID3D11Device* dxDevice)
{
    // Query the current feature level:
    D3D_FEATURE_LEVEL featureLevel = dxDevice->GetFeatureLevel(); //feature level to compile pixel shader 
    switch (featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_1:
    case D3D_FEATURE_LEVEL_11_0:
    {
        return "ps_5_0";
    }
    break;
    case D3D_FEATURE_LEVEL_10_1:
    {
        return "ps_4_1";
    }
    break;
    case D3D_FEATURE_LEVEL_10_0:
    {
        return "ps_4_0";
    }
    break;
    case D3D_FEATURE_LEVEL_9_3:
    {
        return "ps_4_0_level_9_3";
    }
    break;
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
    {
        return "ps_4_0_level_9_1";
    }
    break;
    }
    return "";
}

//////////////

template<> // I setup these despite not using because a good chance exists that to expand PGE this code will be more than helpful - plus these are kinda place holders for loading shaders (need acsess to device in dx11 - plus you won't do it any other way...
std::string GetLatestProfile<ID3D11ComputeShader>(ID3D11Device* dxDevice)
{
    // Query the current feature level:
    D3D_FEATURE_LEVEL featureLevel = dxDevice->GetFeatureLevel(); //feature level to compile pixel shader 
    switch (featureLevel)
    {
    case D3D_FEATURE_LEVEL_11_1:
    case D3D_FEATURE_LEVEL_11_0:
    {
        return "cs_5_0";
    }
    break;
    case D3D_FEATURE_LEVEL_10_1:
    {
        return "cs_4_1";
    }
    break;
    case D3D_FEATURE_LEVEL_10_0:
    {
        return "cs_4_0";
    }
    break;
    case D3D_FEATURE_LEVEL_9_3:
    {
        return "cs_4_0_level_9_3";
    }
    break;
    case D3D_FEATURE_LEVEL_9_2:
    case D3D_FEATURE_LEVEL_9_1:
    {
        return "cs_4_0_level_9_1";
    }
    break;
    }
    return "";
}


template< class ShaderClass >
ShaderClass* CreateShader(ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage, ID3D11Device* dxDevice); //generic shader creation

template<>
ID3D11VertexShader* CreateShader<ID3D11VertexShader>(ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage, ID3D11Device* dxDevice) //vertex shader - shader type
{
    ID3D11VertexShader* pVertexShader = nullptr;
    dxDevice->CreateVertexShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pVertexShader); //make a shader based on buffer, buffer size, classtype, and return to pshader object

    D3D11_INPUT_ELEMENT_DESC dxVertexLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },

    };

    SafeRelease(dxInputLayoutQuad); //stop mem leak if overwrite previously used layout

    auto hr = dxDevice->CreateInputLayout( //make input layout - global change to input Layout
        dxVertexLayoutDesc, //vertex shader - input assembler data
        _countof(dxVertexLayoutDesc), //number of elements
        pShaderBlob->GetBufferPointer(),  //vertex shader buffer
        pShaderBlob->GetBufferSize(), //vetex shader blob size 
        &dxInputLayoutQuad); //input layout to output to

    if (FAILED(hr))
    {
        OutputDebugStringW(L"failed input layout setup");
    }
    return pVertexShader;
}
////////////////////////////////
template<>
ID3D11PixelShader* CreateShader<ID3D11PixelShader>(ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage, ID3D11Device* dxDevice)
{
    ID3D11PixelShader* pPixelShader = nullptr;
    dxDevice->CreatePixelShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pPixelShader); //pixel shader version of the vertex shader above

    return pPixelShader;
}

//////////////////////////////// these are kinda place holders for loading shaders (need acsess to device in dx11 - plus you won't do it any other way...
template<>
ID3D11ComputeShader* CreateShader<ID3D11ComputeShader>(ID3DBlob* pShaderBlob, ID3D11ClassLinkage* pClassLinkage, ID3D11Device* dxDevice)
{
    ID3D11ComputeShader* pComputeShader = nullptr;
    dxDevice->CreateComputeShader(pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), pClassLinkage, &pComputeShader); //pixel shader version of the vertex shader above

    return pComputeShader;
}



template< class ShaderClass >
ShaderClass* LoadShader(const std::string* shaderInfo, const std::string& entryPoint, const std::string& _profile, ID3D11Device* dxDevice) //LoadShader class
{
    ID3DBlob* pShaderBlob = nullptr;
    ID3DBlob* pErrorBlob = nullptr;
    ShaderClass* pShader = nullptr;

    std::string profile = _profile;
    if (profile == "latest")
    {
        profile = GetLatestProfile<ShaderClass>(dxDevice); //get shader profiles/settings
    }

    UINT flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;

#if _DEBUG
    flags |= D3DCOMPILE_DEBUG;
#endif
    HRESULT hr = D3DCompile2(shaderInfo->c_str(), shaderInfo->length(), nullptr,
        nullptr, nullptr, entryPoint.c_str(),
        profile.c_str(), flags, 0, 0, 0, 0, &pShaderBlob, &pErrorBlob);
    OutputDebugStringA("\n");
    if (pErrorBlob != nullptr) {
        OutputDebugStringA((const char*)pErrorBlob->GetBufferPointer());
    }

    pShader = CreateShader<ShaderClass>(pShaderBlob, nullptr, dxDevice);

    SafeRelease(pShaderBlob); // no longer need shader mem
    SafeRelease(pErrorBlob); // no longer need shader mem <-- I frogot to safe release to delete and do other stuff... so I need to look back at that

    return pShader;
}

struct ShaderString {

    std::string CreateFinalComputeLogic1Shader(int BLOCK_SIZE, int SampleSize) {
        return (
            "#define BLOCK_SIZE " + std::to_string(BLOCK_SIZE) + "\n"

            "cbuffer ConstDataFrame : register(b0){\n"
            "float FrameRatio;\n"
            "}\n"

            "RWTexture2D<unorm float4> OutT : register(u0);\n"
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
            );
    }


    std::string FinalComputeLogicSoftBody1(int BLOCK_SIZE, int SampleSize) { //TODO: fix and make interpolating
        return (
            "#define UNNegative " + std::to_string(UNNegative) + "\n"
            "#define BLOCK_SIZE " + std::to_string(BLOCK_SIZE) + "\n"

            "cbuffer ConstDataFrame : register(b0){\n"
            "float FrameRatio;\n"
            "}\n"

            "RWTexture2D<unorm float4> OutT : register(u0);\n"
            "#define SampleSize " + std::to_string(SampleSize * 2 + 1) + "\n"
            "Texture2D tex[SampleSize] : register(t0); \n"//compare texture is 0, rest is extra 
            "Texture2D texFr[SampleSize] : register(t" + std::to_string((SampleSize * 2 + 1)) + ");\n" //frequency
            "Texture2D texRc[SampleSize] : register(t" + std::to_string((SampleSize * 2 + 1) * 2) + ");\n" //rate of change
            "Texture2D texDc[SampleSize] : register(t" + std::to_string((SampleSize * 2 + 1) * 3) + ");\n" //rate of change

            "struct ComputeShaderInput\n"
            "{\n"
            "uint3 groupID : SV_GroupID;           // 3D index of the thread group in the dispatch.\n"
            "uint3 groupThreadID : SV_GroupThreadID;     // 3D index of local thread ID in a thread group.\n"
            "uint3 dispatchThreadID : SV_DispatchThreadID;  // 3D index of global thread ID in the dispatch.\n"
            "uint  groupIndex : SV_GroupIndex;        // Flattened local index of the thread within a thread group.\n"
            "};\n"

            "[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]\n"
            "void CS_main(ComputeShaderInput IN){\n"
            "int2 GTID = IN.groupThreadID.xy;"
            "int2 texC = IN.dispatchThreadID.xy;\n"
            "float4 distC = texDc[0].Load(int3(texC,0));\n"
            "float Fr = FrameRatio;\n"
            "int2 tpos = texC+int2(int2(distC.x-UNNegative-BLOCK_SIZE+GTID.x,distC.y-UNNegative-BLOCK_SIZE+GTID.x)*Fr);"
            "\n"
            "\n"
            "float4 d1 = tex[0].Load(int3(texC, 0));\n"
            "float4 d2 = tex[2].Load(int3(tpos, 0));\n" //average data --distance to--> where I think data goes to

            "float4 final = d1*Fr+d2*(1-Fr);\n"
            "if (Fr!=1.0f)"
            "final = d2;\n"
           // "final.x = (d1.x*(d1e*Fr));\n"
           // "final.y = (d1.y*(d1e*Fr));\n"
           // "final.z = (d1.z*(d1e*Fr));\n"
            //"OutT[texC] = final;\n"
            //
            "OutT[texC] = final;\n"// TODO: use distance for value fixing to point on image
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "\n"
            "}\n"
            );
    }

    std::string CreatePixelFrequencyCalcShader(int BLOCK_SIZE, int SampleSize) {
        /*
        has the goal to flag pixels that change enough within a range of softbody change values to not make weird interpolation in the middle of nowhere
        
        */
        return (
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
            "RWTexture2D<unorm float4> OutT : register(u0);\n"
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
            "int2 texC = IN.dispatchThreadID.xy;\n"
            //"OutT[IN.dispatchThreadID.xy] = tex[0].Load(int3(texCoord,0));"
            "float4 max = 0.0f;\n"
            "float4 cur = 0.0f;\n"
            "\n"
            "[unroll]\n"
            "for(int i = 1; i < SampleSize; i++){\n"
            "max+=float4(1.0f,1.0f,1.0f,1.0f);\n"
            "cur+=tex[i].Load(int3(texC,0));\n"
            "\n"
            "}\n"
            "float4 mmc = max-cur;\n"
            "float4 tlM = tex[0].Load(int3(texC,0));\n"
            "float x = 0.0f;\n"
            "float y = 0.0f;\n"
            "float z = 0.0f;\n"
            "float Check = 0.0f;"
            "float3 Comp = float3(abs(mmc.x-tlM.x),abs(mmc.y-tlM.y),abs(mmc.z-tlM.z));\n" //TODO: fix and make not dumb slow junk THIS CODE IS A SIN UPON GPU SHADER CODE
            "if(Comp.x<0.2f) x=(Comp.x/0.2f); Check = 1.0f;\n"
            "if(Comp.y<0.2f) y=(Comp.y/0.2f); Check = 1.0f;\n"
            "if(Comp.z<0.2f) z=(Comp.z/0.2f); Check = 1.0f;\n"
            "OutT[texC] = float4(x,y,z,Check);\n"
            "\n"
            "\n"
            "}\n"
            );
        //TODO, use distance and rate of change to interpolate in between for new frames
    }
    std::string CreateRateOfChangeAndDistShader(int BLOCK_SIZE, int SampleSize) { //TODO: test if function calls are slower - I'm really - REALLY worried about unneeded copies - since it would do ***hundreds*** which could hurt render time by miles - I highly doubt the compiler could auto optimise this... since its runtime dependent too... but I should check and keep this in mind - keeps code cleaner
        return ( //TODO: add goal from email - for now just remember it gets layer of imaging
            "#define UNNegative " + std::to_string(UNNegative) + "\n"
            "#define BLOCK_SIZE " + std::to_string(BLOCK_SIZE) + "\n"
            //The idea of this compute is to get rate of change for intrpolation of pixels mid frame so I know how to interpolate colors
            "cbuffer ConstData : register(b0){\n"
            "int DepthX;\n"
            "int DepthY;\n"
            "uint pad4;\n"
            "uint pad1;\n"
            "uint3 numTG;\n"
            "uint pad2;\n"
            "uint3 TG;\n"
            "uint pad3;\n"
            "}\n"
            
            //range for +- avg
            "#define MinR 0.99f\n"
            "#define MaxR 1.01f\n"

            "RWTexture2D<unorm float4> OutT : register(u0);\n"
            "RWTexture2D<float4> OutRange : register(u1); \n"//compare texture is 0, rest is extra 

            "#define SampleSize " + std::to_string(SampleSize * 2 + 1) + "\n"
            "#define SampleNext " + std::to_string(SampleSize + 1) + "\n"
            "Texture2D tex[SampleSize] : register(t0); \n"//compare texture is 0, rest is extra 

            "struct ComputeShaderInput\n"
            "{\n"
            "uint3 groupID : SV_GroupID;           // 3D index of the thread group in the dispatch.\n"
            "uint3 dispatchThreadID : SV_DispatchThreadID;  // 3D index of global thread ID in the dispatch.\n"
            "uint  groupIndex : SV_GroupIndex;        // Flattened local index of the thread within a thread group.\n"
            "uint3 groupThreadID : SV_GroupThreadID;     // 3D index of local thread ID in a thread group.\n"
            "};\n"

            "groupshared uint AveragePostX = 0;\n"
            "groupshared uint AveragePostY = 0;\n"
            "groupshared uint AveragePostZ = 0;\n"
            "groupshared float3 AveragePost = float3(0.0f,0.0f,0.0f);\n"
            
            "[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]\n"
            "void CS_main(ComputeShaderInput IN){\n"
            "int2 texC = IN.dispatchThreadID.xy;\n"
            "float4 tM = tex[0].Load(int3(texC,0));\n"
            "float4 tMn = tex[SampleNext].Load(int3(texC,0));\n"
            //value cannot be bigger than 1000f, soooo: i'ma just multiply by 1000 --- 1/255 is max, so this is safe

            "OutT[texC] = float4(tMn.x/tM.x,tMn.y/tM.y,tMn.z/tM.z,1.0f);\n"
            
            "InterlockedAdd(AveragePostX,int(tMn.x*1000.0f));\n" //get average of tile
            "InterlockedAdd(AveragePostY,int(tMn.y*1000.0f));\n"
            "InterlockedAdd(AveragePostZ,int(tMn.z*1000.0f));\n"

            "GroupMemoryBarrierWithGroupSync();\n" // ensure average is calc'ed
            "if(IN.groupIndex == 0){"
            "AveragePost = float3(float(AveragePostX)/1000.0f,float(AveragePostY)/1000.0f,float(AveragePostZ)/1000.0f);"
            "AveragePost /= BLOCK_SIZE*BLOCK_SIZE;\n"
            "}"
            "GroupMemoryBarrierWithGroupSync();\n" // ensure average is calc'ed

            "OutRange[texC] = float4(AveragePost,1.0f);\n"
/*       //     "[unroll]"
            "for(int x = 0; x < BLOCK_SIZE; x++){\n" //highest chance no change, so check for no change in tile group, else you gonna be - goin far
         //   "[unroll]"
            "for(int y = 0; y < BLOCK_SIZE; y++){"
            "if(all(maxAve>tex[0].Load(int3(int2(start.x+x,start.y+y),0)).xyz>minAve)) {\n"

            "x=BLOCK_SIZE; y = BLOCK_SIZE; OutRange[texC] = float4(texC.x-start.x+x,texC.y-start.y+y,0.0f,0.0f);\n"

            "}\n"
            "}\n"
            "}\n"            
            */

            "}\n" //TODO: make it find shortest dist option - slower but more accurate interp
            //The goal of this shader is to fill my distance UAV which I stored in an average value by looking at every single block in a dispatch overall in a loop, for average colors in the tile being within range. if it is  within range I know the colors prob- came from that point meaning I can interpolate from those pixels. smaller range == more accuracy, but I'm trying 0.1f flat for now
            
            //Finds post position, so loads and checks pix pos
            "int Logic(ComputeShaderInput IN){"

            "int2 texC = IN.dispatchThreadID.xy;\n"
            "float3 AverageP = OutRange[texC];"
            "float3 minAve = AverageP*MinR;\n"
            "float3 maxAve = AverageP*MaxR;\n"
            //    "uint2 groupIndex = IN.groupIndex;\n" //so I don't conflict in T group when I read averages from UAV with each read
            "uint2 groupID = IN.groupID.xy;\n"
            "uint2 start = uint2(groupID.x*BLOCK_SIZE,groupID.y*BLOCK_SIZE);\n"
            "int depthXI = DepthX;"
            "int depthYI = DepthY;"

            "for(int x = start+BLOCK_SIZE; x < depthXI; x+=BLOCK_SIZE){\n" //check greater than start

            "for(int y = start+BLOCK_SIZE; y < depthYI; y+=BLOCK_SIZE){"
            
            "int2 Loc = int2(x+groupID.x,y+groupID.y);"

            "float3 Comp = tex[2].Load(int3(Loc, 0)).xyz;"

            "if((maxAve.x>Comp.x>minAve.x) && (maxAve.y>Comp.y>minAve.y) && (maxAve.z>Comp.z>minAve.z)) {\n"

            "OutRange[texC] = float4(Loc.x-start.x+UNNegative,Loc.y-start.y+UNNegative,x,y); return 0;\n" //1.0f/x pos and y pos done to preseve pos rather than get dist

            "}\n"
            "}\n"

            "for(int y = start-BLOCK_SIZE; y > 0; y-=BLOCK_SIZE){"

            "int2 Loc = int2(x+groupID.x,y+groupID.y);"

                "float3 Comp = tex[2].Load(int3(Loc, 0)).xyz;"

                "if((maxAve.x>Comp.x>minAve.x) && (maxAve.y>Comp.y>minAve.y) && (maxAve.z>Comp.z>minAve.z)) {\n"

            "OutRange[texC] = float4(Loc.x-start.x+UNNegative,Loc.y-start.y+UNNegative,x,y); return 0;\n" //1.0f/x pos and y pos done to preseve pos rather than get dist

            "}\n"
            "}\n"

            "}\n"

            "for(int x = start-BLOCK_SIZE; x > 0; x-=BLOCK_SIZE){\n" //check greater than start

            "for(int y = start+BLOCK_SIZE; y < depthYI; y+=BLOCK_SIZE){"

            "int2 Loc = int2(x+groupID.x,y+groupID.y);"

                "float3 Comp = tex[2].Load(int3(Loc, 0)).xyz;"

            "if((maxAve.x>Comp.x>minAve.x) && (maxAve.y>Comp.y>minAve.y) && (maxAve.z>Comp.z>minAve.z)) {\n"

            "OutRange[texC] = float4(Loc.x-start.x+UNNegative,Loc.y-start.y+UNNegative,x,y); return 0;\n" //1.0f/x pos and y pos done to preseve pos rather than get dist

            "}\n"
            "}\n"

            "for(int y = start-BLOCK_SIZE; y > 0; y-=BLOCK_SIZE){"

            "int2 Loc = int2(x+groupID.x,y+groupID.y);"

                "float3 Comp = tex[2].Load(int3(Loc, 0)).xyz;"

                "if((maxAve.x>Comp.x>minAve.x) && (maxAve.y>Comp.y>minAve.y) && (maxAve.z>Comp.z>minAve.z)) {\n"

            "OutRange[texC] = float4(Loc.x-start.x+UNNegative,Loc.y-start.y+UNNegative,x,y); return 0;\n" //1.0f/x pos and y pos done to preseve pos rather than get dist

            "}\n"
            "}\n"

            "}"

            "OutRange[texC] = float4(0.0f,0.0f,texC.x,texC.y);"

            "return 0;"
            "}"
            
            "[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]\n"
            "void CS_Dist(ComputeShaderInput IN){\n"

            "Logic(IN);" //this one just takes first rather than doing everything always





                ////////////////////////////////////////////////////////////////BELOW IS wait until end slow approach - checks each dir once for closest - prefrence same height





            "\n"
            "\n"
            "\n"
            "}\n"


            "int LogicA(ComputeShaderInput IN){"

            "int2 texC = IN.dispatchThreadID.xy;\n"
            "float3 AverageP = OutRange[texC];"
            "float3 minAve = AverageP*MinR;\n"
            "float3 maxAve = AverageP*MaxR;\n"
            //    "uint2 groupIndex = IN.groupIndex;\n" //so I don't conflict in T group when I read averages from UAV with each read
            "uint2 groupID = IN.groupID.xy;\n"
            "uint2 start = uint2(groupID.x*BLOCK_SIZE,groupID.y*BLOCK_SIZE);\n"
            "int4 Dist = float4(DepthX+DepthX,DepthY+DepthY,0,0);"

            "for(int y = start.y; y < DepthY; y+=BLOCK_SIZE){\n" //check greater than start


            "for(int x = start.x; x < DepthX; x+=BLOCK_SIZE){"

            "int2 Loc = int2(x+groupID.x,y+groupID.y);"
            "int2 distT = float2(Loc.x-start.x,Loc.y-start.y);"

            "if((Dist.x + Dist.y) > (distT.x + distT.y)){"
                "float3 Comp = tex[2].Load(int3(Loc, 0)).xyz;"

                "if((maxAve.x>Comp.x>minAve.x) && (maxAve.y>Comp.y>minAve.y) && (maxAve.z>Comp.z>minAve.z)) {\n" //compare distance to make sure you are greater dist to change to closer

            "Dist = int4(distT,x,y); x = DepthX; //; y = DepthY;\n" //x and y are stored for debug

            "}\n"
            "}\n"
                "else x = DepthX;"
            "}\n"

            "for(int x = start.x; x > -1; x-=BLOCK_SIZE){"

            "int2 Loc = int2(x+groupID.x,y+groupID.y);"
            "float2 distT = float2(Loc.x-start.x,Loc.y-start.y);"

                    "if((Dist.x + Dist.y) > (distT.x + distT.y)){"
                    "float3 Comp = tex[2].Load(int3(Loc, 0)).xyz;"


                    "if((Dist.x+Dist.y)>(distT.x+distT.y) && (maxAve.x>Comp.x>minAve.x) && (maxAve.y>Comp.y>minAve.y) && (maxAve.z>Comp.z>minAve.z)) {\n" //compare distance to make sure you are greater dist to change to closer

                    "Dist = int4(distT,x,y); x = 0; // y = DepthY;\n" //x and y are stored for debug

            "}\n"
            "}\n"
                "else x = 0;"
            "}\n"
            "}\n"

            "for(int y = start.y; y > -1; y-=BLOCK_SIZE){\n" //check greater than start

            "for(int x = start.x; x < DepthX; x+=BLOCK_SIZE){"

                "int2 Loc = int2(x+groupID.x,y+groupID.y);"
                "float2 distT = float2(Loc.x-start.x,Loc.y-start.y);"
                
                "if((Dist.x + Dist.y) > (distT.x + distT.y)){"
                "float3 Comp = tex[2].Load(int3(Loc, 0)).xyz;"

                    "if((Dist.x+Dist.y)>(distT.x+distT.y) && (maxAve.x>Comp.x>minAve.x) && (maxAve.y>Comp.y>minAve.y) && (maxAve.z>Comp.z>minAve.z)) {\n" //compare distance to make sure you are greater dist to change to closer

                    "Dist = int4(distT,x,y); x = DepthX; //y = 0;\n" //x and y are stored for debug
            "}\n"
            "}\n"
            "else x = DepthX;"
            "}\n"
            

            "for(int x = start.x; x > -1; x-=BLOCK_SIZE){"

            "int2 Loc = int2(x+groupID.x,y+groupID.y);"
            "float2 distT = float2(Loc.x-start.x,Loc.y-start.y);"
                "if((Dist.x + Dist.y) > (distT.x + distT.y)){"
                "float3 Comp = tex[2].Load(int3(Loc, 0)).xyz;"

                "if((Dist.x+Dist.y)>(distT.x+distT.y) && (maxAve.x>Comp.x>minAve.x) && (maxAve.y>Comp.y>minAve.y) && (maxAve.z>Comp.z>minAve.z)) {\n" //compare distance to make sure you are greater dist to change to closer

                    "Dist = int4(distT,x,y); x= 0; //y = 0;\n" //x and y are stored for debug

            "}\n"
            "}\n"
            "}\n"

            "}"
            "if(Dist.x == DepthX + DepthX && Dist.y == DepthY + DepthY)" "OutRange[texC] = float4(UNNegative,UNNegative,texC.x,texC.y);" //nothing* color is gone
            "else OutRange[texC] = float4(Dist.x+UNNegative,Dist.y+UNNegative,Dist.z,Dist.w);"

            "return 0;"
            "}"//TODO: optimise this by smart skiping if's - dw about uniform, blocks are the same if I keep block semantics in use




            "[numthreads(BLOCK_SIZE, BLOCK_SIZE, 1)]\n"
            "void CS_DistA(ComputeShaderInput IN){\n"

            "LogicA(IN);"


            "\n"
            "\n"
            "\n"
            "}\n"
                
            ""

            );
    }
}SStringC;