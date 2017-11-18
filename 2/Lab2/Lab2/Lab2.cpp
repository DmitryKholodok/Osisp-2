#include <windows.h>
#include <ctime>
#include <fstream>
#include <cmath>
#include <string>

#define ROW_COUNT (3)
#define COLUMN_COUNT (4)
#define	CHAR_WIDTH (20)
#define	CHAR_HEIGHT	(20)
#define INDENT (5)
#define K (-0.2)
#define	MIN_X_WINDOW_SIZE (CHAR_WIDTH * COLUMN_COUNT * 2)
#define	MIN_Y_WINDOW_SIZE (CHAR_HEIGHT * ROW_COUNT * 2)

int wheelY = 0;		//	last Y scroll
int currRowY = 0;	//	last table	Y
std::string text[ROW_COUNT][COLUMN_COUNT];

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) 
{
	WNDCLASSEX wc; HWND hWnd; MSG msg;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_DBLCLKS | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 10);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = L"TableClass";
	wc.hIconSm = wc.hIcon;

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"RegisterClass error!", L"Error", MB_OK);
		return NULL;
	}

	hWnd = CreateWindow(L"TableClass", L"Table", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, 
		NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

bool ScrollingCheckSaysUpdateTable(HWND hWnd, WPARAM wParam) 
{
	RECT clientRect;
	GetClientRect(hWnd, &clientRect);

	short currWheel = GET_WHEEL_DELTA_WPARAM(wParam);

	if (currRowY < clientRect.bottom)
		return false;

	if (clientRect.bottom + wheelY >= currRowY && currWheel < 0)
		return false;

	if (wheelY <= 0 && currWheel > 0)
		return false;


	if (wheelY + K * currWheel <= 0) 
	{
		wheelY = 0;
		return true;
	}

	if (wheelY + K * currWheel + clientRect.bottom >= currRowY) 
	{
		wheelY = currRowY - clientRect.bottom;
		return true;
	}

	wheelY += K * currWheel;
	return true;

}

int findCharCountToWrite(HDC hDc, int columnDist, std::wstring text) 
{
	SIZE stringSize;
	int charCount = (columnDist - 2 * INDENT) / CHAR_WIDTH;
	std::wstring testString = text.substr(0, charCount);
	GetTextExtentPoint32(hDc, testString.c_str(), testString.length(), &stringSize);
	
	while (stringSize.cx < columnDist - 2 * INDENT && charCount < text.length())
	{
		charCount++;
		testString = text.substr(0, charCount);
		GetTextExtentPoint32(hDc, testString.c_str(), testString.length(), &stringSize);
	}

	while (stringSize.cx > columnDist - 2 * INDENT) 
	{
		charCount--;
		testString = text.substr(0, charCount);
		GetTextExtentPoint32(hDc, testString.c_str(), testString.length(), &stringSize);
	}
	
	if (charCount == 0) return 1;
	return charCount;
}

void RowWriting(HWND hWnd, HDC hDc, RECT clientRect, int row, int columnDist)
{	
	int maxY = currRowY;	//	need for a line

	for (int j = 0; j < COLUMN_COUNT; j++) 
	{
		SIZE textSize;
		std::string st = text[row][j];
		std::wstring rowFromText(st.begin(), st.end());
		GetTextExtentPoint32(hDc, rowFromText.c_str(), rowFromText.length(), &textSize);

		// the row fits into one line
		if (textSize.cx <= columnDist - 2 * INDENT) 
		{
			TextOut(hDc, columnDist * j + INDENT, currRowY - wheelY + INDENT, 
				rowFromText.c_str(), rowFromText.length());

			if (currRowY + textSize.cy > maxY) 
				maxY = currRowY + textSize.cy;
		}

		//	more than one line
		else 
		{
			SIZE stringToWriteSize;
			std::wstring stringToWrite = rowFromText,	
			ostString;
			
			int ostWidth, 
			textWidth = textSize.cx;
			
			int currY = currRowY + INDENT;
			
			int textLen = rowFromText.length();
			
			do 
			{
				int charCountToWrite = findCharCountToWrite(hDc, columnDist, rowFromText);				
				stringToWrite = rowFromText.substr(0, charCountToWrite);
				
				if (textLen >= charCountToWrite + 1) 
				{
					ostString = rowFromText.substr(charCountToWrite, textLen);
					GetTextExtentPoint32(hDc, stringToWrite.c_str(), stringToWrite.length(), &stringToWriteSize);
					ostWidth = textWidth - stringToWriteSize.cx;

					TextOut(hDc, columnDist * j + INDENT, currY - wheelY, stringToWrite.c_str(), stringToWrite.length());
					
					currY += stringToWriteSize.cy + INDENT;
					rowFromText = ostString;
					textLen = rowFromText.length();
					textWidth = ostWidth;
				}							
			} 
			while (ostWidth > columnDist - 2 * INDENT);

			if (ostWidth != 0)
				TextOut(hDc, columnDist * j + INDENT, currY - wheelY, rowFromText.c_str(), rowFromText.length());

			if (currY + stringToWriteSize.cy > maxY) {
				maxY = currY + stringToWriteSize.cy;
			}
		}
	}

	currRowY = maxY + INDENT;

	// horizontal
	MoveToEx(hDc, clientRect.left, currRowY - wheelY, NULL);
	LineTo(hDc, clientRect.right, currRowY - wheelY);
}

void TextWriting(HWND hWnd, HDC hDc, RECT clientRect, int columnDist) {
	for (int i = 0; i < ROW_COUNT; i += 1) {
		RowWriting(hWnd, hDc, clientRect, i, columnDist);
	}
}

void DrawTable(HWND hWnd) 
{
	PAINTSTRUCT ps;
	HDC hDc = BeginPaint(hWnd, &ps);

	HPEN pen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	HGDIOBJ objToReturn1 = SelectObject(hDc, pen);

	HFONT font = CreateFont(CHAR_HEIGHT, CHAR_WIDTH, 0, 0, FW_NORMAL, false, false, false, 0, 0, 0, 0, 0, NULL);
	HGDIOBJ objToReturn2 = SelectObject(hDc, font);

	RECT clientRect;
	GetClientRect(hWnd, &clientRect);

	int columnDist = (float)clientRect.right / COLUMN_COUNT;
	float currX = 0;

	TextWriting(hWnd, hDc, clientRect, columnDist);

	// drawing the vertical lines
	for (int i = 0; i < COLUMN_COUNT - 1; i++) 
	{
		currX += columnDist;
		MoveToEx(hDc, currX, 0, NULL);
		LineTo(hDc, round(currX), currRowY);
	}

	// disposing
	SelectObject(hDc, objToReturn1);
	SelectObject(hDc, objToReturn2);

	EndPaint(hWnd, &ps);
}

void LoadTextFromFile() {

	std::ifstream fin("text.txt");

	for (int i = 0; i < ROW_COUNT; i++) 
	{
		for (int j = 0; j < COLUMN_COUNT; j++) 
		{
			std::getline(fin, text[i][j]);
		}
	}

	fin.close();
}

void SetWindowMinParam(LPARAM lParam)
{
	LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
	lpMMI->ptMinTrackSize.x = MIN_X_WINDOW_SIZE;
	lpMMI->ptMinTrackSize.y = MIN_Y_WINDOW_SIZE;
}

void UpdateTable(HWND hWnd)
{
	currRowY = 0;
	InvalidateRect(hWnd, NULL, true);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_GETMINMAXINFO:

			SetWindowMinParam(lParam);
			break;

		case WM_CREATE:

			LoadTextFromFile();
			break;

		case WM_PAINT:

			DrawTable(hWnd);
			break;

		case WM_MOUSEWHEEL: 
			
			if (ScrollingCheckSaysUpdateTable(hWnd, wParam)) 
				UpdateTable(hWnd);
			break;

		case WM_MOVE:
		case WM_SIZE: 

			if (wParam != SIZE_MINIMIZED) 
			{
				wheelY = 0;
				UpdateTable(hWnd);
			}
			break;
		
		case WM_DESTROY:

			PostQuitMessage(0);
			break;

		default:

			return DefWindowProc(hWnd, message, wParam, lParam);

	}
}