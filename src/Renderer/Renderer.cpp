#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Renderer/Renderer.h"
#include "Renderer/Camera.h"

#include <d3dcompiler.h>
#include <directxcolors.h>
#include <directxmath.h>

using namespace DirectX;

namespace
{
	struct SimpleVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT4 Col;
	};

	struct ConstantBuffer
	{
		XMMATRIX mWorld;
		XMMATRIX mView;
		XMMATRIX mProjection;
	};
	HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
	{
		HRESULT hr = S_OK;

		DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
		// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
		// Setting this flag improves the shader debugging experience, but still allows
		// the shaders to be optimized and to run exactly the way they will run in
		// the release configuration of this program.
		dwShaderFlags |= D3DCOMPILE_DEBUG;

		// Disable optimizations to further improve shader debugging
		dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		Microsoft::WRL::ComPtr<ID3DBlob> pErrorBlob;
		hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
			dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
		if (FAILED(hr))
		{
			if (pErrorBlob)
				OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
			return hr;
		}

		return S_OK;
	}
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT Renderer::InitDevice(HWND hWnd)
{
	static_assert(sizeof(ConstantBuffer) % 16 == 0);

	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		m_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
			D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);

		if (hr == E_INVALIDARG)
		{
			// DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
			hr = D3D11CreateDevice(nullptr, m_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
				D3D11_SDK_VERSION, &m_pd3dDevice, &m_featureLevel, &m_pImmediateContext);
		}

		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Obtain DXGI factory from device (since we used nullptr for pAdapter above)
	ComPtr<IDXGIFactory1> dxgiFactory;
	{
		ComPtr<IDXGIDevice> dxgiDevice;
		hr = m_pd3dDevice.As(&dxgiDevice);
		if (SUCCEEDED(hr))
		{
			ComPtr<IDXGIAdapter> adapter;
			hr = dxgiDevice->GetAdapter(&adapter);
			if (SUCCEEDED(hr))
			{
				hr = adapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
			}
		}
	}
	if (FAILED(hr))
		return hr;

	// Create swap chain
	ComPtr<IDXGIFactory2> dxgiFactory2;
	hr = dxgiFactory.As(&dxgiFactory2);
	if (dxgiFactory2)
	{
		// DirectX 11.1 or later
		hr = m_pd3dDevice.As(&m_pd3dDevice1);
		if (SUCCEEDED(hr))
		{
			(void)m_pImmediateContext.As(&m_pImmediateContext1);
		}

		DXGI_SWAP_CHAIN_DESC1 sd = {};
		sd.Width = width;
		sd.Height = height;
		sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = 1;

		hr = dxgiFactory2->CreateSwapChainForHwnd(m_pd3dDevice.Get(), hWnd, &sd, nullptr, nullptr, &m_pSwapChain1);
		if (SUCCEEDED(hr))
		{
			hr = m_pSwapChain1.As(&m_pSwapChain);
		}
	}
	else
	{
		// DirectX 11.0 systems
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 1;
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		hr = dxgiFactory->CreateSwapChain(m_pd3dDevice.Get(), &sd, &m_pSwapChain);
	}

	// Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
	dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

	if (FAILED(hr))
		return hr;

	// Create a render target view
	ComPtr<ID3D11Texture2D> pBackBuffer;
	hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	if (FAILED(hr))
		return hr;

	// Create depth buffer
	D3D11_TEXTURE2D_DESC depthStencilBufferDesc = {};
	depthStencilBufferDesc.Width = width;
	depthStencilBufferDesc.Height = height;
	depthStencilBufferDesc.MipLevels = 1;
	depthStencilBufferDesc.ArraySize = 1;
	depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilBufferDesc.SampleDesc.Count = 1;
	depthStencilBufferDesc.SampleDesc.Quality = 0;
	depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilBufferDesc.CPUAccessFlags = 0;
	depthStencilBufferDesc.MiscFlags = 0;
	hr = m_pd3dDevice->CreateTexture2D(&depthStencilBufferDesc, nullptr, &m_pDepthStencilBuffer);
	if (FAILED(hr))
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = depthStencilBufferDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	hr = m_pd3dDevice->CreateDepthStencilView(m_pDepthStencilBuffer.Get(), &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(hr))
		return hr;

	hr = m_pd3dDevice->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_pRenderTargetView);
	if (FAILED(hr))
		return hr;

	m_pImmediateContext->OMSetRenderTargets(1, m_pRenderTargetView.GetAddressOf(), m_pDepthStencilView.Get());

	// Setup the viewport
	D3D11_VIEWPORT vp;
	vp.Width = (FLOAT)width;
	vp.Height = (FLOAT)height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	m_pImmediateContext->RSSetViewports(1, &vp);


	hr = S_OK;

	const WCHAR* szVertexShaderFileName = L"shaders/triangle_VS.hlsl";
	const WCHAR* szPixelShaderFileName = L"shaders/triangle_PS.hlsl";
	ComPtr<ID3DBlob> pVSBlob;
	ComPtr<ID3DBlob> pPSBlob;

	hr = CompileShaderFromFile(szVertexShaderFileName, "VS", "vs_5_0", &pVSBlob);
	if (FAILED(hr)) {
		OutputDebugStringW(L"omg u fucked up\n");
		return hr;
	}
	hr = m_pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &m_pVertexShader);
	if (FAILED(hr)) {
		OutputDebugStringW(L"omg u fucked up\n");
		return hr;
	}

	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	hr = m_pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), &m_pVertexLayout);
	if (FAILED(hr)) {
		OutputDebugStringW(L"omg u fucked up\n");
		return hr;
	}
	m_pImmediateContext->IASetInputLayout(m_pVertexLayout.Get());


	hr = CompileShaderFromFile(szPixelShaderFileName, "PS", "ps_5_0", &pPSBlob);
	if (FAILED(hr)) {
		OutputDebugStringW(L"omg u fucked up\n");
		return hr;
	}
	hr = m_pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &m_pPixelShader);
	if (FAILED(hr)) {
		OutputDebugStringW(L"omg u fucked up\n");
		return hr;
	}

	// Create vertex buffer
	/*
	simd- single instruction multi date stream
	simd-intel -> SSE(XMM).AVX(YMM).AVX-512(ZMM)
	SSE 128bit -> 32bit float x 4ea. 한 번의 연산으로? 4개 float을 한 방에 연산한다~~
	이론상??? 속도가? 4배가 빨라진다~
	AVX 256bit -> 32bit float x 8ea. 이론상??? 속도가? 8배가 빨라진다~
	AVX 512bit -> 32bit float x 16ea. 이론상??? 속도가 거의 16배가 빨라진다~~ 물론. 코어 전압과 부하가 너무 쌔서 avx 512는 사장되었다.
	AVX -> 32bit float x 8ea. or 16bit float x 16. or 64bit float 4ea.

	*/
	// Example
	//float src[4] = { 0.1f, 0.2f, 0.3f, 0.4f };
	//float mul[4] = { 4.1f, 3.2f, 2.5f, 1.1f };

	//__m128 xmm0 = _mm_load_ps(src);
	//__m128 xmm1 = _mm_load_ps(mul);
	//		
	//__m128 xmm2 = _mm_mul_ps(xmm0, xmm1); // _mm_dot_ps
	//		
	//float res[4];
	//_mm_store_ps(res, xmm2)

	//XM = DirectX Math
	SimpleVertex vertices[] =
	{
		{
			XMFLOAT3(-1.0f, -1.0f, -1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),// R1
		},
		{
			XMFLOAT3(-1.0f, 1.0f, -1.0f),
			XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),// G1
		},
		{
			XMFLOAT3(1.0f, 1.0f, -1.0f),
			XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),// B1
		},
		{
			XMFLOAT3(1.0f, -1.0f, -1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),// R2
		},
		{
			XMFLOAT3(-1.0f, -1.0f, 1.0f),
			XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),// G2
		},
		{
			XMFLOAT3(-1.0f, 1.0f, 1.0f),
			XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f),// B2
		},
		{
			XMFLOAT3(1.0f, 1.0f, 1.0f),
			XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),// R3
		},
		{
			XMFLOAT3(1.0f, -1.0f, 1.0f),
			XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),// G3
		},
	};
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(SimpleVertex) * 8;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA VBInitData = {};
	VBInitData.pSysMem = vertices;
	hr = m_pd3dDevice->CreateBuffer(&vbd, &VBInitData, &m_pVertexBuffer);
	if (FAILED(hr))
		return hr;

	// Set vertex buffer
	UINT stride = sizeof(SimpleVertex);
	UINT voffset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, m_pVertexBuffer.GetAddressOf(), &stride, &voffset);

	// Set primitive topology
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



	// Create index buffer
	UINT indices[36] = { 0,1,2, 0,2,3, 3,2,6, 3,6,7, 7,6,5, 7,5,4, 4,5,1, 4,1,0, 1,5,6, 1,6,2, 4,0,3, 4,3,7};

	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.ByteWidth = sizeof(UINT) * 36;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA IBInitData = {};
	IBInitData.pSysMem = indices;
	hr = m_pd3dDevice->CreateBuffer(&ibd, &IBInitData, &m_pIndexBuffer);
	if (FAILED(hr))
		return hr;

	// Set index buffer
	UINT ioffset = 0;
	m_pImmediateContext->IASetIndexBuffer(m_pIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, ioffset);


	D3D11_BUFFER_DESC cbd = {  };
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = 0;

	hr = m_pd3dDevice->CreateBuffer(&cbd, nullptr, m_pConstantBuffer.GetAddressOf());
	if (FAILED(hr))
		return hr;




	OutputDebugStringW(L"InitDevice OK\n");
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Render the frame
//--------------------------------------------------------------------------------------
void Renderer::Render(const Camera& camera, double totalTime)
{
	// Just clear the backbuffer
	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView.Get(), Colors::MidnightBlue);
	m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0 );

	ConstantBuffer cb;
	cb.mWorld = XMMatrixMultiply(XMMatrixRotationX(totalTime * 0.5f), XMMatrixRotationY(totalTime));
	cb.mWorld = XMMatrixTranspose(cb.mWorld);
	cb.mView = XMMatrixTranspose(camera.View());
	cb.mProjection = XMMatrixTranspose(camera.Projection());

	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);

	m_pImmediateContext->VSSetShader(m_pVertexShader.Get(), nullptr, 0);
	m_pImmediateContext->VSSetConstantBuffers(0, 1, m_pConstantBuffer.GetAddressOf());
	m_pImmediateContext->PSSetShader(m_pPixelShader.Get(), nullptr, 0);
	m_pImmediateContext->DrawIndexed(36, 0, 0);

	cb.mWorld = XMMatrixTranslation(1.0f, 0.0f, 5.0f);
	cb.mWorld = XMMatrixTranspose(cb.mWorld);
	m_pImmediateContext->UpdateSubresource(m_pConstantBuffer.Get(), 0, nullptr, &cb, 0, 0);
	m_pImmediateContext->DrawIndexed(36, 0, 0);

	m_pSwapChain->Present(0, 0);
}


//--------------------------------------------------------------------------------------
// Unbind everything from the pipeline. The ComPtr members release themselves afterwards.
//--------------------------------------------------------------------------------------
void Renderer::CleanupDevice()
{
	if (m_pImmediateContext)
		m_pImmediateContext->ClearState();
}

Renderer::~Renderer()
{
	CleanupDevice();
	OutputDebugStringW(L"Device cleaned up\n");

	//유니코드 - UTF8 - UTF16 - UTF32 - 멀티바이트 - UTF BOM

	//cdecl, stdcall, fastcall, thiscall
	//
	//UAC와 관리자권한의 차이
}
