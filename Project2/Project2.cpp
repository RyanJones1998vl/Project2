#include<windows.h>
#include<d3d10.h>
#include<d3d9.h>;

HINSTANCE hInst;		//global handle to hold the application instance
HWND hWnd;			//global variable to hold the window handle
//forward declarations
int GameInit(HWND hWnd);
void GameRun(HWND hWnd);
void GameEnd(void);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void render(void);
LPDIRECT3D9 pD3D;
LPDIRECT3DDEVICE9 pd3dDevice;
void cleanUp(void);
//This is winmain, the main entry point fro windows applications
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc;
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.hInstance = hInstance;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbWndExtra = 0;
	wc.cbClsExtra = 0;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "APP_CLASS";
	wc.hIconSm = NULL;

	RegisterClassEx(&wc);

	HWND hWnd = CreateWindow("APP_CLASS",
		"APP_CLASS",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		500,
		500,
		NULL,
		NULL,
		hInstance,
		NULL);
	if (!hWnd) return FALSE;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);


	DWORD frame_start = GetTickCount();
	DWORD count_per_frame = 1000 / 60;
	MSG msg;
	if (!GameInit(hWnd)) return FALSE;
	int done = 0;
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) done=1;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		DWORD now = GetTickCount();
		if (now - frame_start >= count_per_frame) {
			frame_start = now;
			GameRun(hWnd);
		}
	} 
	cleanUp();
}
LPDIRECT3DSURFACE9 back_buffer = NULL;
LPDIRECT3DSURFACE9 surface = NULL;
int GameInit(HWND hWnd) {
	pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	pd3dDevice = NULL;
	
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.BackBufferCount = 1;
	d3dpp.BackBufferHeight = 500;
	d3dpp.BackBufferWidth = 500;
	d3dpp.hDeviceWindow = hWnd;

//Create a default DirectX Device
	if (FAILED(pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_REF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &pd3dDevice)))
		return false;


	return true;
		
}
int left = 0, top = 0;
void GameRun(HWND hWnd) {
	pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 255, 255), 1.0f, 0);

	pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
	int result = pd3dDevice->CreateOffscreenPlainSurface(100, 100, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &surface, NULL);

	if (pd3dDevice->BeginScene()) {

		int r = rand()%255;
		int g = 100;
		int b = 100;
		
		pd3dDevice->ColorFill(surface, NULL, D3DCOLOR_XRGB(r, g, b));
		
		RECT rect;
		if (left < 400 && top == 0) {
			left += 10;
			rect.left = left;
			rect.top = top;
			rect.right = rect.left + 100;
			rect.bottom = rect.top + 100;
		}
		else if (left == 400 && top <400) {
			top += 10;
			rect.left = left;
			rect.top = top;
			rect.right = rect.left + 100;
			rect.bottom = rect.top + 100;
		}
		else if (left > 0 && top <= 400 && top!=0) {
			left -= 10;
			rect.left = left;
			rect.top = top;
			rect.right = rect.left + 100;
			rect.bottom = rect.top + 100;
		}
		else{
			top -= 10;
			rect.left = left;
			rect.top = top;
			rect.right = rect.left + 100;
			rect.bottom = rect.top + 100;
		}

		
		pd3dDevice->StretchRect(surface, NULL, back_buffer, &rect, D3DTEXF_NONE);
		pd3dDevice->EndScene();
	}
	pd3dDevice->Present(NULL, NULL, NULL, NULL);
}
void cleanUp(void) {
	if (pd3dDevice != NULL) {
		pd3dDevice->Release();
	}

	if (pD3D != NULL) {
		pD3D->Release();
	}
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	//check any available messages from the queue
	switch (message) {
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	//always return the message to the default window
	//procedure for further processing
	return DefWindowProc(hWnd, message, wParam, lParam);
}