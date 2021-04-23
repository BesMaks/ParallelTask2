#include <cmath>
#include <windows.h>
#include <thread>
#include <mutex>
#include <time.h>
using namespace std;

mutex mtx;
LONG WINAPI WndProc(HWND, UINT, WPARAM, LPARAM);

const double overall_timeD = 550.0;
const int fps = 25;
const double overall_frames = 2000;
const int frame_timing = overall_frames / fps * 9;

const int tileSize = 100, checkerRadius = 35;
/*в массиве обозначим
	-1 -> белая клетка
	 0 -> пустая черная клетка

	 1 -> черная клетка с белой шашкой
	 2 -> черная клетка с красной шашкой
	 3 -> черная клетка с чёрной шашкой
	 4 -> черная клетка с зелёной шашкой
*/
int desk[8][8];

void RenderTile(HDC hdc, int x, int y) {
	HBRUSH hBrush = CreateSolidBrush(RGB(90, 90, 90));
	HPEN hpen = CreatePen(PS_NULL, 0, RGB(90, 90, 90));

	SelectObject(hdc, hBrush);
	SelectObject(hdc, hpen);

	Rectangle(hdc, x, y, x + tileSize, y + tileSize);

	DeleteObject(hBrush);
	DeleteObject(hpen);
}
void RenderCheckerInsideTile(HDC hdc, int red, int green, int blue, int x, int y) {
	HBRUSH hBrush = CreateSolidBrush(RGB(red, green, blue));
	HPEN hpen = CreatePen(PS_NULL, 0, RGB(red, green, blue));

	SelectObject(hdc, hBrush);
	SelectObject(hdc, hpen);
	x *= tileSize; y *= tileSize;
	Ellipse(hdc, y + 85, x + 85, y + 15, x + 15);

	DeleteObject(hBrush);
	DeleteObject(hpen);

	//Sleep(fps);
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

void initRandomCheckersDesk() {
	//Шашки для обеих сторон имеют равные пропорции по цветам
	srand(time(NULL));
	int whiteBlackCheckersAmount = rand() % 9 + 2;
	int redGreenCheckersAmount = 12 - whiteBlackCheckersAmount;

	/*в массиве обозначим
		-1 -> белая клетка
		 0 -> пустая черная клетка

		 1 -> черная клетка с белой шашкой
		 2 -> черная клетка с красной шашкой
		 3 -> черная клетка с чёрной шашкой
		 4 -> черная клетка с зелёной шашкой
	*/

	bool whiteStartColour = 0;
	for (int i = 0; i < 8; i++) {
		if (i % 2 == 0) whiteStartColour = 0;
		else whiteStartColour = 1;
		for (int j = 0; j < 8; j++) {
			if (whiteStartColour) desk[i][j] = -1;
			else desk[i][j] = 0;
			whiteStartColour = !whiteStartColour;
		}
	}

	int whiteSum = 0, redSum = 0;
	//заполнение верхних трёх строк доски
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 8; j++) {
			if (desk[i][j] == 0 && whiteSum < whiteBlackCheckersAmount) {
				desk[i][j] = 1;
				whiteSum++;
			}
			if (desk[i][j] == 0 && whiteSum >= whiteBlackCheckersAmount) {
				desk[i][j] = 2;
				redSum++;
			}
		}

	//заполнение нижних трёх строк доски
	int blackSum = 0, greenSum = 0;
	for (int i = 5; i < 8; i++)
		for (int j = 0; j < 8; j++) {
			if (desk[i][j] == 0 && blackSum < whiteBlackCheckersAmount) {
				desk[i][j] = 3;
				blackSum++;
			}
			if (desk[i][j] == 0 && blackSum >= whiteBlackCheckersAmount) {
				desk[i][j] = 4;
				greenSum++;
			}
		}
}
void RenderDesk(HDC hdc) {
	for (int i = 0; i < 8; i++) {
		if (i % 2 == 0)
			RenderBlackChessLine(hdc, i * tileSize);
		else RenderWhiteChessLine(hdc, i * tileSize);
	}
}
void RenderCurrentCheckersOnDesk(HDC hdc) {
	RenderDesk(hdc);
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++) {
			switch (desk[i][j])
			{
			case 1: {
				RenderCheckerInsideTile(hdc, 255, 255, 255, i, j);
				break;
			}
			case 2: {
				RenderCheckerInsideTile(hdc, 220, 0, 0, i, j);
				break;
			}
			case 3: {
				RenderCheckerInsideTile(hdc, 0, 0, 0, i, j);
				break;
			}
			case 4: {
				RenderCheckerInsideTile(hdc, 0, 220, 0, i, j);
				break;
			}
			default:
				break;
			}
		}
	Sleep(frame_timing);
}

