#include<windows.h>
#include<d3d10.h>
#include<d3d9.h>;
#include<dinput.h>;

HINSTANCE hInst;		//global handle to hold the application instance
HWND hWnd;			//global variable to hold the window handle


//Essential functions 
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int GameInit(HINSTANCE hInstance, HWND hWnd);
void GameRun(HWND hWnd);
void GameEnd(void);


//init functions
int InitKeyboard(HINSTANCE hIntance, HWND hWnd);

//rende function 
void render(void);

//Global variables
LPDIRECT3D9 pD3D;
LPDIRECT3DDEVICE9 pd3dDevice;

HRESULT result; //variable used to hold return codes
LPDIRECTINPUTDEVICE8 DI_Device; // the DirectInput object
LPDIRECTINPUT8 DI_Object;
HRESULT Acquire(VOID);
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
	if (!GameInit(hInstance, hWnd)) return FALSE;
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
int GameInit(HINSTANCE hInstance, HWND hWnd) {
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


	return InitKeyboard(hInstance, hWnd);
		
}
float vPar = 10, vxBall = 10, vyBall = -10;
#define KEYDOWN(name, key) (name[key] & 0x80)
int InitKeyboard(HINSTANCE hIntance, HWND hWnd) {
	
	//Create the DirectInput object
	result = DirectInput8Create(hIntance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)& DI_Object, NULL);
	result = DI_Object->CreateDevice(GUID_SysKeyboard, &DI_Device, NULL);
	result = DI_Device->SetDataFormat(&c_dfDIKeyboard);
	result = DI_Device->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	result = DI_Device->Acquire();
	//check the return code for DirectInput8Create
	if FAILED(result) {
		return false;
	}
	return true;

}


RECT par_1, par_2, ball;
RECT aBound, bBound;
RECT rect;
bool start = true;

float isCollision(RECT b1, RECT b2, float vx, float vy) {
	float xInvEntry, yInvEntry;
	float xInvExit, yInvExit;
	if (vx > 0.0f) {
		xInvEntry = (b1.left - b2.right);
		xInvExit = (b1.right - b2.left);
	}
	else {
		xInvEntry = (b1.right - b2.left);
		xInvExit = (b1.left - b2.right);
	}
	if (vy > 0.0f) {
		yInvEntry = (b1.top - b2.bottom);
		yInvExit = (b1.bottom - b2.top);
	}
	else {
		yInvEntry = (b1.bottom - b2.top);
		yInvExit = (b1.top - b2.bottom);
	}
	float xEntry, yEntry;
	float xExit, yExit;
	if (vx == 0.0f) {
		xEntry = -9999999;
		xExit = 9999999;
	}
	else {
		xEntry = xInvEntry / vx;
		xExit = xInvExit / vx;
	}
	if (vy == 0.0f) {
		yEntry = -9999999;
		yExit = 9999999;
	}
	else {
		yEntry = yInvEntry / vy;
		yExit = yInvExit / vy;
	}
	float entryTime = max(xEntry, yEntry);
	float exitTime = min(xExit, yExit);
	if (entryTime > exitTime || (xEntry < 0.0f && yEntry < 0.0f) || xEntry > 1.0f || yEntry > 1.0f)
	{
		return 1.0f;
	}
	return entryTime;
}
float timeCollision(RECT b1, RECT b2) {
	float time = isCollision(b1, b2, vxBall, vyBall);
	return time;
}
#define KEYDOWN(name, key) (name[key] & 0x80)
char buffer[256];

void GameRun(HWND hWnd) {
	DI_Device->GetDeviceState(sizeof(buffer), &buffer);
	if (KEYDOWN(buffer, DIK_DOWNARROW)) {
		par_2.top += 10;
		if (par_2.top >= 200) par_2.top = 200;
		par_2.left = 490;
		par_2.right = par_2.left + 10;
		par_2.bottom = par_2.top + 300;

		par_1.top += 10;
		if (par_1.top >= 200) par_1.top = 200;
		par_1.left = 0;
		par_1.right = par_1.left + 10;
		par_1.bottom = par_1.top + 300;
	}
	else if (KEYDOWN(buffer, DIK_UPARROW)) {
		par_2.top -= 10;
		if (par_2.top <= 0) par_2.top = 0;
		par_2.left = 490;
		par_2.right = par_2.left + 10;
		par_2.bottom = par_2.top + 300;

		par_1.top -= 10;
		if (par_1.top <= 0) par_1.top = 0;
		par_1.left = 0;
		par_1.right = par_1.left + 10;
		par_1.bottom = par_1.top + 300;
	} 
	pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 255, 255), 1.0f, 0);

	pd3dDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer);
	int result = pd3dDevice->CreateOffscreenPlainSurface(100, 100, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &surface, NULL);

	if (pd3dDevice->BeginScene()) {

		int r = rand()%255;
		int g = 100;
		int b = 100;
		
		pd3dDevice->ColorFill(surface, NULL, D3DCOLOR_XRGB(r, g, b));

		aBound.left = 0;
		aBound.right = 500;
		aBound.top = 0;
		aBound.bottom = 1;

		bBound.left = 0;
		bBound.right = 500;
		bBound.top = 499;
		bBound.bottom = 500;
		
		float isLC = timeCollision(par_1, ball);
		float isRC = timeCollision(par_2, ball);
		float isAC = timeCollision(aBound, ball);
		float isBC = timeCollision(bBound, ball);
		if (isLC != 1.0f) {
			ball.left += vxBall * (isLC);

			vxBall = -vxBall;
		}else if (isRC != 1.0f){
			ball.left += vxBall * (isRC);
			vxBall = -vxBall;
		}
		else if (isAC != 1.0f) {
			ball.top += vyBall * (isAC);
			vyBall = -vyBall;

		}
		else if (isBC != 1.0f) {
			ball.top += vyBall * (isBC);
			vyBall = -vyBall;

		}else {
			ball.top += vyBall;
			ball.left += vxBall;
		}
		if (start) {
			ball.left = 250;
			ball.top = 400;
			ball.bottom = ball.top + 10;
			ball.right = ball.left + 10;
			start = !start;


			par_1.left = 0;
			par_1.top = 100;
			par_1.right = par_1.left + 10;
			par_1.bottom = par_1.top + 300;

			par_2.left = 490;
			par_2.top = 100;
			par_2.right = par_2.left + 10;
			par_2.bottom = par_2.top + 300;
		}
		else {
			ball.bottom = ball.top + 10;
			ball.right = ball.left + 10;
		}
		
		pd3dDevice->StretchRect(surface, NULL, back_buffer, &par_1, D3DTEXF_NONE);
		pd3dDevice->StretchRect(surface, NULL, back_buffer, &par_2, D3DTEXF_NONE);
		pd3dDevice->StretchRect(surface, NULL, back_buffer, &ball, D3DTEXF_NONE);
		pd3dDevice->StretchRect(surface, NULL, back_buffer, &aBound, D3DTEXF_NONE);
		pd3dDevice->StretchRect(surface, NULL, back_buffer, &bBound, D3DTEXF_NONE);
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
	if (DI_Object != NULL) {
		if (DI_Device != NULL) {
			DI_Device->Unacquire();
			DI_Device->Release();
			DI_Device = NULL;
		}
		DI_Object->Release();
		DI_Object = NULL;
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
struct Box {
	RECT rect;
	float x, y;
	float w, h;
	float vx, vy;
};