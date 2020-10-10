#include "paint.h"

int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)

{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Win32 Guided Tour"),
			NULL);

		return 1;
	}

	hInst = hInstance;

	HWND hWnd = CreateWindow(
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		1940, 1050,
		NULL,
		NULL,
		hInstance,
		NULL
		);

	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Win32 Guided Tour"),
			NULL);
		return 1;
	}

	ShowWindow(hWnd, SW_MAXIMIZE);
	UpdateWindow(hWnd);

	MSG msg;
	HACCEL hAccel = LoadAccelerators(hInstance, (LPCWSTR)IDR_ACCELERATOR);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateAccelerator(hWnd, hAccel, &msg);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

void SelectTool(HWND hWnd, int wmId)
{
	CheckMenuItem(GetMenu(hWnd), ID_TOOLS_PEN, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), ID_TOOLS_LINE, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), ID_TOOLS_POLYLINE, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), ID_TOOLS_RECTANGLE, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), ID_TOOLS_ELLIPSE, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), ID_TOOLS_TEXT, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), wmId, MF_CHECKED);
}


// Saving functions
PBITMAPINFO CreateBitmapInfoStruct(HWND hwnd, HBITMAP hBmp)
{
	BITMAP bmp;
	PBITMAPINFO pbmi;
	WORD cClrBits;

	GetObject(hBmp, sizeof(BITMAP), (LPSTR)&bmp);
	cClrBits = (WORD)(bmp.bmPlanes * bmp.bmBitsPixel);

	if (cClrBits == 1)
		cClrBits = 1;
	else if (cClrBits <= 4)
		cClrBits = 4;
	else if (cClrBits <= 8)
		cClrBits = 8;
	else if (cClrBits <= 16)
		cClrBits = 16;
	else if (cClrBits <= 24)
		cClrBits = 24;
	else cClrBits = 32;

	if (cClrBits < 24)
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)* (1 << cClrBits));
	else
		pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));

	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	pbmi->bmiHeader.biWidth = bmp.bmWidth;
	pbmi->bmiHeader.biHeight = bmp.bmHeight;
	pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
	pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;

	if (cClrBits < 24)
		pbmi->bmiHeader.biClrUsed = (1 << cClrBits);

	pbmi->bmiHeader.biCompression = BI_RGB;
	pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * cClrBits + 31) & ~31) / 8 * pbmi->bmiHeader.biHeight;
	pbmi->bmiHeader.biClrImportant = 0;
	return pbmi;
}

void CreateBMPFile(HWND hwnd, LPTSTR pszFile, PBITMAPINFO pbi, HBITMAP hBMP, HDC hDC)
{
	HANDLE hf;
	BITMAPFILEHEADER hdr;
	PBITMAPINFOHEADER pbih;
	LPBYTE lpBits;
	DWORD dwTotal;
	DWORD cb;
	BYTE *hp;
	DWORD dwTmp;

	pbih = (PBITMAPINFOHEADER)pbi;
	lpBits = (LPBYTE)GlobalAlloc(GMEM_FIXED, pbih->biSizeImage);

	GetDIBits(hDC, hBMP, 0, (WORD)pbih->biHeight, lpBits, pbi, DIB_RGB_COLORS);

	hf = CreateFile(pszFile, GENERIC_READ | GENERIC_WRITE, (DWORD)0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, (HANDLE)NULL);

	hdr.bfType = 0x4d42;
	hdr.bfSize = (DWORD)(sizeof(BITMAPFILEHEADER)+pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD)+pbih->biSizeImage);
	hdr.bfReserved1 = 0;
	hdr.bfReserved2 = 0;
	hdr.bfOffBits = (DWORD) sizeof(BITMAPFILEHEADER)+pbih->biSize + pbih->biClrUsed * sizeof(RGBQUAD);
	WriteFile(hf, (LPVOID)&hdr, sizeof(BITMAPFILEHEADER), (LPDWORD)&dwTmp, NULL);
	WriteFile(hf, (LPVOID)pbih, sizeof(BITMAPINFOHEADER)+pbih->biClrUsed * sizeof(RGBQUAD), (LPDWORD)&dwTmp, (NULL));
	dwTotal = cb = pbih->biSizeImage;
	hp = lpBits;
	WriteFile(hf, (LPSTR)hp, (int)cb, (LPDWORD)&dwTmp, NULL);
	CloseHandle(hf);
	GlobalFree((HGLOBAL)lpBits);
}

