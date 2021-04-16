#include <cmath>
#include <windows.h>
#include <thread>
#include <mutex>
using namespace std;

mutex mtx;

const double overall_timeD = 550.0;
const int fps = 25;

const double verticalBallSpeed = 5;
const double horizontalBallSpeed = 7;

const double overall_frames = 1000;
const int frame_timing = overall_frames / fps;

const int tileSize = 100, checkerRadius = 35;

LONG WINAPI WndProc(HWND, UINT, WPARAM, LPARAM);

void RenderTile(HDC hdc, int x, int y) {
	HBRUSH hBrush = CreateSolidBrush(RGB(90, 90, 90));
	HPEN hpen = CreatePen(PS_NULL, 0, RGB(90, 90, 90));

	SelectObject(hdc, hBrush);
	SelectObject(hdc, hpen);

	Rectangle(hdc, x, y, x + tileSize, y + tileSize);

	DeleteObject(hBrush);
	DeleteObject(hpen);
}
void RenderBlackChessLine(HDC hdc, int y) {
	HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 0));
	HPEN hpen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));

	SelectObject(hdc, hBrush);
	SelectObject(hdc, hpen);

	for (int i = 0; i < 4; i++) {
		RenderTile(hdc, i * 2 * tileSize, y);
	}
	DeleteObject(hBrush);
	DeleteObject(hpen);
}
void RenderWhiteChessLine(HDC hdc, int y) {
	HBRUSH hBrush = CreateSolidBrush(RGB(90, 90, 90));
	HPEN hpen = CreatePen(PS_NULL, 0, RGB(90, 90, 90));

	SelectObject(hdc, hBrush);
	SelectObject(hdc, hpen);

	for (int i = 0; i < 4; i++) {
		RenderTile(hdc, i * 2 * tileSize + tileSize, y);
	}
	DeleteObject(hBrush);
	DeleteObject(hpen);
}

void RenderCheckerInsideTile(HDC hdc, int red, int green, int blue, int x, int y) {
	HBRUSH hBrush = CreateSolidBrush(RGB(red, green, blue));
	HPEN hpen = CreatePen(PS_NULL, 0, RGB(red, green, blue));

	SelectObject(hdc, hBrush);
	SelectObject(hdc, hpen);

	Ellipse(hdc, x + 15, y + 15, x + 85, y + 85);

	DeleteObject(hBrush);
	DeleteObject(hpen);
}

void RenderDesk(HDC hdc) {
	for (int i = 0; i < 8; i++) {
		if (i % 2 == 0)
			RenderBlackChessLine(hdc, i * tileSize);
		else RenderWhiteChessLine(hdc, i * tileSize);
	}
}
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASS w;
	memset(&w, 0, sizeof(WNDCLASS));

	w.style = CS_HREDRAW | CS_VREDRAW;
	w.lpfnWndProc = WndProc;
	w.hInstance = hInstance;
	w.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));
	w.lpszClassName = L"My Class";

	RegisterClass(&w);

	HWND hwnd =
		CreateWindow(L"My Class", L"Шашки", WS_OVERLAPPEDWINDOW,
			370, 5, 815, 850, NULL, NULL, hInstance, NULL);

	ShowWindow(hwnd, nCmdShow);

	//thread THVerticalBall(moveVerticalBall);
	//thread THHorizontalBall(moveHorizontalBall);
	//thread THTrafficLights(trafficLightsBlinking);

	for (int i = 0; i < overall_timeD; i++) {
		InvalidateRect(hwnd, 0, 1);
		UpdateWindow(hwnd);
		Sleep(frame_timing);
	}
	Sleep(frame_timing * 40);
}
LONG WINAPI WndProc(HWND hwnd, UINT Message, WPARAM wparam, LPARAM lparam) {
	HDC hdc;
	PAINTSTRUCT ps;
	HPEN hpen;

	switch (Message) {
	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		RenderDesk(hdc);
		EndPaint(hwnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, Message, wparam, lparam);
	}

	return 0;
}