/// <summary>
/// moves checker at pointed tile
/// </summary>
/// <param name="x">row where checker now is</param>
/// <param name="y">column where checker now is</param>
/// <returns></returns>
pair<int, int> moveCheckerTopRight(int x, int y) {
	mtx.lock();
	int t = desk[x][y];
	desk[x][y] = desk[x - 1][y + 1];
	desk[x - 1][y + 1] = t;
	mtx.unlock();
	pair<int, int> p = make_pair(x - 1, y + 1);
	return p;
}
/// <summary>
/// moves checker at pointed tile
/// </summary>
/// <param name="x">row where checker now is</param>
/// <param name="y">column where checker now is</param>
/// <returns></returns>
pair<int, int> moveCheckerBotRight(int x, int y) {
	mtx.lock();
	int t = desk[x][y];
	desk[x][y] = desk[x + 1][y + 1];
	desk[x + 1][y + 1] = t;
	mtx.unlock();
	pair<int, int> p = make_pair(x + 1, y + 1);
	return p;
}
/// <summary>
/// moves checker at pointed tile
/// </summary>
/// <param name="x">row where checker now is</param>
/// <param name="y">column where checker now is</param>
/// <returns></returns>
pair<int, int> moveCheckerTopLeft(int x, int y) {
	mtx.lock();
	int t = desk[x][y];
	desk[x][y] = desk[x - 1][y - 1];
	desk[x - 1][y - 1] = t;
	mtx.unlock();
	pair<int, int> p = make_pair(x - 1, y - 1);
	return p;
}
/// <summary>
/// moves checker at pointed tile
/// </summary>
/// <param name="x">row where checker now is</param>
/// <param name="y">column where checker now is</param>
/// <returns></returns>
pair<int, int> moveCheckerBotLeft(int x, int y) {
	mtx.lock();
	int t = desk[x][y];
	desk[x][y] = desk[x + 1][y - 1];
	desk[x + 1][y - 1] = t;
	mtx.unlock();
	pair<int, int> p = make_pair(x + 1, y - 1);
	return p;
}

pair<int, int> takeCheckerTopRight(int x, int y) {
	mtx.lock();
	desk[x - 2][y + 2] = desk[x][y];
	desk[x][y] = 0;
	desk[x - 1][y + 1] = 0;
	mtx.unlock();
	pair<int, int> p = make_pair(x - 2, y + 2);
	return p;
}
pair<int, int> takeCheckerBotRight(int x, int y) {
	mtx.lock();
	desk[x + 2][y + 2] = desk[x][y];
	desk[x][y] = 0;
	desk[x + 1][y + 1] = 0;
	mtx.unlock();
	pair<int, int> p = make_pair(x + 2, y + 2);
	return p;
}
pair<int, int> takeCheckerTopLeft(int x, int y) {
	mtx.lock();
	desk[x - 2][y - 2] = desk[x][y];
	desk[x][y] = 0;
	desk[x - 1][y - 1] = 0;
	mtx.unlock();
	pair<int, int> p = make_pair(x - 2, y - 2);
	return p;
}
pair<int, int> takeCheckerBotLeft(int x, int y) {
	mtx.lock();
	desk[x + 2][y - 2] = desk[x][y];
	desk[x][y] = 0;
	desk[x + 1][y - 1] = 0;
	mtx.unlock();
	pair<int, int> p = make_pair(x + 2, y - 2);
	return p;
}

