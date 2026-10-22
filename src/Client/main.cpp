#include <windows.h>
#include "resource.h"
#include "Renderer/Renderer.h"
#include "Renderer/Timer.h"
#include "Renderer/Camera.h"

HINSTANCE               g_hInst = nullptr;
HWND                    g_hWnd = nullptr;

// -------------------------
// forward declarations
// -------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdshow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


HRESULT InitWindow(HINSTANCE hInsatnce, int nCmdShow)
{
	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInsatnce;
	wcex.hIcon = LoadIcon(hInsatnce, (LPCTSTR)IDI_TUTORIAL1);
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_3DDKSHADOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = L"TutorialWindowClass";
	wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TUTORIAL1);
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	g_hInst = hInsatnce;
	RECT rc = { 0,0,800,600 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindow(L"TutorialWindowClass", L"Direct3D 11 Tutorial 1: Direct3D 11 Basics",
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInsatnce,
		nullptr);
	if (!g_hWnd)
		return E_FAIL;
	ShowWindow(g_hWnd, nCmdShow);
	return S_OK;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(pCmdLine);

	if (FAILED(InitWindow(hInstance, nCmdShow)))
		return 0;

	Timer timer;
	Camera camera;
	RECT rc;
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;
	camera.SetAspect(static_cast<float>(width) / static_cast<float>(height));

	Renderer renderer;
	if (FAILED(renderer.InitDevice(g_hWnd)))
		return 0;

	MSG msg = { 0 };

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			// Should Fix like this

			// timer.Tick();
			// float dt = timer.DeltaTime();

			// accumulator += dt;
			// while (accumulator >= FIXED_DT)      ← 고정 간격 단계(물리)
			// {
			// 	FixedUpdate(FIXED_DT);
			//	 accumulator -= FIXED_DT;
			// }

			// Update(dt);                          ← 프레임마다(입력, 카메라)
			// renderer.Render(…);                  ← 그리기만
			timer.Tick();

			//DirectX::XMFLOAT3 pos = camera.GetPosition();
			//pos.x = sinf(timer.TotalTime());
			//pos.y = 0.0f;
			//pos.z = -0.5f;
			//camera.SetPosition(pos);

			renderer.Render(camera, timer.TotalTime());
		}
	}

	return (int)msg.wParam;
}

//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

		// Note that this tutorial does not handle resizing (WM_SIZE) requests,
		// so we created the window without the resize border.

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}