HBITMAP Create_hBitmap(HDC hDC, int w, int h)
{
	HDC hDCmem;
	HBITMAP hbm, holdBM;
	hDCmem = CreateCompatibleDC(hDC);
	hbm = CreateCompatibleBitmap(hDC, w, h);
	holdBM = (HBITMAP)SelectObject(hDCmem, hbm);
	BitBlt(hDCmem, 0, 0, w, h, hDC, 0, 0, SRCCOPY);
	SelectObject(hDCmem, holdBM);
	DeleteDC(hDCmem);
	DeleteObject(holdBM);
	return hbm;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	PAINTSTRUCT ps;
	static HDC mainDc, paintDc, currentDc = 0, bufferDc = 0, backupDc[BACKUPS];
	static INT backupDepth = -1, restoreCount = 0;
	static boolean initCheck;
	static draw drawMode;
	static CustomShape* shape = NULL;
	static CustomRubber* rubber = NULL;
	static Tools ToolId = PEN;
	static INT prevX = -1, prevY = -1, startX = -1, startY = -1;
	static BOOL isPolyLine;
	static HFONT font = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
	static POINT prevCoord, prevMove = { 0, 0 }, delta = { 0, 0 };
	static String str;
	HPEN pen;
	HBRUSH brush;
	CHOOSECOLOR cc;
	COLORREF acrCustClr[16];
	OPENFILENAME ofn;
	static TCHAR sfile[MAX_PATH];
	static BOOL isFile = false;
	static DOUBLE zoom = DEFAULT_ZOOM;
	static INT x, y;
	if (!initCheck){
		SelectTool(hWnd, ID_TOOLS_PEN);
		initCheck = true;
	}
	
	switch (message)
	{
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{

		case ID_TOOLS_PEN:
			SelectTool(hWnd, ID_TOOLS_PEN);
			ToolId = PEN;
			break;

		case ID_TOOLS_LINE:
			SelectTool(hWnd, ID_TOOLS_LINE);
			ToolId = LINE;
			break;

		case ID_TOOLS_RECTANGLE:
			SelectTool(hWnd, ID_TOOLS_RECTANGLE);
			ToolId = RECTANGLE;
			break;

		case ID_TOOLS_ELLIPSE:
			SelectTool(hWnd, ID_TOOLS_ELLIPSE);
			ToolId = ELLIPSE;
			break;



		case ID_TOOLS_POLYLINE:
			SelectTool(hWnd, ID_TOOLS_POLYLINE);
			isPolyLine = TRUE;
			prevX = -1;
			prevY = -1;
			ToolId = POLY;
			break;

		case ID_TOOLS_TEXT:
			ToolId = TEXT;
			break;

		case ID_FILE_EXIT:
			exit(0);
		}
		break;

	case WM_CREATE:
		GetClientRect(hWnd, &rect);
		initializeDcs(hWnd, mainDc, currentDc, bufferDc);
		initializeBackup(hWnd, mainDc, backupDc);
		createBackup(hWnd, backupDepth, restoreCount, bufferDc, backupDc);
		restoreCount = 0;
		DragAcceptFiles(hWnd, TRUE);
		break;

	case WM_LBUTTONDOWN:
		x = (short)(LOWORD(lParam) * zoom) - delta.x;
		y = (short)(HIWORD(lParam) * zoom) - delta.y;
		if (wParam & MK_CONTROL)
		{
			prevMove.x = (short)LOWORD(lParam);
			prevMove.y = (short)HIWORD(lParam);
		}
		else if (ToolId == PEN)
		{
			shape = new CustomPencil(x, y);
			shape->draw(bufferDc, x, y);
			drawMode = BUFFER;
		}
		else
		{
			switch (ToolId)
			{
			case LINE:
				shape = new CustomLine(x, y);
				break;

			case RECTANGLE:
				shape = new CustomRectangle(x, y);
				break;

			case ELLIPSE:
				shape = new CustomEllipse(x, y);
				break;

			case POLY:
				if (prevX == -1 && prevY == -1)
				{
					prevX = x;
					prevY = y;
					startX = prevX;
					startY = prevY;
				}
				shape = new CustomLine(prevX, prevY);
				break;

			case TEXT:
				prevX = x;
				prevY = y;
				str.clear();
				break;
			}
			drawMode = CURRENT;
		}
		SetCapture(hWnd);
		break;

	case WM_RBUTTONDOWN:
		x = (short)(LOWORD(lParam) * zoom) - delta.x;
		y = (short)(HIWORD(lParam) * zoom) - delta.y;
		rubber = new CustomRubber(x, y);
		useRubber(hWnd, rubber, x, y, currentDc, bufferDc, drawMode);
		SetCapture(hWnd);
		break;

	case WM_MOUSEMOVE:
		x = (short)(LOWORD(lParam) * zoom) - delta.x;
		y = (short)(HIWORD(lParam) * zoom) - delta.y;
		prevCoord.x = x;
		prevCoord.y = y;
		InvalidateRect(hWnd, NULL, FALSE);
		if (wParam & MK_LBUTTON)
		{
			if (wParam & MK_CONTROL)
			{
				delta.x += (short)((LOWORD(lParam) - prevMove.x) * zoom);
				delta.y += (short)((HIWORD(lParam) - prevMove.y) *zoom);
				prevMove.x = (short)LOWORD(lParam);
				prevMove.y = (short)HIWORD(lParam);
				drawMode = BUFFER;
				InvalidateRect(hWnd, NULL, true);
			}
			else if (shape)
			{
				if (ToolId == PEN)
				{
					shape->draw(bufferDc, x, y);
					drawMode = BUFFER;
				}
				else
				{
					BitBlt(currentDc, 0, 0, rect.right, rect.bottom, bufferDc, 0, 0, SRCCOPY);
					shape->draw(currentDc, x, y);
					drawMode = CURRENT;
				}
			}
		}
		else if (wParam & MK_RBUTTON)
		{
			if (rubber)
			{
				useRubber(hWnd, rubber, x, y, currentDc, bufferDc, drawMode);
			}
		}
		else
		{
			BitBlt(currentDc, 0, 0, rect.right, rect.bottom, bufferDc, 0, 0, SRCCOPY);
			MoveToEx(currentDc, prevCoord.x, prevCoord.y, NULL);
			LineTo(currentDc, prevCoord.x, prevCoord.y);
			drawMode = CURRENT;
		}
		break;

	case WM_LBUTTONUP:
		x = (short)(LOWORD(lParam) * zoom) - delta.x;
		y = (short)(HIWORD(lParam) * zoom) - delta.y;
		ReleaseCapture();
		if ((ToolId != PEN) && shape != NULL)
		{
			if (prevX != -1 && prevY != -1)
			{
				prevX = x;
				prevY = y;
			}
			shape->draw(bufferDc, x, y);
			drawMode = BUFFER;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		if (!(wParam & MK_CONTROL))
		{
			createBackup(hWnd, backupDepth, restoreCount, bufferDc, backupDc);
		}
		delete shape;
		shape = NULL;
		break;

	case WM_LBUTTONDBLCLK:
		if (ToolId == 4)
		{
			shape = new CustomLine(prevX, prevY);
			ReleaseCapture();
			InvalidateRect(hWnd, NULL, FALSE);
			if (!isPolyLine)
				shape->draw(bufferDc, startX, startY);
			else
				shape->draw(bufferDc, prevX, prevY);
			drawMode = BUFFER;
			prevX = -1;
			prevY = -1;
			startX = -1;
			startY = -1;
			delete shape;
			shape = NULL;
		}
		else if (wParam & MK_CONTROL)
		{
			delta = { 0, 0 };
			zoom = DEFAULT_ZOOM;
		}
		break;

	case WM_RBUTTONUP:
		ReleaseCapture();
		createBackup(hWnd, backupDepth, restoreCount, bufferDc, backupDc);
		if (rubber)
		{
			delete rubber;
			rubber = NULL;
		}
		break;

	case WM_PAINT:
		paintDc = BeginPaint(hWnd, &ps);

		switch (drawMode)
		{
		case CURRENT:
			StretchBlt(paintDc, 0, 0, rect.right, rect.bottom, currentDc, -delta.x, -delta.y, (int)(rect.right * zoom), (int)(rect.bottom * zoom), SRCCOPY);
			break;

		case BUFFER:
			StretchBlt(paintDc, 0, 0, rect.right, rect.bottom, bufferDc, -delta.x, -delta.y, (int)(rect.right * zoom), (int)(rect.bottom * zoom), SRCCOPY);
			break;

		case BACKUP:
			undo(hWnd, backupDepth, restoreCount, bufferDc, backupDc);
			StretchBlt(paintDc, 0, 0, rect.right, rect.bottom, bufferDc, -delta.x, -delta.y, (int)(rect.right * zoom), (int)(rect.bottom * zoom), SRCCOPY);
			drawMode = CURRENT;
			break;

		case RESTORE:
			restore(hWnd, backupDepth, restoreCount, bufferDc, backupDc);
			StretchBlt(paintDc, 0, 0, rect.right, rect.bottom, bufferDc, -delta.x, -delta.y, (int)(rect.right * zoom), (int)(rect.bottom * zoom), SRCCOPY);
			drawMode = CURRENT;
			break;
		}
		EndPaint(hWnd, &ps);
		break;

	case WM_MOUSEWHEEL:
		HPEN pen;
		if (wParam & MK_RBUTTON)
		{
			CustomRubber::rubberWidth += GET_WHEEL_DELTA_WPARAM(wParam) / 20;
			if (CustomRubber::rubberWidth < 0)
				CustomRubber::rubberWidth = 0;
			useRubber(hWnd, rubber, prevCoord.x, prevCoord.y, currentDc, bufferDc, drawMode);
		}
		else if (wParam & MK_CONTROL)
		{
			if (GET_WHEEL_DELTA_WPARAM(wParam) < 0)
			{
				zoom *= DELTA_ZOOM;
			}
			else
			{
				zoom /= DELTA_ZOOM;
			}
			drawMode = BUFFER;
			InvalidateRect(hWnd, &rect, true);
		}
		else
		{
			CustomShape::penWidth += GET_WHEEL_DELTA_WPARAM(wParam) / 20;
			if (CustomShape::penWidth < 0)
				CustomShape::penWidth = 0;
			pen = CreatePen(CustomShape::penStyle, CustomShape::penWidth, CustomShape::penColor);
			DeleteObject(SelectObject(currentDc, pen));
			DeleteObject(SelectObject(bufferDc, pen));
			InvalidateRect(hWnd, NULL, FALSE);
			BitBlt(currentDc, 0, 0, rect.right, rect.bottom, bufferDc, 0, 0, SRCCOPY);
			MoveToEx(currentDc, prevCoord.x, prevCoord.y, NULL);
			LineTo(currentDc, prevCoord.x, prevCoord.y);
			drawMode = CURRENT;
		}
		break;

	case WM_CHAR:
		if (ToolId == 5)
		{
			str += (TCHAR)wParam;
			InvalidateRect(hWnd, NULL, FALSE);
			TextOut(bufferDc, prevX, prevY, str.data(), str.size());
			drawMode = BUFFER;
		}
		break;

	case WM_ERASEBKGND:
		FillRect(mainDc, &rect, (HBRUSH)BLACK_BRUSH);
		break;

	case WM_SIZE:
	case WM_MOVE:
		InvalidateRect(hWnd, &rect, TRUE);
		drawMode = BUFFER;
		break;

	case WM_DROPFILES:
		HDROP hdrop;
		hdrop = (HDROP)wParam;

		if (DragQueryFile(hdrop, 0xFFFFFFFF, NULL, NULL) != 1){
			MessageBox(NULL, _T("Dropping multiple files is not supported."), NULL, MB_ICONERROR);
			DragFinish(hdrop);
			break;
		}
		if (DragQueryFile(hdrop, 0, sfile, MAX_PATH))
		{
			HBITMAP hBitmap;
			BITMAP bm;
			HDC hDC;
			HDC hMemDC;

			hDC = GetDC(hWnd);
			hMemDC = CreateCompatibleDC(hDC);
			hBitmap = (HBITMAP)LoadImage(hInst, sfile, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
			isFile = true;
			GetObject(hBitmap, sizeof(BITMAP), &bm);
			SelectObject(bufferDc, hBitmap);
			InvalidateRect(hWnd, &rect, TRUE);
			DeleteDC(hMemDC);
			ReleaseDC(hWnd, hDC);
			DeleteObject(hBitmap);
		}
		DragFinish(hdrop);
		break;

	case WM_DESTROY:
		ReleaseDC(hWnd, mainDc);
		ReleaseDC(hWnd, currentDc);
		ReleaseDC(hWnd, bufferDc);
		DragAcceptFiles(hWnd, FALSE);
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

