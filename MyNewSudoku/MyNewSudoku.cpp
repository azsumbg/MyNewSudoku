#include "framework.h"
#include "MyNewSudoku.h"
#include <mmsystem.h>
#include <d2d1.h>
#include <dwrite.h>
#include "D2BMPLOADER.h"
#include "ErrH.h"
#include "FCheck.h"
#include "sudoku.h"
#include <chrono>
#include <clocale>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d2bmploader.lib")
#pragma comment(lib, "errh.lib")
#pragma comment(lib, "fcheck.lib")
#pragma comment(lib, "sudoku.lib")

constexpr wchar_t bWinClassName[]{ L"MySudoku" };
constexpr char tmp_file[]{ ".\\res\\data\\temp.dat" };
constexpr wchar_t Ltmp_file[]{ L".\\res\\data\\temp.dat" };
constexpr wchar_t sound_file[]{ L".\\res\\snd\\main.wav" };
constexpr wchar_t help_file[]{ L".\\res\\data\\help.dat" };
constexpr wchar_t record_file[]{ L".\\res\\data\\record.dat" };
constexpr wchar_t save_file[]{ L".\\res\\data\\save.dat" };

constexpr int mNew{ 1001 };
constexpr int mLvl{ 1002 };
constexpr int mExit{ 1003 };
constexpr int mSave{ 1004 };
constexpr int mLoad{ 1005 };
constexpr int mHoF{ 1006 };

constexpr int record{ 2001 };
constexpr int first_record{ 2002 };
constexpr int no_record{ 2003 };

WNDCLASS bWinClass{};
HINSTANCE bIns{ nullptr };
HWND bHwnd{ nullptr };
HICON mainIcon{ nullptr };
HCURSOR mainCur{ nullptr };
HCURSOR outCur{ nullptr };
POINT cur_pos{};
float x_scale{ 0 };
float y_scale{ 0 };
HDC PaintDC{ nullptr };
HMENU bBar{ nullptr };
HMENU bMain{ nullptr };
HMENU bStore{ nullptr };
PAINTSTRUCT bPaint{};
UINT bTimer{ 0 };
MSG bMsg{};
BOOL bRet{ 0 };

D2D1_RECT_F b1Rect{ 20.0f, 0, scr_width / 3.0f - 50.0f, 50.0f };
D2D1_RECT_F b2Rect{ scr_width / 3.0f + 20.0f, 0, scr_width * 2.0f / 3.0f - 50.0f, 50.0f };
D2D1_RECT_F b3Rect{ scr_width * 2.0f / 3.0f + 20.0f, 0, scr_width - 50.0f, 50.0f };

D2D1_RECT_F b1TxtRect{ 40.0f, 5.0f, scr_width / 3.0f - 40.0f, 50.0f };
D2D1_RECT_F b2TxtRect{ scr_width / 3.0f + 30.0f, 5.0f, scr_width * 2.0f / 3.0f - 40.0f, 50.0f };
D2D1_RECT_F b3TxtRect{ scr_width * 2.0f / 3.0f + 30.0f, 5.0f, scr_width - 40.0f, 50.0f };

bool pause = false;
bool show_help = false;
bool in_client = true;
bool sound = true;
bool name_set = false;
bool b1Hglt = false;
bool b2Hglt = false;
bool b3Hglt = false;

wchar_t current_player[16]{ L"TARLYO" };

int level = 1;
int score = 0;
int mins = 0;
int secs = 0;

ID2D1Factory* iFactory{ nullptr };
ID2D1HwndRenderTarget* Draw{ nullptr };

ID2D1RadialGradientBrush* b1BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b2BckgBrush{ nullptr };
ID2D1RadialGradientBrush* b3BckgBrush{ nullptr };

ID2D1SolidColorBrush* statBrush{ nullptr };
ID2D1SolidColorBrush* txtBrush{ nullptr };
ID2D1SolidColorBrush* hgltBrush{ nullptr };
ID2D1SolidColorBrush* inactBrush{ nullptr };
ID2D1SolidColorBrush* gridBrush{ nullptr };

ID2D1SolidColorBrush* validBrush{ nullptr };
ID2D1SolidColorBrush* invalidBrush{ nullptr };

