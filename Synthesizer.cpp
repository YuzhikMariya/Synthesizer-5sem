#include <windows.h>
#include "resource.h"
#include "Definitions.h"
#pragma comment(lib, "Winmm.lib")

HBRUSH whiteBrushArray[] = { CreateSolidBrush(RGB(255, 255, 255)), CreateSolidBrush(RGB(232, 179, 179)), CreateSolidBrush(RGB(230, 230, 250)), CreateSolidBrush(RGB(130, 232, 205)) };
HBRUSH blackBrushArray[] = { CreateSolidBrush(RGB(0, 0, 0)), CreateSolidBrush(RGB(51, 0, 0)), CreateSolidBrush(RGB(0, 20, 43)), CreateSolidBrush(RGB(0, 36, 26)) };

int keyArray[37], Octave = 0, deltaVolume = 0x22222222, instrument = 0x0000, 
	whiteBrushIndex = 0, blackBrushIndex = 0, instrumentIndex = 0;
HMIDIOUT hmidi;
DWORD midimsg;
bool isPedal = false, isSign = false;
static HBITMAP sprite = NULL;
LOGFONT lf;
HFONT hFont;
static HWND hcbSign, hcbPedal;

typedef struct
{
	HBRUSH hbrWhite, hbrBlack, hbrGray;
	bool isWhitePress[19];
	bool isBlackPress[13];
	RECT whiteRect[19];
	RECT blackRect[13];
} KeyStruct, *PKeyStruct;

PKeyStruct pKey = {};


SIZE GetBitmapSize(HBITMAP hBitmap)
{
	BITMAP bitmap;
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);
	SIZE result;
	result.cx = bitmap.bmWidth;
	result.cy = bitmap.bmHeight;
	return result;
}

void FillKeyArrays() 
{
	for (int i = 0; i < 17; i++)
		keyArray[i] = i;
	for (int i = 17; i < 22; i++)
		keyArray[i] = i-5;
	for (int i = 22; i < 37; i++)
		keyArray[i] = i-5;
}

bool isWhite(int index)
{
	if ((index == 1) || (index == 3) || (index == 6) || (index == 8) || (index == 10) || (index == 13) ||
		(index == 15) || (index == 18) || (index == 20) || (index == 22) || (index == 25) || (index == 27) || (index == 30))
		return false;
	else
		return true;
}

HWND SetButton(HWND hWnd, HINSTANCE hInstance, int path, int id, int x, int yCenter, bool isLeft) 
{
	HWND hBtn;
	HBITMAP hBitMap;
	SIZE sz;
	int xPos;

	hBitMap = (HBITMAP)LoadImage(hInstance, MAKEINTRESOURCE(path), IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION | LR_COPYFROMRESOURCE);
	sz = GetBitmapSize(hBitMap);
	sz.cx += 6;
	sz.cy += 6;
	if (isLeft)
		xPos = x - sz.cx - 20;
	else
		xPos = x + 20;
	hBtn = CreateWindow("button", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | BS_BITMAP,
		xPos, yCenter - sz.cy/2, sz.cx, sz.cy,
		hWnd, (HMENU)id, hInstance, NULL);
	SendMessage(hBtn, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitMap);
	ShowWindow(hBtn, SW_SHOWNORMAL);
	return hBtn;
}