pair<int, int> jumpOverCheckerTopRight(int x, int y) {
	mtx.lock();
	desk[x - 2][y + 2] = desk[x][y];
	desk[x][y] = 0;
	mtx.unlock();
	pair<int, int> p = make_pair(x - 2, y + 2);
	return p;
}
pair<int, int> jumpOverCheckerBotRight(int x, int y) {
	mtx.lock();
	desk[x + 2][y + 2] = desk[x][y];
	desk[x][y] = 0;
	mtx.unlock();
	pair<int, int> p = make_pair(x + 2, y + 2);
	return p;
}
pair<int, int> jumpOverCheckerTopLeft(int x, int y) {
	mtx.lock();
	desk[x - 2][y - 2] = desk[x][y];
	desk[x][y] = 0;
	mtx.unlock();
	pair<int, int> p = make_pair(x - 2, y - 2);
	return p;
}
pair<int, int> jumpOverCheckerBotLeft(int x, int y) {
	mtx.lock();
	desk[x + 2][y - 2] = desk[x][y];
	desk[x][y] = 0;
	mtx.unlock();
	pair<int, int> p = make_pair(x + 2, y - 2);
	return p;
}

bool isContraryCheckerToThis(int x, int y, int xToCheck, int yToCheck) {
	int firstColour = desk[x][y];
	int colourToCompare = desk[xToCheck][yToCheck];
	if (abs(firstColour - colourToCompare) == 2 && colourToCompare != 0) return 1;
	else return 0;
}
bool isLeftEdge(int x, int y) {
	return (x % 2 == 0 && y == 0);
}
bool isRightEdge(int x, int y) {
	return (x % 2 == 1 && y == 7);
}
bool isTopEdge(int x, int y) {
	return (x == 0 && y % 2 == 0);
}
bool isBotEdge(int x, int y) {
	return (x == 7 && y % 2 == 1);
}

