#include <windows.h>

#define STEP (30)
#define INDENT_X (50)
#define INDENT_Y (70)
#define ID_TIMER (1)
#define TIME_FOR_TIMER (10)
#define STEP_WITH_TIMER (1)

enum direction { TOP, RIGHT, BOTTOM, LEFT };

int _x1 = 10, _y1 = 10, _x2 = 123, _y2 = 124;
int _xStart, _yStart; //for moving
HBITMAP _bitmap;
int _inOneSide;
direction _direction;
int _tempStep;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, //адрес начала кода программы в ее адресном пространстве.
				HINSTANCE hPrevInstance, 
				LPSTR lpCmdLine, //для запуска окна в режиме командной строки
				int nCmdShow) // режим отображения окна
{
	TCHAR className[] = L"Osisp";
	MSG msg;
	HWND hWnd;
	WNDCLASSEX wc;

	wc.cbSize = sizeof(wc);
	wc.style = NULL;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = NULL;
	wc.cbWndExtra = NULL;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, NULL);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;//resouce_menu_name
	wc.lpszClassName = className;
	wc.hIconSm = LoadIcon(NULL, NULL);

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"RegisterClass error!", L"Error", MB_OK);
		return NULL;
	}
		
	hWnd = CreateWindow(wc.lpszClassName, className, WS_OVERLAPPEDWINDOW,
		200, 200, 400, 400, NULL, (HMENU)NULL, hInstance, NULL);
	if (!hWnd)
	{
		MessageBox(NULL, L"CreateWindow error!", L"Error", MB_OK);
		return NULL;
	}

	ShowWindow(hWnd, SW_SHOW);
	if (!UpdateWindow(hWnd)) 
	{
		MessageBox(NULL, L"UpdateWindow error!", L"Error", MB_OK);
		return NULL;
	}

	while (GetMessage(&msg, NULL, NULL, NULL)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam; 
}

void OnPaint(HDC hDc, int x1, int y1, int x2, int y2, HBRUSH brush)
{
	SelectObject(hDc, brush);
	Rectangle(hDc, x1, y1, x2, y2);
}

void RefreshFigure(int *k1, int *k2, int step, HWND hWnd, RECT rect) 
{
	*k1 += step;
	*k2 += step;
	InvalidateRect(hWnd, &rect, TRUE);
}

void Clear(HDC hDc, RECT rect)
{
	HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
	OnPaint(hDc, 0, 0, rect.right, rect.bottom, brush);
	DeleteObject(brush);
}

bool IsMovingInBorder(RECT rect, direction direction) 
{
	switch (direction)
	{
		case TOP:

			if (_y1 - STEP < rect.top) return true;
			break;

		case RIGHT:

			if (_x2 + STEP > rect.right) return true;
			break;

		case BOTTOM:

			if (_y2 + STEP > rect.bottom) return true;
			break;

		case LEFT:
			if (_x1 - STEP < rect.left) return true;
	}
	return false;
}

void Drawing(HDC hDc)
{
	HBRUSH brush = CreateSolidBrush(RGB(255, 0, 0));
	OnPaint(hDc, _x1, _y1, _x2, _y2, brush);
	DeleteObject(brush);
}

void GetCursorCoordinates(HWND hWnd, LPARAM lParam, int *x, int *y)
{
	HDC hDC;
	hDC = GetDC(hWnd);
	*x = LOWORD(lParam); 
	*y = HIWORD(lParam);
	ReleaseDC(hWnd, hDC);
}

bool IsCursorOnFigure(HWND hWnd, LPARAM lParam) {
	int x, y;
	GetCursorCoordinates(hWnd, lParam, &x, &y);
	if (x >= _x1 && x <= _x2  && y >= _y1 && y <= _y2) 
	{
		_xStart = x;
		_yStart = y;
		return true;
	}
	return false;
}

void MovingWithTimer(HWND hWnd, direction direction, int inOneSide)
{
	_direction = direction;
	_inOneSide = inOneSide;
	_tempStep = STEP;
	SetTimer(hWnd, ID_TIMER, TIME_FOR_TIMER, NULL);
}

int GetStepCountToBorder(RECT rect, direction direction)
{
	switch (direction)
	{
		case BOTTOM:
			return rect.bottom - _y2;

		case TOP:
			return _y1 - rect.top;

		case LEFT:
			return _x1 - rect.left;

		case RIGHT:
			return rect.right - _x2;
	}
}

int GetStep(direction direction)
{
	switch (direction) 
	{
		case BOTTOM:
		case RIGHT:
			return STEP;
		default:
			return -STEP;
	}
}

void Moving(HWND hWnd, RECT rect, int *k1, int *k2, direction direction)
{
	if (IsMovingInBorder(rect, direction)) 
	{
		int stepCountToBorder = GetStepCountToBorder(rect, direction);
		MovingWithTimer(hWnd, direction, stepCountToBorder);
	}
	else
		RefreshFigure(k1, k2, GetStep(direction), hWnd, rect);
}

