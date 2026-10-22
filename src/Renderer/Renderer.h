#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <wrl/client.h>
#include "Renderer/Camera.h"

class Renderer {
	template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	Renderer() = default;
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	~Renderer();

	HRESULT InitDevice(HWND hWnd);

	void Render(const Camera& camera, double totalTime);

private:
	void CleanupDevice();

	D3D_DRIVER_TYPE         m_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL       m_featureLevel = D3D_FEATURE_LEVEL_11_0;

	// Members are destroyed in reverse declaration order: buffers first, device last.
	ComPtr<ID3D11Device> m_pd3dDevice;
	ComPtr<ID3D11Device1> m_pd3dDevice1;
	ComPtr<ID3D11DeviceContext> m_pImmediateContext;
	ComPtr<ID3D11DeviceContext1> m_pImmediateContext1;
	ComPtr<IDXGISwapChain> m_pSwapChain;
	ComPtr<IDXGISwapChain1> m_pSwapChain1;
	ComPtr<ID3D11RenderTargetView> m_pRenderTargetView;

	ComPtr<ID3D11VertexShader> m_pVertexShader;
	ComPtr<ID3D11PixelShader> m_pPixelShader;
	ComPtr<ID3D11InputLayout> m_pVertexLayout;
	ComPtr<ID3D11Buffer> m_pVertexBuffer;
	ComPtr<ID3D11Buffer> m_pIndexBuffer;
	ComPtr<ID3D11Buffer> m_pConstantBuffer;

	ComPtr<ID3D11Texture2D> m_pDepthStencilBuffer;
	ComPtr<ID3D11DepthStencilView> m_pDepthStencilView;

	//QueryPerformanceCounter time;
};