void SetInstrument(HWND hWnd, const char* path, int instr) {
	instrument = instr;
	sprite = (HBITMAP)LoadImage(NULL, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	InvalidateRect(hWnd, NULL, false);
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int VK = wParam;     
	DWORD midimsg;

	PAINTSTRUCT ps;
	RECT clientRect;
	HDC hdc, hCmpDC, spriteDC;
	HBITMAP hBmp;
	SIZE bitmapSize;
	LRESULT res;
	


	int blackCount, tempInd, x, y, index;

	BOOL Rpt = (lParam & 0x40000000); 
	BYTE Speed;

	int pos = 0;
	bool isContinue = true;
	
	DWORD vol;

	
	switch (message)
	{
		case WM_KEYDOWN:
			
			Speed = 127;
			while ((pos < 37) && (isContinue))
			{
				if (keySimbols[pos] == VK)
					isContinue = false;
				else
					pos++;
			}
			if (!isContinue) 
			{
				if (!Rpt) {   
					midiOutShortMsg(hmidi, 0x00000000 + instrument + 0xC0);
					midimsg = 0x90 + (60 + keyArray[pos] + Octave*12) * 0x100 + Speed * 0x10000;
					midiOutShortMsg(hmidi, midimsg);
					if (isWhite(keyArray[pos]))
						pKey->isWhitePress[tranformFromAllKeysToWB[keyArray[pos]]] = true;
					else
						pKey->isBlackPress[tranformFromAllKeysToWB[keyArray[pos]]] = true;
					InvalidateRect(hWnd, NULL, false);
				}	
			}

			if ((wParam > 112) && (wParam < 119))
			{
				instrumentIndex = wParam - 113;
				SetInstrument(hWnd, instrumentPicArray[instrumentIndex], instrumentArray[instrumentIndex]);
				break;
			}

			switch (wParam)
			{
				case VK_F8:
					res = SendMessage(hcbSign, BM_GETCHECK, FALSE, FALSE);
					if (res == BST_CHECKED)
					{
						SendMessage(hcbSign, BM_SETCHECK, BST_UNCHECKED, NULL);
					}
					else if (res == BST_UNCHECKED)
					{
						SendMessage(hcbSign, BM_SETCHECK, BST_CHECKED, NULL);
					}
					SendMessage(hWnd, WM_COMMAND, ID_BTN_SIGN, NULL);
					break;
				case VK_UP:
					waveOutGetVolume(0, &vol);
					if (vol < 0xeeeeeeee)
					{
						vol = vol + deltaVolume;
						waveOutSetVolume(0, vol);
					}
					break;
				case VK_DOWN:
					waveOutGetVolume(0, &vol);
					if (vol > 0)
					{
						vol = vol - deltaVolume;
						waveOutSetVolume(0, vol);
					}
					break;
				case VK_LEFT:
					if (Octave > -3)
					{
						Octave--;
						InvalidateRect(hWnd, NULL, false);
					}	
					break;
				case VK_RIGHT:
					if (Octave < 2)
					{
						Octave++;
						InvalidateRect(hWnd, NULL, false);
					}				
					break;
				case VK_SPACE:
					res = SendMessage(hcbPedal, BM_GETCHECK, FALSE, FALSE);
					if (res == BST_CHECKED)
					{
						SendMessage(hcbPedal, BM_SETCHECK, BST_UNCHECKED, NULL);
					}
					else if (res == BST_UNCHECKED)
					{
						SendMessage(hcbPedal, BM_SETCHECK, BST_CHECKED, NULL);
					}
					SendMessage(hWnd, WM_COMMAND, ID_BTN_PEDAL, NULL);
					break;
			}
			break;
		case WM_KEYUP:			
				Speed = 0;
				while ((pos < 37) && (isContinue))
				{
					if (keySimbols[pos] == VK)
						isContinue = false;
					else
						pos++;
				}
				if (!isContinue) {
					if (!isPedal) {
						midimsg = 0x90 + (60 + keyArray[pos] + Octave * 12) * 0x100 + Speed * 0x10000;
						midiOutShortMsg(hmidi, midimsg);
					}
					if (isWhite(keyArray[pos]))
						pKey->isWhitePress[tranformFromAllKeysToWB[keyArray[pos]]] = false;
					else
						pKey->isBlackPress[tranformFromAllKeysToWB[keyArray[pos]]] = false;
					InvalidateRect(hWnd, NULL, false);
				}
				break;
		case WM_LBUTTONDOWN:
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			if (((x - KEY_LEFT) < 19*WHITE_WIDTH) && ((x - KEY_LEFT) > 0) && ((y - KEY_TOP) < WHITE_HEIGHT) && (y - KEY_TOP > 0))
			{
				index = (x - KEY_LEFT) / WHITE_WIDTH;
				isContinue = true;
				tempInd = 0;
				if ((y - KEY_TOP) <= BLACK_HEIGHT)
				{
					while(isContinue && (tempInd < 13))
					{
						if ((x >= pKey->blackRect[tempInd].left) && (x <= pKey->blackRect[tempInd].right))
						{
							isContinue = false;
						}
						tempInd++;
					}
					if (!isContinue)
					{
						pKey->isBlackPress[tempInd - 1] = true;
						midiOutShortMsg(hmidi, 0x00000000 + instrument + 0xC0);
						midimsg = 0x90 + (60 + tranformFromBlackToAll[tempInd-1] + Octave * 12) * 0x100 + 127 * 0x10000;
						midiOutShortMsg(hmidi, midimsg);
					}
					else
					{
						pKey->isWhitePress[index] = true;
						midiOutShortMsg(hmidi, 0x00000000 + instrument + 0xC0);
						midimsg = 0x90 + (60 + tranformFromWhiteToAll[index] + Octave * 12) * 0x100 + 127 * 0x10000;
						midiOutShortMsg(hmidi, midimsg);
					}
				}
				else
				{
					pKey->isWhitePress[index] = true;
					midiOutShortMsg(hmidi, 0x00000000 + instrument + 0xC0);
					midimsg = 0x90 + (60 + tranformFromWhiteToAll[index] + Octave * 12) * 0x100 + 127 * 0x10000;
					midiOutShortMsg(hmidi, midimsg);
				}
				InvalidateRect(hWnd, NULL, false);
			}	
			break;
		case WM_LBUTTONUP:
			x = LOWORD(lParam);
			y = HIWORD(lParam);
			if (((x - KEY_LEFT) < 19 * WHITE_WIDTH) && ((x - KEY_LEFT) > 0) &&  ((y - KEY_TOP) < WHITE_HEIGHT) && (y - KEY_TOP > 0))
			{
				index = (x - KEY_LEFT) / WHITE_WIDTH;
				isContinue = true;
				tempInd = 0;
				if ((y - KEY_TOP) <= BLACK_HEIGHT)
				{
					while (isContinue && (tempInd < 13))
					{
						if ((x >= pKey->blackRect[tempInd].left) && (x <= pKey->blackRect[tempInd].right))
						{
							isContinue = false;
						}
						tempInd++;
					}
					if (!isContinue)
					{
						pKey->isBlackPress[tempInd - 1] = false;
						midimsg = 0x90 + (60 + tranformFromBlackToAll[tempInd-1] + Octave * 12) * 0x100 + 0 * 0x10000;
						midiOutShortMsg(hmidi, midimsg);
					}
					else
					{
						pKey->isWhitePress[index] = false;
						midimsg = 0x90 + (60 + tranformFromWhiteToAll[index] + Octave * 12) * 0x100 + 0 * 0x10000;
						midiOutShortMsg(hmidi, midimsg);
					}
				}
				else
				{
					pKey->isWhitePress[index] = false;
					midimsg = 0x90 + (60 + tranformFromWhiteToAll[index] + Octave * 12) * 0x100 + 0 * 0x10000;
					midiOutShortMsg(hmidi, midimsg);
				}
				InvalidateRect(hWnd, NULL, false);
			}


			break;
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case ID_BTN_DOWN_COLOR1:
					blackBrushIndex = (blackBrushIndex - 1 + 4) % 4;
					pKey->hbrBlack = blackBrushArray[blackBrushIndex];
					InvalidateRect(hWnd, NULL, false);
					SetFocus(hWnd);
					break;
				case ID_BTN_UP_COLOR1:
					blackBrushIndex = (blackBrushIndex + 1) % 4;
					pKey->hbrBlack = blackBrushArray[blackBrushIndex];
					InvalidateRect(hWnd, NULL, false);
					SetFocus(hWnd);
					break;
				case ID_BTN_DOWN_COLOR2:
					whiteBrushIndex = (whiteBrushIndex - 1 + 4) % 4;
					pKey->hbrWhite = whiteBrushArray[whiteBrushIndex];
					InvalidateRect(hWnd, NULL, false);
					SetFocus(hWnd);
					break;
				case ID_BTN_UP_COLOR2:
					whiteBrushIndex = (whiteBrushIndex + 1) % 4;
					pKey->hbrWhite = whiteBrushArray[whiteBrushIndex];
					InvalidateRect(hWnd, NULL, false);
					SetFocus(hWnd);
					break;
				case ID_BTN_DOWN_OCTAVE:
					keybd_event(VK_LEFT, 0, 0, 0);
					SetFocus(hWnd);
					break;
				case ID_BTN_UP_OCTAVE:
					keybd_event(VK_RIGHT, 0, 0, 0);
					SetFocus(hWnd);
					break;
				case ID_BTN_DOWN_INSTRUMENT:
					instrumentIndex = (instrumentIndex - 1 + 6) % 6;
					SetInstrument(hWnd, instrumentPicArray[instrumentIndex], instrumentArray[instrumentIndex]);
					SetFocus(hWnd);
					break;
				case ID_BTN_UP_INSTRUMENT:
					instrumentIndex = (instrumentIndex + 1) % 6;
					SetInstrument(hWnd, instrumentPicArray[instrumentIndex], instrumentArray[instrumentIndex]);
					SetFocus(hWnd);
					break;
				case ID_BTN_SIGN:
					isSign = !isSign;
					InvalidateRect(hWnd, NULL, false);
					SetFocus(hWnd);
					break;
				case ID_BTN_PEDAL:
					isPedal = !isPedal;
					SetFocus(hWnd);
					break;
			}
			break;
		case WM_CREATE:
			pKey = (PKeyStruct)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(KeyStruct));
			SetWindowLong(hWnd, 0, (LONG)pKey);

			ZeroMemory(&lf, sizeof(LOGFONT));
			lf.lfHeight = 30;
			strcpy_s(lf.lfFaceName, "Monotype Corsiva");
			hFont = CreateFontIndirect(&lf);

			pKey->hbrBlack = blackBrushArray[blackBrushIndex];
			pKey->hbrWhite = whiteBrushArray[whiteBrushIndex];
			pKey->hbrGray = CreateSolidBrush(RGB(255, 0, 0));
			for (int i = 0; i < 19; i++)
			{
				pKey->whiteRect[i].top = KEY_TOP;
				pKey->whiteRect[i].bottom = KEY_TOP + WHITE_HEIGHT;
				pKey->whiteRect[i].left = KEY_LEFT + WHITE_WIDTH*i;
				pKey->whiteRect[i].right = pKey->whiteRect[i].left + WHITE_WIDTH;
			}
			blackCount = 0;
			for (int i = 0; i < 18; i++)
			{
				if ((i == 2) || (i == 6) || (i == 9) || (i == 13) || (i == 16))
					continue;
				pKey->blackRect[blackCount].top = KEY_TOP;
				pKey->blackRect[blackCount].bottom = KEY_TOP + BLACK_HEIGHT;
				pKey->blackRect[blackCount].left = KEY_LEFT + WHITE_WIDTH*(i+1) - BLACK_WIDTH/2;
				pKey->blackRect[blackCount].right = pKey->blackRect[blackCount].left + BLACK_WIDTH;
				blackCount++;
			}
			sprite = (HBITMAP)LoadImage(NULL, instrumentPicArray[instrumentIndex], IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

			break;
		case WM_CTLCOLORSTATIC:
			if (((HWND)lParam == hcbSign) || ((HWND)lParam == hcbPedal))
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (LRESULT)GetStockObject(NULL_BRUSH);
			}
			break;
		case WM_PAINT:
			GetClientRect(hWnd, &clientRect);
			hdc = BeginPaint(hWnd, &ps);

			spriteDC = CreateCompatibleDC(hdc);
			SelectObject(spriteDC, sprite);
			bitmapSize = GetBitmapSize(sprite);


			hCmpDC = CreateCompatibleDC(hdc);
			hBmp = CreateCompatibleBitmap(hdc, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);
			SelectObject(hCmpDC, hBmp);

			FillRect(hCmpDC, &clientRect, (HBRUSH)RGB(255, 255, 255));	

			//Отобразить инструмент на буффер
			BitBlt(hCmpDC, INSTRUMENT_LEFT, INSTRUMENT_TOP, bitmapSize.cx, bitmapSize.cy, spriteDC, 0, 0, SRCCOPY);

			//Надпись Instrument
			SelectObject(hCmpDC, hFont);
			TextOut(hCmpDC, (int)(INSTRUMENT_LEFT + 0.5*GetBitmapSize(sprite).cx - 6 * 0.3*lf.lfHeight), 0, "Instrument", 10);

			//надпись Color
			TextOut(hCmpDC, (int)(COLOR_LEFT + 0.5*COLOR_WIDTH - 3*0.3*lf.lfHeight), 0, "Color", 5);


			//Рисуем прямоугольнички с цветом
			SelectObject(hCmpDC, pKey->hbrBlack);
			Rectangle(hCmpDC, COLOR_LEFT, COLOR_TOP, COLOR_LEFT+COLOR_WIDTH, COLOR_TOP+COLOR_HEIGHT);
			SelectObject(hCmpDC, pKey->hbrWhite);
			Rectangle(hCmpDC, COLOR_LEFT, COLOR_TOP + COLOR_HEIGHT + 20, COLOR_LEFT + COLOR_WIDTH, COLOR_TOP + COLOR_HEIGHT + 20 + COLOR_HEIGHT);

				//Рисуем белые клавиши
			for (int i = 0; i < 19; i++)
				{
					if (pKey->isWhitePress[i])
						SelectObject(hCmpDC, pKey->hbrGray);
					else
						SelectObject(hCmpDC, pKey->hbrWhite);
					Rectangle(hCmpDC, pKey->whiteRect[i].left, pKey->whiteRect[i].top, pKey->whiteRect[i].right, pKey->whiteRect[i].bottom);
				}

				//Рисуем черные клавиши
			for (int i = 0; i < 13; i++)
				{
					if (pKey->isBlackPress[i])
						SelectObject(hCmpDC, pKey->hbrGray);
					else
						SelectObject(hCmpDC, pKey->hbrBlack);
					Rectangle(hCmpDC, pKey->blackRect[i].left, pKey->blackRect[i].top, pKey->blackRect[i].right, pKey->blackRect[i].bottom);
				}
			
			//Подписываем клавиши
			if (isSign)
			{
				SelectObject(hCmpDC, hFont);
				for (int i = 0; i < 13; i++)
				{
					SetTextColor(hCmpDC, RGB(255, 255, 255));
					SetBkMode(hCmpDC, TRANSPARENT);
					DrawText(hCmpDC, blackSimbols[i], 1, &pKey->blackRect[i], DT_BOTTOM | DT_SINGLELINE | DT_CENTER);
				}
				for(int i = 0; i < 19; i++)
				{
					SetTextColor(hCmpDC, RGB(0, 0, 0));
					SetBkMode(hCmpDC, TRANSPARENT);
					DrawText(hCmpDC, whiteSimbols[i], 1, &pKey->whiteRect[i], DT_BOTTOM | DT_SINGLELINE | DT_CENTER);
				}
				
					
			}

			//Подписываем октаву
			SelectObject(hCmpDC, hFont);
			TextOut(hCmpDC, KEY_LEFT, KEY_TOP + WHITE_HEIGHT, octaves[Octave+3], 15);

			//Отобразить буффер на окно
			SetStretchBltMode(hdc, COLORONCOLOR);
			BitBlt(hdc, 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, hCmpDC, 0, 0, SRCCOPY);

			//DeleteObject(hFont);
			DeleteDC(hCmpDC);
			DeleteDC(spriteDC);
			DeleteObject(hBmp);
			hCmpDC = NULL;
			spriteDC = NULL;
			EndPaint(hWnd, &ps);

			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex; HWND hWnd; MSG msg;
	static HWND hbtnDownOctave, hbtnUpOctave, hbtnDownInstrument, hbtnUpInstrument, 
		hbtnDownColor1, hbtnUpColor1, hbtnDownColor2, hbtnUpColor2;
	LOGFONT logf;
	HFONT hf;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = NULL;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON3));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = "SinthesizerClass";
	wcex.hIconSm = wcex.hIcon;
	RegisterClassEx(&wcex);


	hWnd = CreateWindow("SinthesizerClass", "Piano Master", WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU, 100, 200, 2*KEY_LEFT + 19*WHITE_WIDTH + 15, KEY_TOP + WHITE_HEIGHT + 80, NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	
	hbtnDownOctave = SetButton(hWnd, hInstance, IDB_DOWN_OCTAVE, ID_BTN_DOWN_OCTAVE,
		KEY_LEFT, KEY_TOP + WHITE_HEIGHT/2, true);
	hbtnUpOctave = SetButton(hWnd, hInstance, IDB_UP_OCTAVE, ID_BTN_UP_OCTAVE,
		KEY_LEFT + 19*WHITE_WIDTH, KEY_TOP + WHITE_HEIGHT / 2, false);
	
	hbtnDownOctave = SetButton(hWnd, hInstance, IDB_DOWN_INSTRUMENT, ID_BTN_DOWN_INSTRUMENT,
		INSTRUMENT_LEFT, INSTRUMENT_TOP + GetBitmapSize(sprite).cy / 2, true);
	hbtnDownOctave = SetButton(hWnd, hInstance, IDB_UP_INSTRUMENT, ID_BTN_UP_INSTRUMENT,
		INSTRUMENT_LEFT + GetBitmapSize(sprite).cx, INSTRUMENT_TOP + GetBitmapSize(sprite).cy / 2, false);
	
	hbtnDownColor1 = SetButton(hWnd, hInstance, IDB_DOWN_COLOR, ID_BTN_DOWN_COLOR1,
		COLOR_LEFT, COLOR_TOP + COLOR_HEIGHT / 2, true);
	hbtnUpColor1 = SetButton(hWnd, hInstance, IDB_UP_COLOR, ID_BTN_UP_COLOR1,
		COLOR_LEFT + COLOR_WIDTH, COLOR_TOP + COLOR_HEIGHT / 2, false);
	
	hbtnDownColor2 = SetButton(hWnd, hInstance, IDB_DOWN_COLOR, ID_BTN_DOWN_COLOR2,
		COLOR_LEFT, (int)(COLOR_TOP + 20 + 1.5 * COLOR_HEIGHT), true);
	hbtnUpColor2 = SetButton(hWnd, hInstance, IDB_UP_COLOR, ID_BTN_UP_COLOR2,
		COLOR_LEFT + COLOR_WIDTH, (int)(COLOR_TOP + 20 + 1.5 * COLOR_HEIGHT), false);

	hcbSign = CreateWindow("button", "Sign keys",
		WS_CHILD | WS_VISIBLE| BS_CHECKBOX | BS_AUTOCHECKBOX,
		20, 20, 110, 30,
		hWnd, (HMENU)ID_BTN_SIGN, hInstance, NULL);
	hcbPedal = CreateWindow("button", "Pedal",
		WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_AUTOCHECKBOX,
		20, 50, 100, 30,
		hWnd, (HMENU)ID_BTN_PEDAL, hInstance, NULL);
	
	ZeroMemory(&logf, sizeof(LOGFONT));
	logf.lfHeight = 28;
	strcpy_s(logf.lfFaceName, "Monotype Corsiva");
	hf = CreateFontIndirect(&logf);

	SendMessage(hcbSign, WM_SETFONT, (WPARAM)hf, TRUE);
	SendMessage(hcbPedal, WM_SETFONT, (WPARAM)hf, TRUE);

	midiOutOpen(&hmidi, MIDI_MAPPER, 0, 0, 0);
	waveOutSetVolume(0, 0x66666666);
	FillKeyArrays();

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}



