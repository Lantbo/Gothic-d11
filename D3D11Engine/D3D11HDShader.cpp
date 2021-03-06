#include "pch.h"
#include "D3D11HDShader.h"
#include "D3D11GraphicsEngineBase.h"
#include "Engine.h"
#include "GothicAPI.h"
#include "D3D11ConstantBuffer.h"
#include <d3dcompiler.h>
#include "D3D11_Helpers.h"

using namespace DirectX;

// Patch HLSL-Compiler for http://support.microsoft.com/kb/2448404
#if D3DX_VERSION == 0xa2b
#pragma ruledisable 0x0802405f
#endif

D3D11HDShader::D3D11HDShader() {
    ConstantBuffers = std::vector<D3D11ConstantBuffer*>();
}


D3D11HDShader::~D3D11HDShader() {
    for ( unsigned int i = 0; i < ConstantBuffers.size(); i++ ) {
        delete ConstantBuffers[i];
    }
}

//--------------------------------------------------------------------------------------
// Find and compile the specified shader
//--------------------------------------------------------------------------------------
HRESULT D3D11HDShader::CompileShaderFromFile( const CHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut ) {
    HRESULT hr = S_OK;

    char dir[260];
    GetCurrentDirectoryA( 260, dir );
    SetCurrentDirectoryA( Engine::GAPI->GetStartDirectory().c_str() );

    DWORD dwShaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    //dwShaderFlags |= D3DCOMPILE_DEBUG;
#else
    dwShaderFlags |= D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;
    hr = D3DCompileFromFile( Toolbox::ToWideChar( szFileName ).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, szEntryPoint, szShaderModel, dwShaderFlags, 0, ppBlobOut, pErrorBlob.GetAddressOf() );
    if ( FAILED( hr ) ) {
        LogInfo() << "Shader compilation failed!";
        if ( pErrorBlob.Get() ) {
            LogErrorBox() << (char*)pErrorBlob->GetBufferPointer() << "\n\n (You can ignore the next error from Gothic about too small video memory!)";
        }

        SetCurrentDirectoryA( dir );
        return hr;
    }

    SetCurrentDirectoryA( dir );
    return S_OK;
}

/** Loads both shaders at the same time */
XRESULT D3D11HDShader::LoadShader( const char* hullShader, const char* domainShader ) {
    HRESULT hr;
    D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase*)Engine::GraphicsEngine;

    Microsoft::WRL::ComPtr<ID3DBlob> hsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> dsBlob;

    if ( Engine::GAPI->GetRendererState().RendererSettings.EnableDebugLog )
        LogInfo() << "Compilling hull shader: " << hullShader;

    // Compile shaders
    if ( FAILED( CompileShaderFromFile( hullShader, "HSMain", "hs_5_0", hsBlob.GetAddressOf() ) ) ) {
        return XR_FAILED;
    }

    if ( FAILED( CompileShaderFromFile( domainShader, "DSMain", "ds_5_0", dsBlob.GetAddressOf() ) ) ) {
        return XR_FAILED;
    }

    // Create the shaders
    LE( engine->GetDevice()->CreateHullShader( hsBlob->GetBufferPointer(), hsBlob->GetBufferSize(), nullptr, HullShader.GetAddressOf() ) );
    LE( engine->GetDevice()->CreateDomainShader( dsBlob->GetBufferPointer(), dsBlob->GetBufferSize(), nullptr, DomainShader.GetAddressOf() ) );

    SetDebugName( HullShader.Get(), hullShader );
    SetDebugName( DomainShader.Get(), domainShader );

    return XR_SUCCESS;
}

/** Applys the shaders */
XRESULT D3D11HDShader::Apply() {
    D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase*)Engine::GraphicsEngine;

    engine->GetContext()->HSSetShader( HullShader.Get(), nullptr, 0 );
    engine->GetContext()->DSSetShader( DomainShader.Get(), nullptr, 0 );

    return XR_SUCCESS;
}

/** Unbinds the currently bound hull/domain shaders */
void D3D11HDShader::Unbind() {
    D3D11GraphicsEngineBase* engine = (D3D11GraphicsEngineBase*)Engine::GraphicsEngine;

    engine->GetContext()->HSSetShader( nullptr, nullptr, 0 );
    engine->GetContext()->DSSetShader( nullptr, nullptr, 0 );
}

/** Returns a reference to the constantBuffer vector*/
std::vector<D3D11ConstantBuffer*>& D3D11HDShader::GetConstantBuffer() {
    return ConstantBuffers;
}
