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

bool turn_the_game = false;
bool level_skipped = false;

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
IDWriteTextFormat* statFormat{ nullptr };
IDWriteTextFormat* bigFormat{ nullptr };

ID2D1Bitmap* bmpLogo{ nullptr };
ID2D1Bitmap* bmpRecord{ nullptr };
ID2D1Bitmap* bmpFirstRecord{ nullptr };
ID2D1Bitmap* bmpNoRecord{ nullptr };
ID2D1Bitmap* bmpLevelUp{ nullptr };
ID2D1Bitmap* bmpVictory{ nullptr };
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
	if (!ClearMem(&gridBrush))LogErr(L"Error releasing gridBrush !");

	if (!ClearMem(&iWriteFactory))LogErr(L"Error releasing D2D1 WriteFactory !");
	if (!ClearMem(&nrmFormat))LogErr(L"Error releasing D2D1 nrmTextFormat !");
	if (!ClearMem(&midFormat))LogErr(L"Error releasing D2D1 midTextFormat !");
	if (!ClearMem(&bigFormat))LogErr(L"Error releasing D2D1 bigTextFormat !");
	if (!ClearMem(&statFormat))LogErr(L"Error releasing D2D1 statTextFormat !");

	if (!ClearMem(&bmpLogo))LogErr(L"Error releasing D2D1 bmpLogo !");
	if (!ClearMem(&bmpLevelUp))LogErr(L"Error releasing D2D1 bmpLevelUp !");
	if (!ClearMem(&bmpVictory))LogErr(L"Error releasing D2D1 bmpVictory !");
	if (!ClearMem(&bmpRecord))LogErr(L"Error releasing D2D1 bmpRecord !");
	if (!ClearMem(&bmpFirstRecord))LogErr(L"Error releasing D2D1 bmpFirstRecord !");
	if (!ClearMem(&bmpNoRecord))LogErr(L"Error releasing D2D1 bmpNoRecord !");

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
	static int delay = 3;

	--delay;
	if (delay < 0)
	{
		delay = 5;
		++frame;
		if (frame > 29)frame = 0;
	}

	return frame;
}
BOOL CheckRecord()
{
	if (score < 1)return no_record;

	int result = 0;
	CheckFile(record_file, &result);

	if (result == FILE_NOT_EXIST)
	{
		std::wofstream rec(record_file);
		rec << score << std::endl;
		for (int i = 0; i < 16; ++i)rec << static_cast<int>(current_player[i]) << std::endl;
		rec.close();
		return first_record;
	}
	else
	{
		std::wifstream check(record_file);
		check >> result;
		check.close();
	}

	if (score > result)
	{
		std::wofstream rec(record_file);
		rec << score << std::endl;
		for (int i = 0; i < 16; ++i)rec << static_cast<int>(current_player[i]) << std::endl;
		rec.close();
		return record;
	}

	return no_record;
}
void GameOver()
{
	PlaySound(NULL, NULL, NULL);
	KillTimer(bHwnd, bTimer);

	if (turn_the_game)
	{
		score += 1000;
		Draw->BeginDraw();
		Draw->DrawBitmap(bmpVictory, D2D1::RectF(0, 0, scr_width, scr_height));
		Draw->EndDraw();
		if (sound)PlaySound(L".\\res\\snd\\victory.wav", NULL, SND_SYNC);
		else Sleep(3000);
	}

	switch (CheckRecord())
	{
	case no_record:
		Draw->BeginDraw();
		Draw->DrawBitmap(bmpNoRecord, D2D1::RectF(0, 0, scr_width, scr_height));
		Draw->EndDraw();
		if (sound)PlaySound(L".\\res\\snd\\loose.wav", NULL, SND_SYNC);
		else Sleep(3000);
		break;

	case first_record:
		Draw->BeginDraw();
		Draw->DrawBitmap(bmpFirstRecord, D2D1::RectF(0, 0, scr_width, scr_height));
		Draw->EndDraw();
		if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_SYNC);
		else Sleep(3000);
		break;

	case record:
		Draw->BeginDraw();
		Draw->DrawBitmap(bmpRecord, D2D1::RectF(0, 0, scr_width, scr_height));
		Draw->EndDraw();
		if (sound)PlaySound(L".\\res\\snd\\record.wav", NULL, SND_SYNC);
		else Sleep(3000);
		break;
	}

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

	level_skipped = false;

	if (Grid)delete Grid;
	Grid = new dll::GRID();
	Grid->set_level(1);
}
void LevelUp()
{
	if (level + 1 > 5 && !level_skipped)
	{
		turn_the_game = true;
		GameOver();
	}
	else if (level + 1 > 5)
	{
		if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
		MessageBox(bHwnd, L"Няма повече нива !", L"Последно ниво", MB_OK | MB_APPLMODAL | MB_ICONERROR);
		return;
	}
	
	Draw->BeginDraw();
	if (bmpLogo)Draw->DrawBitmap(bmpLevelUp, D2D1::RectF(0, 0, scr_width, scr_height));
	Draw->EndDraw();

	if (sound)
	{
		PlaySound(NULL, NULL, NULL);
		PlaySound(L".\\res\\snd\\levelup.wav", NULL, SND_SYNC);
		PlaySound(sound_file, NULL, SND_ASYNC | SND_LOOP);
	}
	else Sleep(3000);

	if (!level_skipped)
	{
		int time_needed = 300 * level;
		if (time_needed - secs > 0)score += 50 * level + time_needed - secs;
		else score += 50 * level;
	}

	++level;
	level_skipped = false;
	
	mins = 0;
	secs = 0;

	if (Grid)delete Grid;
	Grid = new dll::GRID();
	Grid->set_level(level);
}
void HallOfFame()
{
	int result = 0;
	CheckFile(record_file, &result);

	if (result == FILE_NOT_EXIST)
	{
		if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
		MessageBox(bHwnd, L"Все още липсва рекорд на играта !\n\nПостарай се повече !", L"Липсва файл !",
			MB_OK | MB_APPLMODAL | MB_ICONERROR);
		return;
	}

	wchar_t rec_txt[75]{ L"НАЙ-ДОБЪР ИГРАЧ: " };
	wchar_t add[5]{ L"\0" };

	wchar_t saved_player[16]{ L"\0" };
	int saved_score = 0;

	std::wifstream rec{ record_file };
	rec >> saved_score;
	for (int i = 0; i < 16; ++i)
	{
		int letter = 0;
		rec >> letter;
		saved_player[i] = static_cast<wchar_t>(letter);
	}
	rec.close();

	wsprintf(add, L"%d", saved_score);
	wcscat_s(rec_txt, saved_player);
	wcscat_s(rec_txt, L"\n\nСВЕТОВЕН РЕКОРД: ");
	wcscat_s(rec_txt, add);

	result = 0;
	for (int i = 0; i < 75; ++i)
	{
		if (rec_txt[i] != '\0')++result;
		else break;
	}

	Draw->BeginDraw();
	Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkCyan));
	if (midFormat && hgltBrush)Draw->DrawTextW(rec_txt, result, midFormat, D2D1::RectF(5.0f, 80.0f, scr_width, scr_height), 
		hgltBrush);
	Draw->EndDraw();

	if (sound)mciSendString(L"play .\\res\\snd\\showrec.wav", NULL, NULL, NULL);

	Sleep(5000);
}
void SaveGame()
{
	int result = 0;
	CheckFile(save_file, &result);
	if (result == FILE_EXIST)
	{
		if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
		if (MessageBox(bHwnd, L"Има предишна записана игра, която ще изгубиш !\n\nНаистина ли я презаписваш ?", L"Презапис",
			MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)return;
	}

	std::wofstream save(save_file);
	
	save << level << std::endl;
	save << score << std::endl;
	save << mins << std::endl;
	save << secs << std::endl;
	save << level_skipped << std::endl;

	for (int i = 0; i < 16; ++i)save << static_cast<int>(current_player[i]) << std::endl;
	save << name_set << std::endl;
	save << turn_the_game << std::endl;

	for (int rows = 0; rows < MAX_ROWS; ++rows)
	{
		for (int cols = 0; cols < MAX_COLS; ++cols)
		{
			save << Grid->get_value(rows, cols) << std::endl;
		}
	}

	save.close();

	if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);
	MessageBox(bHwnd, L"Играта е записана !", L"Запис", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
}
void LoadGame()
{
	int result = 0;
	CheckFile(save_file, &result);

	if (result == FILE_NOT_EXIST)
	{
		if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
		MessageBox(bHwnd, L"Все още няма записана игра !\n\nПостарай се повече !", L"Липсва файл !",
			MB_OK | MB_APPLMODAL | MB_ICONERROR);
		return;
	}
	else
	{
		if (sound)mciSendString(L"play .\\res\\snd\\exclamation.wav", NULL, NULL, NULL);
		if (MessageBox(bHwnd, L"Настоящата игра ще бъде изгубена !\n\nНаистина ли я презаписваш ?", L"Презапис",
			MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)return;
	}

	if (Grid)delete Grid;
	Grid = new dll::GRID();
	Grid->set_level(level);

	std::wifstream save(save_file);

	save >> level;
	save >> score;
	save >> mins;
	save >> secs;
	save >> level_skipped;

	for (int i = 0; i < 16; ++i)
	{
		int letter = 0;
		save >> letter;
		current_player[i] = static_cast<wchar_t>(letter);
	}
	save >> name_set;
	save >> turn_the_game;

	for (int rows = 0; rows < MAX_ROWS; ++rows)
	{
		for (int cols = 0; cols < MAX_COLS; ++cols)
		{
			int avalue = CLEAR_VALUE;
			save >> avalue;
			Grid->set_value(rows, cols, avalue);
		}
	}

	for (int rows = 0; rows < MAX_ROWS; ++rows)
	{
		for (int cols = 0; cols < MAX_COLS; ++cols)
		{
			Grid->set_value(rows, cols, Grid->get_value(rows, cols));
		}
	}

	save.close();

	if (sound)mciSendString(L"play .\\res\\snd\\save.wav", NULL, NULL, NULL);
	MessageBox(bHwnd, L"Играта е заредена !", L"Зареждане", MB_OK | MB_APPLMODAL | MB_ICONINFORMATION);
}
void ShowHelp()
{
	int result = 0;
	CheckFile(help_file, &result);

	if (result == FILE_NOT_EXIST)
	{
		if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
		MessageBox(bHwnd, L"Липсва помощна информация за играта !", L"Липсва файл !",
			MB_OK | MB_APPLMODAL | MB_ICONERROR);
		return;
	}

	wchar_t help_txt[1000]{ L"\0" };

	std::wifstream help(help_file);
	help >> result;

	for (int i = 0; i < result; ++i)
	{
		int letter = 0;
		help >> letter;

		help_txt[i] = static_cast<wchar_t>(letter);
	}

	help.close();

	Draw->BeginDraw();
	if (statBrush && txtBrush && hgltBrush && inactBrush && b1BckgBrush && b2BckgBrush && b3BckgBrush && nrmFormat && 
		gridBrush)
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
	
		Draw->DrawTextW(help_txt, result, statFormat, D2D1::RectF(10.0f, 100.0f, scr_width, scr_height), gridBrush);
	}
	Draw->EndDraw();

	if (sound)mciSendString(L"play .\\res\\snd\\help.wav", NULL, NULL, NULL);
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
		++secs;
		if (secs > 59)
		{
			secs = 0;
			++mins;
		}
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
			level_skipped = true;
			LevelUp();
			break;

		case mExit:
			SendMessage(hwnd, WM_CLOSE, NULL, NULL);
			break;

		case mSave:
			pause = true;
			SaveGame();
			pause = false;
			break;

		case mLoad:
			pause = true;
			LoadGame();
			pause = false;
			break;

		case mHoF:
			pause = true;
			HallOfFame();
			pause = false;
			break;
		}
		break;

	case WM_LBUTTONDOWN:
		if (HIWORD(lParam) * y_scale <= 50)
		{
			if (LOWORD(lParam) * x_scale >= b1Rect.left && LOWORD(lParam) * x_scale <= b1Rect.right)
			{
				if (name_set)
				{
					if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
					break;
				}
				if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
				if (DialogBox(bIns, MAKEINTRESOURCE(IDD_PLAYER), hwnd, &DlgProc) == IDOK)name_set = true;
				break;
			}
			if (LOWORD(lParam) * x_scale >= b2Rect.left && LOWORD(lParam) * x_scale <= b2Rect.right)
			{
				if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);

				if (sound)
				{
					sound = false;
					PlaySound(NULL, NULL, NULL);
					break;
				}
				else
				{
					sound = true;
					PlaySound(sound_file, NULL, SND_ASYNC|SND_LOOP);
					break;
				}
			}
			if (LOWORD(lParam) * x_scale >= b3Rect.left && LOWORD(lParam) * x_scale <= b3Rect.right)
			{
				if (!show_help)
				{
					pause = true;
					show_help = true;
					ShowHelp();
					break;
				}
				else
				{
					if (sound)mciSendString(L"play .\\res\\snd\\select.wav", NULL, NULL, NULL);
					pause = false;
					show_help = false;
					break;
				}
			}
		}
		else if (!pause)
		{
			float cur_x{ LOWORD(lParam) * x_scale };
			float cur_y{ HIWORD(lParam) * y_scale };

			for (int rows = 0; rows < MAX_ROWS; ++rows)
			{
				for (int cols = 0; cols < MAX_COLS; ++cols)
				{
					FRECT box{ Grid->get_dims(rows,cols) };
					if (cur_x >= box.left && cur_x <= box.right && cur_y >= box.up && cur_y <= box.down)
					{
						int curr_value{ Grid->get_value(rows,cols) };
						if (curr_value == CLEAR_VALUE)curr_value = 1;
						else
						{
							++curr_value;
							if (curr_value > 9)curr_value = 1;
						}
						Grid->set_value(rows, cols, curr_value);
						if (sound)mciSendString(L"play .\\res\\snd\\put.wav", NULL, NULL, NULL);
					}
				}
			}

			for (int rows = 0; rows < MAX_ROWS; ++rows)
			{
				for (int cols = 0; cols < MAX_COLS; ++cols)
				{
					Grid->set_value(rows, cols, Grid->get_value(rows, cols));
				}
			}
		}
		break;

	case WM_RBUTTONDOWN:
		if (!pause)
		{
			float cur_x{ LOWORD(lParam) * x_scale };
			float cur_y{ HIWORD(lParam) * y_scale };

			for (int rows = 0; rows < MAX_ROWS; ++rows)
			{
				for (int cols = 0; cols < MAX_COLS; ++cols)
				{
					FRECT box{ Grid->get_dims(rows,cols) };
					if (cur_x >= box.left && cur_x <= box.right && cur_y >= box.up && cur_y <= box.down)
					Grid->set_value(rows, cols, CLEAR_VALUE);
					if (sound)mciSendString(L"play .\\res\\snd\\put.wav", NULL, NULL, NULL);
				}
			}

			for (int rows = 0; rows < MAX_ROWS; ++rows)
			{
				for (int cols = 0; cols < MAX_COLS; ++cols)
				{
					Grid->set_value(rows, cols, Grid->get_value(rows, cols));
				}
			}
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
				hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Lime), &gridBrush);

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

				bmpRecord = Load(L".\\res\\img\\record.png", Draw);
				if (!bmpRecord)
				{
					LogErr(L"Error loading game record bmp !");
					ErrExit(eD2D);
				}

				bmpFirstRecord = Load(L".\\res\\img\\firstrecord.png", Draw);
				if (!bmpFirstRecord)
				{
					LogErr(L"Error loading game first record bmp !");
					ErrExit(eD2D);
				}

				bmpNoRecord = Load(L".\\res\\img\\norecord.png", Draw);
				if (!bmpNoRecord)
				{
					LogErr(L"Error loading game no record bmp !");
					ErrExit(eD2D);
				}

				bmpLevelUp = Load(L".\\res\\img\\LevelUp.png", Draw);
				if (!bmpLevelUp)
				{
					LogErr(L"Error loading game LevelUp !");
					ErrExit(eD2D);
				}

				bmpVictory = Load(L".\\res\\img\\Victory.png", Draw);
				if (!bmpVictory)
				{
					LogErr(L"Error loading game Victory !");
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
			hr = iWriteFactory->CreateTextFormat(L"Copperplate Gothic", NULL, DWRITE_FONT_WEIGHT_EXTRA_BLACK,
				DWRITE_FONT_STYLE_OBLIQUE, DWRITE_FONT_STRETCH_NORMAL, 16.0f, L"", &statFormat);
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
		if ((bRet = PeekMessage(&bMsg, nullptr, NULL, NULL, PM_REMOVE)) != 0)
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

		int boxes_ok = 0;

		for (int rows = 0; rows < MAX_ROWS; ++rows)
		{
			for (int cols = 0; cols < MAX_COLS; ++cols)
			{
				if (Grid->get_value(rows, cols) != CLEAR_VALUE && Grid->value_ok(rows, cols))++boxes_ok;
				if (boxes_ok == 81)LevelUp();
			}
		}

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
				if (i % 3 == 0)Draw->DrawLine(D2D1::Point2F(5.0f, line_y), D2D1::Point2F(temp_dims.right, line_y), gridBrush,5.0f);
				else Draw->DrawLine(D2D1::Point2F(5.0f, line_y), D2D1::Point2F(temp_dims.right, line_y), gridBrush);
			
				if (i == 8)Draw->DrawLine(D2D1::Point2F(5.0f, temp_dims.down),
					D2D1::Point2F(temp_dims.right, temp_dims.down), gridBrush, 5.0f);
			}

			for (int i = 0; i < MAX_COLS; ++i)
			{
				FRECT temp_dims{ Grid->get_dims(8, i) };
				float line_x = temp_dims.left;
				if (i % 3 == 0)Draw->DrawLine(D2D1::Point2F(line_x, sky + 5.0f), 
					D2D1::Point2F(line_x, temp_dims.down), gridBrush, 5.0f);
				else Draw->DrawLine(D2D1::Point2F(line_x, sky + 5.0f), D2D1::Point2F(line_x, temp_dims.down), gridBrush);
			
				if (i == 8)Draw->DrawLine(D2D1::Point2F(temp_dims.right, sky + 5.0f),
					D2D1::Point2F(temp_dims.right, temp_dims.down), gridBrush, 5.0f);
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

		if (gridBrush && statFormat)
		{
			wchar_t stat_txt[150]{ L"играч: " };
			wchar_t add[5]{L"\0"};
			int size = 0;

			wcscat_s(stat_txt, current_player);

			wsprintf(add, L"%d", level);
			wcscat_s(stat_txt, L", ниво: ");
			wcscat_s(stat_txt, add);

			wsprintf(add, L"%d", score);
			wcscat_s(stat_txt, L", точки: ");
			wcscat_s(stat_txt, add);

			wsprintf(add, L"%d", mins);
			wcscat_s(stat_txt, L", време: ");
			if (mins < 10)wcscat_s(stat_txt, L"0");
			wcscat_s(stat_txt, add);
			wcscat_s(stat_txt, L" : ");
			if (secs < 10)
			{
				wcscat_s(stat_txt, L"0");
			}
			wsprintf(add, L"%d", secs);
			wcscat_s(stat_txt, add);

			for (int i = 0; i < 150; ++i)
			{
				if (stat_txt[i] != '\0')++size;
				else break;
			}

			Draw->DrawTextW(stat_txt, size, statFormat, D2D1::RectF(5.0f, ground + 5.0f, scr_width, scr_height), gridBrush);
		}

		/////////////////////////////////////////////
		
		Draw->EndDraw();
	}

	ClearResources();
	std::remove(tmp_file);

    return (int) bMsg.wParam;
}