void DoMove(HWND hWnd, RECT rect, int *k1, int *k2, int step)
{
	if (_inOneSide > 0)
	{
		RefreshFigure(k1, k2, step, hWnd, rect);
	}
	else
	{
		RefreshFigure(k1, k2, -step, hWnd, rect);
	}
}

void DrawPicture(HDC hDc)
{
	BITMAP bitmap;
	HDC	hDcMem = CreateCompatibleDC(hDc);
	HGDIOBJ oldBmp = SelectObject(hDcMem, _bitmap);
	GetObject(_bitmap, sizeof(bitmap), &bitmap);
	BitBlt(hDc, _x1, _y1, _x2, _y2, hDcMem, 0, 0, SRCCOPY);
	SelectObject(hDcMem, oldBmp);
	DeleteDC(hDcMem);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static PAINTSTRUCT	ps;
	static HDC hDc;
	static RECT rect;
	static bool isPressed = false;
	static LPMINMAXINFO lpMMI;

	switch (msg)
	{	 
		case WM_TIMER:

			switch (_direction)
			{
				case TOP:

					DoMove(hWnd, rect, &_y1, &_y2, -STEP_WITH_TIMER);
					break;

				case RIGHT:

					DoMove(hWnd, rect, &_x1, &_x2, STEP_WITH_TIMER);
					break;

				case BOTTOM:

					DoMove(hWnd, rect, &_y1, &_y2, STEP_WITH_TIMER);
					break;

				case LEFT:

					DoMove(hWnd, rect, &_x1, &_x2, -STEP_WITH_TIMER);
					break;
			}

			_tempStep--;
			_inOneSide--;
			if (_tempStep == 0)
				KillTimer(hWnd, ID_TIMER);
			break;

		case WM_SIZE:
			
			InvalidateRect(hWnd, &rect, TRUE);
			break;

		case WM_CREATE:

			_bitmap = (HBITMAP)LoadImage(NULL, L"nike6.0.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			if (_bitmap == NULL)
			{
				MessageBox(NULL, L"LoadBitmap error!", L"Error", MB_OK);
				return NULL;
			}
			break;

		case WM_GETMINMAXINFO:

			lpMMI = (LPMINMAXINFO)lParam;
			lpMMI->ptMinTrackSize.x = _x2 + INDENT_X;
			lpMMI->ptMinTrackSize.y = _y2 + INDENT_Y ;
			break;

		case WM_PAINT: 
		{
			hDc = BeginPaint(hWnd, &ps);
			GetClientRect(hWnd, &rect);
			Clear(hDc, rect);
			//Drawing(hDc);

			DrawPicture(hDc);

			EndPaint(hWnd, &ps);
			break;
		}

		case WM_KEYDOWN:

			switch (wParam)
			{
				case VK_DOWN:

					Moving(hWnd, rect, &_y1, &_y2, BOTTOM);
					break;

				case VK_UP:

					Moving(hWnd, rect, &_y1, &_y2, TOP);
					break;

				case VK_LEFT:

					Moving(hWnd, rect, &_x1, &_x2, LEFT);
					break;

				case VK_RIGHT:

					Moving(hWnd, rect, &_x1, &_x2, RIGHT);
					break;

				default:
					break;
			}
			break;
		
		case WM_MOUSEWHEEL:
		{
			short x = GET_WHEEL_DELTA_WPARAM(wParam);
			if (GetKeyState(VK_SHIFT) & 8000) // shift pressed // GET_KEYSTATE_WPARAM(wParam)
			{
				if (x > 0)
					Moving(hWnd, rect, &_x1, &_x2, RIGHT);
				else
					Moving(hWnd, rect, &_x1, &_x2, LEFT);

			}
			else
			{
				if (x > 0)
					Moving(hWnd, rect, &_y1, &_y2, TOP);
				else
					Moving(hWnd, rect, &_y1, &_y2, BOTTOM);
			}
			break;
		}

		case WM_LBUTTONDOWN:

			if (IsCursorOnFigure(hWnd, lParam))
				isPressed = true;
			break;

		case WM_MOUSEMOVE:

			if (isPressed) {

				int xCurr, yCurr;
				GetCursorCoordinates(hWnd, lParam, &xCurr, &yCurr);

				int dx = xCurr - _xStart;
				int dy = yCurr - _yStart;

				_x1 += dx;
				_x2 += dx;
				_y1 += dy;
				_y2 += dy;

				_xStart = xCurr;
				_yStart = yCurr;

				InvalidateRect(hWnd, &rect, TRUE);
			}
			break;

		case WM_LBUTTONUP:

			isPressed = false;
			break;

		case WM_DESTROY:

			DeleteObject(_bitmap);
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}
	