IDWriteFactory* iWriteFactory{ nullptr };
IDWriteTextFormat* nrmFormat{ nullptr };
IDWriteTextFormat* midFormat{ nullptr };
IDWriteTextFormat* bigFormat{ nullptr };

ID2D1Bitmap* bmpLogo{ nullptr };
ID2D1Bitmap* bmpIntro[30]{ nullptr };

/////////////////////////////////////////////////////////

dll::GRID* Grid{ nullptr };

dll::RANDIT RandIt{};



/////////////////////////////////////////////////////////

template<typename T>concept HasRelease = requires(T check)
{
	check.Release();
};
template<HasRelease T>bool ClearMem(T** var)
{
	if ((*var))
	{
		(*var)->Release();
		(*var) = nullptr;
		return true;
	}
	return false;
}
void LogErr(LPCWSTR what)
{
	std::wofstream log(L".\\res\\data\\error.log", std::ios::app);
	log << what << L" time stamp: " << std::chrono::system_clock::now() << std::endl;
	log.close();
}
void ClearResources()
{
	if (!ClearMem(&iFactory))LogErr(L"Error releasing D2D1 Factory !");
	if (!ClearMem(&Draw))LogErr(L"Error releasing D2D1 HwndRenderTarget !");
	if (!ClearMem(&b1BckgBrush))LogErr(L"Error releasing b1BckgBrush !");
	if (!ClearMem(&b2BckgBrush))LogErr(L"Error releasing b2BckgBrush !");
	if (!ClearMem(&b3BckgBrush))LogErr(L"Error releasing b3BckgBrush !");

	if (!ClearMem(&statBrush))LogErr(L"Error releasing statBrush !");
	if (!ClearMem(&txtBrush))LogErr(L"Error releasing txtBrush !");
	if (!ClearMem(&hgltBrush))LogErr(L"Error releasing hgltBrush !");
	if (!ClearMem(&inactBrush))LogErr(L"Error releasing inactBrush !");
	if (!ClearMem(&validBrush))LogErr(L"Error releasing validBrush !");
	if (!ClearMem(&invalidBrush))LogErr(L"Error releasing invalidBrush !");
	if (!ClearMem(&invalidBrush))LogErr(L"Error releasing gridBrush !");

	if (!ClearMem(&iWriteFactory))LogErr(L"Error releasing D2D1 WriteFactory !");
	if (!ClearMem(&nrmFormat))LogErr(L"Error releasing D2D1 nrmTextFormat !");
	if (!ClearMem(&midFormat))LogErr(L"Error releasing D2D1 midTextFormat !");
	if (!ClearMem(&bigFormat))LogErr(L"Error releasing D2D1 bigTextFormat !");

	if (!ClearMem(&bmpLogo))LogErr(L"Error releasing D2D1 bmpLogo !");

	for (int i = 0; i < 30; ++i)if (!ClearMem(&bmpIntro[i]))LogErr(L"Error releasing D2D1 bmpIntro !");
}
void ErrExit(int what)
{
	MessageBeep(MB_ICONERROR);
	MessageBox(NULL, ErrHandle(what), L"Критична грешка !", MB_ICONERROR | MB_OK | MB_APPLMODAL);

	ClearResources();
	std::remove(tmp_file);
	exit(1);
}
int IntroFrame()
{
	static int frame = 0;
	static int delay = 5;

	--delay;
	if (delay < 0)
	{
		delay = 5;
		++frame;
		if (frame > 29)frame = 0;
	}

	return frame;
}

void GameOver()
{
	PlaySound(NULL, NULL, NULL);
	KillTimer(bHwnd, bTimer);



	bMsg.message = WM_QUIT;
	bMsg.wParam = 0;
}
void InitGame()
{
	wcscpy_s(current_player, L"TARLYO");
	name_set = false;

	level = 1;
	score = 0;
	mins = 0;
	secs = 0;

	if (Grid)delete Grid;
	Grid = new dll::GRID();
	Grid->set_level(1);
}