/// <summary>
/// checks if a checker can go
/// </summary>
/// <param name="player">
/// <para>:</para>
/// <para>1 -> white + red checkers player</para>
/// <para>0 -> black + green checkers player</para>
/// </param>
/// <returns>
/// <para>0 -> nowhere to go</para>
/// <para>1 -> can go left</para>
/// <para>2 -> can go right</para>
/// <para>3 -> can jump over left (depends on player)</para>
/// <para>4 -> can jump over right (depends on player)</para>
/// </returns>
int checkWhereCanGo(int x, int y, bool player) {
	bool rightFree = 1;
	bool leftFree = 1;

	bool LeftEdge = isLeftEdge(x, y);
	bool RightEdge = isRightEdge(x, y);
	bool TopEdge = isTopEdge(x, y);
	bool BotEdge = isBotEdge(x, y);

	if (player) {
		rightFree = 1;
		leftFree = 1;
		mtx.lock();
		bool canJumpOverBotLeft = 0;
		if (isContraryCheckerToThis(x, y, x + 1, y - 1) &&
			desk[x + 2][y - 2] == 0 &&
			!isLeftEdge(x + 1, y + 1) &&
			!isBotEdge(x + 1, y + 1))
			canJumpOverBotLeft = 1;

		bool canJumpOverBotRight = 0;
		if (isContraryCheckerToThis(x, y, x + 1, y + 1) &&
			desk[x + 2][y + 2] == 0 &&
			!isRightEdge(x + 1, y + 1) &&
			!isBotEdge(x + 1, y + 1))
			canJumpOverBotRight = 1;

		if (desk[x + 1][y - 1] != 0 || LeftEdge)
			leftFree = 0;
		if (desk[x + 1][y + 1] != 0 || RightEdge)
			rightFree = 0;
		mtx.unlock();
		if (BotEdge) return 0;
		if (canJumpOverBotLeft) return 3;
		if (canJumpOverBotRight) return 4;
	}
	else {
		rightFree = 1;
		leftFree = 1;
		mtx.lock();
		bool canJumpOverTopLeft = 0;
		if (isContraryCheckerToThis(x, y, x - 1, y - 1) &&
			desk[x - 2][y - 2] == 0 &&
			!isLeftEdge(x - 1, y - 1) &&
			!isTopEdge(x - 1, y - 1))
			canJumpOverTopLeft = 1;

		bool canJumpOverTopRight = 0;
		if (isContraryCheckerToThis(x, y, x - 1, y + 1) &&
			desk[x - 2][y + 2] == 0 &&
			!isRightEdge(x - 1, y + 1) &&
			!isTopEdge(x - 1, y + 1))
			canJumpOverTopRight = 1;

		if (desk[x - 1][y - 1] != 0 || LeftEdge) leftFree = 0;
		if (desk[x - 1][y + 1] != 0 || RightEdge) rightFree = 0;
		mtx.unlock();
		if (TopEdge) return 0;
		if (canJumpOverTopLeft) return 3;
		if (canJumpOverTopRight) return 4;
	}

	if (leftFree) return 1;
	if (rightFree) return 2;
	if (!leftFree && !rightFree) return 0;
}
/// <summary>
/// Says where this checker can go or which enemy can it beat
/// </summary>
/// <returns>
/// <para> 0 -> can't GO anywhere</para>
/// <para> 1 -> can take left checker</para>
/// <para> 2 -> can take right checker</para>
/// <para>3 -> can jump over left</para>
/// <para>4 -> can jump over right</para>
/// <para> 5 -> can GO left</para>
/// <para> 6 -> can GO right</para>
/// </returns>
int optionsForWhiteRedChecker(int x, int y, bool player) {
	int possibleWays = checkWhereCanGo(x, y, player);
	if (possibleWays == 0) return 0;

	//checkings if enemy checker has appropriate colour and isn't at the edge of desk and isn't covered behind with ally checker
	if (!isLeftEdge(x + 1, y - 1) &&
		!isBotEdge(x + 1, y - 1) &&
		!isLeftEdge(x, y) &&
		!isBotEdge(x, y) &&
		desk[x + 2][y - 2] == 0 &&
		isContraryCheckerToThis(x, y, x + 1, y - 1))
		return 1;
	if (possibleWays == 1) return 5;

	if (!isRightEdge(x + 1, y + 1) &&
		!isBotEdge(x + 1, y + 1) &&
		!isRightEdge(x, y) &&
		!isBotEdge(x, y) &&
		desk[x + 2][y + 2] == 0 &&
		isContraryCheckerToThis(x, y, x + 1, y + 1))
		return 2;
	if (possibleWays == 2) return 6;
	//can jump over enemy?
	if (possibleWays == 3) return 3;
	if (possibleWays == 4) return 4;
}
/// <summary>
/// Says where this checker can go or which enemy can it beat
/// </summary>
/// <returns>
/// <para> 0 -> can't GO anywhere</para>
/// <para> 1 -> can take left checker</para>
/// <para> 2 -> can take right checker</para>
/// <para>3 -> can jump over left</para>
/// <para>4 -> can jump over right</para>
/// <para> 5 -> can GO left</para>
/// <para> 6 -> can GO right</para>
/// </returns>
int optionsForBlackGreenChecker(int x, int y, bool player) {
	int possibleWays = checkWhereCanGo(x, y, player);
	if (possibleWays == 0) return 0;

	//checkings if enemy checker has appropriate colour and isn't at the edge of desk and isn't covered behind with ally checker
	//can take top&left enemy?
	if (!isLeftEdge(x - 1, y - 1) &&
		!isTopEdge(x - 1, y - 1) &&
		!isLeftEdge(x, y) &&
		!isTopEdge(x, y) &&
		desk[x - 2][y - 2] == 0 &&
		isContraryCheckerToThis(x, y, x - 1, y - 1))
		return 1;
	if (possibleWays == 1) return 5;
	//can take top&right enemy?
	if (!isRightEdge(x - 1, y + 1) &&
		!isTopEdge(x - 1, y + 1) &&
		!isRightEdge(x, y) &&
		!isTopEdge(x, y) &&
		desk[x - 2][y + 2] == 0 &&
		isContraryCheckerToThis(x, y, x - 1, y + 1))
		return 2;
	if (possibleWays == 2) return 6;
	//can jump over enemy?
	if (possibleWays == 3) return 3;
	if (possibleWays == 4) return 4;
}

