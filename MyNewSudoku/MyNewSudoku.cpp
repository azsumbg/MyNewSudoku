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

D2D1_RECT_F bTxt1Rect{ 40.0f, 5.0f, scr_width / 3.0f - 50.0f, 50.0f };
D2D1_RECT_F bTxt2Rect{ scr_width / 3.0f + 40.0f, 5.0f, scr_width * 2.0f / 3.0f - 50.0f, 50.0f };
D2D1_RECT_F bTxt3Rect{ scr_width * 2.0f / 3.0f + 40.0f, 5.0f, scr_width - 50.0f, 50.0f };

bool pause = false;
bool show_help = false;
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
}







int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	bIns = hInstance;
	if (!bIns)ErrExit(eClass);













	ClearResources();
	std::remove(tmp_file);

    return (int) bMsg.wParam;
}