INT_PTR CALLBACK DlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
	switch (ReceivedMsg)
	{
	case WM_INITDIALOG:
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)(mainIcon));
		return true;

	case WM_CLOSE:
		EndDialog(hwnd, IDCANCEL);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;

		case IDOK:
			if (GetDlgItemText(hwnd, IDC_NAME, current_player, 16) < 1)
			{
				if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
				wcscpy_s(current_player, L"TARLYO");
				MessageBox(bHwnd, L"Ха, ха, хааа ! Забрави си името !", L"Забраватор !", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
				EndDialog(hwnd, IDCANCEL);
				break;
			}
			EndDialog(hwnd, IDOK);
			break;
		}
		break;
	}

	return (INT_PTR)(FALSE);
}
LRESULT CALLBACK WinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
	switch (ReceivedMsg)
	{
	case WM_CREATE:
		if (bIns)
		{
			SetTimer(hwnd, bTimer, 1000, NULL);

			bBar = CreateMenu();
			bMain = CreateMenu();
			bStore = CreateMenu();

			AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bMain), L"Основно меню");
			AppendMenu(bBar, MF_POPUP, (UINT_PTR)(bStore), L"Меню за данни");

			AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
			AppendMenu(bMain, MF_STRING, mLvl, L"Повиши трудността");
			AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
			AppendMenu(bMain, MF_STRING, mExit, L"Изход");

			AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
			AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
			AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
			AppendMenu(bStore, MF_STRING, mHoF, L"Зала на славата");

			SetMenu(hwnd, bBar);

			InitGame();
		}
		break;

	case WM_CLOSE:
		pause = true;
		if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
		if (MessageBox(hwnd, L"Ако излезеш, губиш тази игра !\n\nНаистина ли излизаш ?", L"Изход",
			MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
		{
			pause = false;
			break;
		}
		GameOver();
		break;

	case WM_PAINT:
		PaintDC = BeginPaint(hwnd, &bPaint);
		FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(10, 10, 10)));
		EndPaint(hwnd, &bPaint);
		break;

	case WM_TIMER:
		if (pause)break;
		++mins;
		secs = mins / 60;
		break;

	case WM_SETCURSOR:
		GetCursorPos(&cur_pos);
		ScreenToClient(hwnd, &cur_pos);
		if (LOWORD(lParam) == HTCLIENT)
		{
			if (!in_client)
			{
				in_client = true;
				pause = false;
			}

			if (cur_pos.y * y_scale <= 50)
			{
				if (cur_pos.x * x_scale >= b1Rect.left && cur_pos.x * x_scale <= b1Rect.right)
				{
					if (!b1Hglt)
					{
						if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
						b1Hglt = true;
						b2Hglt = false;
						b3Hglt = false;
					}
				}
				else if (cur_pos.x * x_scale >= b2Rect.left && cur_pos.x * x_scale <= b2Rect.right)
				{
					if (!b2Hglt)
					{
						if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
						b1Hglt = false;
						b2Hglt = true;
						b3Hglt = false;
					}
				}
				else if (cur_pos.x * x_scale >= b3Rect.left && cur_pos.x * x_scale <= b3Rect.right)
				{
					if (!b3Hglt)
					{
						if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
						b1Hglt = false;
						b2Hglt = false;
						b3Hglt = true;
					}
				}
				else
				{
					if (b1Hglt || b2Hglt || b3Hglt)
					{
						if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
						b1Hglt = false;
						b2Hglt = false;
						b3Hglt = false;
					}
				}

				SetCursor(outCur);
				return true;
			}
			else
			{
				if (b1Hglt || b2Hglt || b3Hglt)
				{
					if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
					b1Hglt = false;
					b2Hglt = false;
					b3Hglt = false;
				}
			
				SetCursor(mainCur);
				return true;
			}
		}
		else
		{
			if (in_client)
			{
				in_client = false;
				pause = true;
			}

			if (b1Hglt || b2Hglt || b3Hglt)
			{
				if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
				b1Hglt = false;
				b2Hglt = false;
				b3Hglt = false;
			}

			SetCursor(LoadCursor(NULL, IDC_ARROW));
			return true;
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case mNew:
			pause = true;
			if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
			if (MessageBox(hwnd, L"Ако рестартираш, губиш тази игра !\n\nНаистина ли рестартираш ?", 
				L"Рестарт", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
			{
				pause = false;
				break;
			}
			InitGame();
			break;

		case mLvl:
			pause = true;
			if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
			if (MessageBox(hwnd, L"Ако смениш трудността, губиш тази игра !\n\nНаистина ли сменяш трудността ?",
				L"Смяна на трудността", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
			{
				pause = false;
				break;
			}
			//LevelUp();
			break;

		case mExit:
			SendMessage(hwnd, WM_CLOSE, NULL, NULL);





		}
		break;


	default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
	}

	return (LRESULT)(FALSE);
}

void CreateResources()
{
	int result = 0;
	CheckFile(Ltmp_file, &result);
	if (result == FILE_EXIST)ErrExit(eStarted);
	else
	{
		std::wofstream start(Ltmp_file);
		start << L"Game started at: " << std::chrono::system_clock::now();
		start.close();
	}

	int win_x = GetSystemMetrics(SM_CXSCREEN) / 2 - (int)(scr_width / 2.0f);
	int win_y = 100;

	if (GetSystemMetrics(SM_CXSCREEN) < (int)(scr_width) + win_x || GetSystemMetrics(SM_CYSCREEN) < (int)(scr_height) + win_y)
		ErrExit(eScreen);

	mainIcon = (HICON)(LoadImage(NULL, L".\\res\\main.ico", IMAGE_ICON, 255, 255, LR_LOADFROMFILE));
	if (!mainIcon)ErrExit(eIcon);

	mainCur = LoadCursorFromFile(L".\\res\\main.ani");
	outCur = LoadCursorFromFile(L".\\res\\out.ani");

	if (!mainCur || !outCur)ErrExit(eCursor);

	bWinClass.lpszClassName = bWinClassName;
	bWinClass.hInstance = bIns;
	bWinClass.lpfnWndProc = &WinProc;
	bWinClass.hbrBackground = CreateSolidBrush(RGB(10, 10, 10));
	bWinClass.hIcon = mainIcon;
	bWinClass.hCursor = mainCur;
	bWinClass.style = CS_DROPSHADOW;

	if (!RegisterClass(&bWinClass))ErrExit(eClass);

	bHwnd = CreateWindow(bWinClassName, L"ГОТИНО СУДОКУ", WS_CAPTION | WS_SYSMENU, win_x, win_y, (int)(scr_width),
		(int)(scr_height), NULL, NULL, bIns, NULL);
	if (!bHwnd)ErrExit(eWindow);
	else
	{
		ShowWindow(bHwnd, SW_SHOWDEFAULT);

		HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
		if (hr != S_OK)
		{
			LogErr(L"Error creating D2D1 main Factory !");
			ErrExit(eD2D);
		}

		if (iFactory)
		{
			hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(bHwnd,
				D2D1::SizeU((UINT32)(scr_width), (UINT32)(scr_height))), &Draw);
		
			if (hr != S_OK)
			{
				LogErr(L"Error creating D2D1 HwndRenderTarget !");
				ErrExit(eD2D);
			}
		
			if (Draw)
			{
				RECT ClR{};
				D2D1_SIZE_F hwndR{};

				GetClientRect(bHwnd, &ClR);
				hwndR = Draw->GetSize();
			
				x_scale = static_cast<float>(hwndR.width / (ClR.right - ClR.left));
				y_scale = static_cast<float>(hwndR.height / (ClR.bottom - ClR.top));
			
				D2D1_GRADIENT_STOP gStops[2]{};
				ID2D1GradientStopCollection* gColl{ nullptr };

				gStops[0].position = 0;
				gStops[0].color = D2D1::ColorF(D2D1::ColorF::PaleGreen);
				gStops[1].position = 1.0f;
				gStops[1].color = D2D1::ColorF(D2D1::ColorF::DarkOliveGreen);

				hr = Draw->CreateGradientStopCollection(gStops, 2, &gColl);
				if (hr != S_OK)
				{
					LogErr(L"Error creating D2D1 GradientStopCollection !");
					ErrExit(eD2D);
				}

				if (gColl)
				{
					hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b1Rect.left +
						((b1Rect.right - b1Rect.left) / 2.0f), 25.0f), D2D1::Point2F(0, 0), 
						(b1Rect.right - b1Rect.left) / 2.0f, 25.0f), gColl, &b1BckgBrush);
					hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b2Rect.left +
						((b2Rect.right - b2Rect.left) / 2.0f), 25.0f), D2D1::Point2F(0, 0),
						(b2Rect.right - b2Rect.left) / 2.0f, 25.0f), gColl, &b2BckgBrush);
					hr = Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F(b3Rect.left +
						((b3Rect.right - b3Rect.left) / 2.0f), 25.0f), D2D1::Point2F(0, 0),
						(b3Rect.right - b3Rect.left) / 2.0f, 25.0f), gColl, &b3BckgBrush);
					
					if (hr != S_OK)
					{
						LogErr(L"Error creating D2D1 RadialGradientBrush !");
						ErrExit(eD2D);
					}
				
					ClearMem(&gColl);
				}

				hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::CornflowerBlue), &statBrush);
				hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkBlue), &txtBrush);
				hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gold), &hgltBrush);
				hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Firebrick), &inactBrush);
				hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green), &validBrush);
				hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::OrangeRed), &invalidBrush);
				hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Brown), &gridBrush);

				if (hr != S_OK)
				{
					LogErr(L"Error creating D2D1 SolidColorBrushes !");
					ErrExit(eD2D);
				}
			
				bmpLogo = Load(L".\\res\\img\\logo.png", Draw);
				if (!bmpLogo)
				{
					LogErr(L"Error loading game logo !");
					ErrExit(eD2D);
				}
			
				for (int i = 0; i < 30; ++i)
				{
					wchar_t name[50]{ L".\\res\\img\\intro\\" };
					wchar_t add[5]{ L"\0" };

					wsprintf(add, L"%d", i);
					wcscat_s(name, add);
					wcscat_s(name, L".png");

					bmpIntro[i] = Load(name, Draw);
					if (!bmpIntro[i])
					{
						LogErr(L"Error loading game Intro !");
						ErrExit(eD2D);
					}
				}
			}
		}

		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(&iWriteFactory));
		if (hr != S_OK)
		{
			LogErr(L"Error creating D2D1 WriteFactory !");
			ErrExit(eD2D);
		}

		if (iWriteFactory)
		{
			hr = iWriteFactory->CreateTextFormat(L"Copperplate Gothic", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK,
				DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 10.0f, L"", &nrmFormat);
			hr = iWriteFactory->CreateTextFormat(L"Copperplate Gothic", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK,
				DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 28.0f, L"", &midFormat);
			hr = iWriteFactory->CreateTextFormat(L"Copperplate Gothic", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK,
				DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 72.0f, L"", &bigFormat);
			if (hr != S_OK)
			{
				LogErr(L"Error creating D2D1 TextFormats !");
				ErrExit(eD2D);
			}
		}
	}

	Draw->BeginDraw();
	if (bmpLogo)Draw->DrawBitmap(bmpLogo, D2D1::RectF(0, 0, scr_width, scr_height));
	Draw->EndDraw();
	
	PlaySound(L".\\res\\snd\\intro.wav", NULL, SND_SYNC);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	_wsetlocale(LOCALE_ALL, L"");
	
	bIns = hInstance;
	if (!bIns)ErrExit(eClass);

	CreateResources();

	PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);


	while (bMsg.message != WM_QUIT)
	{
		if ((bRet = PeekMessage(&bMsg, bHwnd, NULL, NULL, PM_REMOVE)) != 0)
		{
			if (bRet == -1)ErrExit(eMsg);

			TranslateMessage(&bMsg);
			DispatchMessage(&bMsg);
		}

		if (pause)
		{
			if (show_help)continue;
			
			Draw->BeginDraw();
			Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkCyan));
			Draw->DrawBitmap(bmpIntro[IntroFrame()], D2D1::RectF(0, 0, scr_width, scr_height));
			if (txtBrush && bigFormat)Draw->DrawTextW(L"ПАУЗА", 6, bigFormat, D2D1::RectF(scr_width / 2.0f - 120.0f,
				80.0f, scr_width, scr_height), txtBrush);
			Draw->EndDraw();
			continue;
		}

		///////////////////////////////////////////////////////////////////

		








		//DRAW THINGS *******************************

		Draw->BeginDraw();

		if (statBrush && txtBrush && hgltBrush && inactBrush && b1BckgBrush && b2BckgBrush && b3BckgBrush && nrmFormat)
		{
			Draw->Clear(D2D1::ColorF(D2D1::ColorF::Indigo));
			Draw->FillRectangle(D2D1::RectF(0, 0, scr_width, 50.0f), statBrush);

			Draw->FillRoundedRectangle(D2D1::RoundedRect(b1Rect, 10.0f, 15.0f), b1BckgBrush);
			Draw->FillRoundedRectangle(D2D1::RoundedRect(b2Rect, 10.0f, 15.0f), b2BckgBrush);
			Draw->FillRoundedRectangle(D2D1::RoundedRect(b3Rect, 10.0f, 15.0f), b3BckgBrush);

			if (name_set)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmFormat, b1TxtRect, inactBrush);
			else
			{
				if (b1Hglt)Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmFormat, b1TxtRect, hgltBrush);
				else Draw->DrawTextW(L"ИМЕ НА ИГРАЧ", 13, nrmFormat, b1TxtRect, txtBrush);
			}
			if (b2Hglt)Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmFormat, b2TxtRect, hgltBrush);
			else Draw->DrawTextW(L"ЗВУЦИ ON / OFF", 15, nrmFormat, b2TxtRect, txtBrush);
			if (b3Hglt)Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmFormat, b3TxtRect, hgltBrush);
			else Draw->DrawTextW(L"ПОМОЩ ЗА ИГРАТА", 16, nrmFormat, b3TxtRect, txtBrush);
		}

		///////////////////////////////////////////////////

		for (int rows = 0; rows < MAX_ROWS; ++rows)
		{
			for (int cols = 0; cols < MAX_COLS; ++cols)
			{
				if (Grid->get_value(rows, cols) != CLEAR_VALUE)
				{
					FRECT box{ Grid->get_dims(rows,cols) };
					if (Grid->value_ok(rows, cols))Draw->FillRectangle(D2D1::RectF(box.left, box.up, box.right, box.down), validBrush);
					else Draw->FillRectangle(D2D1::RectF(box.left, box.up, box.right, box.down), invalidBrush);
				}
			}
		}

		if (gridBrush)
		{
			for (int i = 0; i < MAX_ROWS; ++i)
			{
				FRECT temp_dims{ Grid->get_dims(i, 8) };
				float line_y = temp_dims.up;
				if (i % 3 == 0)Draw->DrawLine(D2D1::Point2F(5.0f, line_y), D2D1::Point2F(temp_dims.right, line_y), gridBrush,3.0f);
				else Draw->DrawLine(D2D1::Point2F(5.0f, line_y), D2D1::Point2F(temp_dims.right, line_y), gridBrush);
			
				if (i == 8)Draw->DrawLine(D2D1::Point2F(5.0f, temp_dims.down),
					D2D1::Point2F(temp_dims.right, temp_dims.down), gridBrush, 3.0f);
			}

			for (int i = 0; i < MAX_COLS; ++i)
			{
				FRECT temp_dims{ Grid->get_dims(8, i) };
				float line_x = temp_dims.left;
				if (i % 3 == 0)Draw->DrawLine(D2D1::Point2F(line_x, sky + 5.0f), 
					D2D1::Point2F(line_x, temp_dims.down), gridBrush, 3.0f);
				else Draw->DrawLine(D2D1::Point2F(line_x, sky + 5.0f), D2D1::Point2F(line_x, temp_dims.down), gridBrush);
			
				if (i == 8)Draw->DrawLine(D2D1::Point2F(temp_dims.right, sky + 5.0f),
					D2D1::Point2F(temp_dims.right, temp_dims.down), gridBrush, 3.0f);
			}
		}

		for (int rows = 0; rows < MAX_ROWS; rows++)
		{
			for (int cols = 0; cols < MAX_ROWS; cols++)
			{
				if (Grid->get_value(rows, cols) != CLEAR_VALUE)
				{
					FRECT temp_dim{ Grid->get_dims(rows,cols) };
					D2D1_RECT_F numRect{ temp_dim.left + 5.0f,temp_dim.up + 5.0f,temp_dim.right - 5.0f,temp_dim.down - 5.0f };
					wchar_t num[2]{ L"\0" };
					wsprintf(num, L"%d", Grid->get_value(rows, cols));
				
					if (hgltBrush)Draw->DrawTextW(num, 2, midFormat, numRect, hgltBrush);
				}
			}
		}





		/////////////////////////////////////////////
		
		Draw->EndDraw();

	}

	ClearResources();
	std::remove(tmp_file);

    return (int) bMsg.wParam;
}