void makeWhiteRedRandomTurn() {
	bool turnMade = 0;
	this_thread::sleep_for(chrono::milliseconds(frame_timing * 2));
	//сначала рассмотрим варианты, когда можно побить
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++) {
			if (desk[i][j] == 1 || desk[i][j] == 2)
				switch (optionsForWhiteRedChecker(i, j, 1))
				{
				case 0:
					break;
				case 1:
					turnMade = 1;
					takeCheckerBotLeft(i, j);
					break;
				case 2:
					turnMade = 1;
					takeCheckerBotRight(i, j);
					break;
				default:
					break;
				}
			if (turnMade) return;
		}
	//варианты перескакивания
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++) {
			if (desk[i][j] == 1 || desk[i][j] == 2)
				switch (optionsForWhiteRedChecker(i, j, 1))
				{
				case 0:
					break;
				case 3:
					turnMade = 1;
					jumpOverCheckerBotLeft(i, j);
					break;
				case 4:
					turnMade = 1;
					jumpOverCheckerBotRight(i, j);
					break;
				default:
					break;
				}
			if (turnMade) return;
		}
	//варианты обычного хода
	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++) {
			if (desk[i][j] == 1 || desk[i][j] == 2)
				switch (optionsForWhiteRedChecker(i, j, 1))
				{
				case 0:
					break;
				case 5:
					turnMade = 1;
					moveCheckerBotLeft(i, j);
					break;
				case 6:
					turnMade = 1;
					moveCheckerBotRight(i, j);
					break;
				default:
					break;
				}
			if (turnMade) return;
		}
}
void whiteRedGameCycle() {
	for (int time = 0; time < overall_timeD; time++) {
		makeWhiteRedRandomTurn();
		this_thread::sleep_for(chrono::milliseconds(frame_timing * 6));
	}
}

void makeBlackGreenRandomTurn() {
	bool turnMade = 0;
	this_thread::sleep_for(chrono::milliseconds(frame_timing * 6));
	//сначала рассмотрим варианты, когда можно побить
	for (int i = 7; i >= 0; i--)
		for (int j = 7; j >= 0; j--) {
			if (desk[i][j] == 3 || desk[i][j] == 4)
				switch (optionsForBlackGreenChecker(i, j, 0))
				{
				case 0:
					break;
				case 1:
					turnMade = 1;
					takeCheckerTopLeft(i, j);
					break;
				case 2:
					turnMade = 1;
					takeCheckerTopRight(i, j);
					break;
				default:
					break;
				}
			if (turnMade) return;
		}
	//варианты перескакивания
	for (int i = 7; i >= 0; i--)
		for (int j = 7; j >= 0; j--) {
			if (desk[i][j] == 3 || desk[i][j] == 4)
				switch (optionsForBlackGreenChecker(i, j, 0))
				{
				case 0:
					break;
				case 3:
					turnMade = 1;
					jumpOverCheckerTopLeft(i, j);
					break;
				case 4:
					turnMade = 1;
					jumpOverCheckerTopRight(i, j);
					break;
				default:
					break;
				}
			if (turnMade) return;
		}
	//варианты обычного хода
	for (int i = 7; i >= 0; i--)
		for (int j = 7; j >= 0; j--) {
			if (desk[i][j] == 3 || desk[i][j] == 4)
				switch (optionsForBlackGreenChecker(i, j, 0))
				{
				case 0:
					break;
				case 5:
					turnMade = 1;
					moveCheckerTopLeft(i, j);
					break;
				case 6:
					turnMade = 1;
					moveCheckerTopRight(i, j);
					break;
				default:
					break;
				}
			if (turnMade) return;
		}
}
void blackGreenGameCycle() {
	for (int time = 0; time < overall_timeD; time++) {
		makeBlackGreenRandomTurn();
		this_thread::sleep_for(chrono::milliseconds(frame_timing * 2));
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
	initRandomCheckersDesk();

	//moveCheckerTopRight(5, 1);
	//moveCheckerTopRight(5, 5);
	//moveCheckerTopLeft(6, 6);

	thread THWhiteRedPlayer(whiteRedGameCycle);
	thread THBlackGreenPlayer(blackGreenGameCycle);
	Sleep(frame_timing * 2);

	THWhiteRedPlayer.detach();
	THBlackGreenPlayer.detach();
	Sleep(frame_timing * 2);

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
		RenderCurrentCheckersOnDesk(hdc);
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