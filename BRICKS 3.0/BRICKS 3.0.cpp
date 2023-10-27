#include "framework.h"
#include "BRICKS 3.0.h"
#include "Resource.h"
#include <mmsystem.h>
#include <d2d1.h>
#include <dwrite.h>
#include <ctime>
#include "brickfactory.h"
#include "D2BMPLOADER.h"
#include "ErrH.h"
#include "FCheck.h"
#include <vector>
#include <fstream>

#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "d2d1.lib")
#pragma comment (lib, "dwrite.lib")
#pragma comment (lib, "brickfactory.lib")
#pragma comment (lib, "d2bmploader.lib")
#pragma comment (lib, "errh.lib")
#pragma comment (lib, "fcheck.lib")

#define bWinClassName L"My_next_gen_bricks"
#define scr_width 816  // 800 x 600
#define scr_height 639

#define temp_file ".\\res\\data\\temp.dat"
#define temp_fileL L".\\res\\data\\temp.dat"
#define sound_file L".\\res\\snd\\main.wav"
#define help_file L".\\res\\data\\help.dat"
#define record_file L".\\res\\data\\record.dat"
#define save_file L".\\res\\data\\save.dat"

#define mNew 1001
#define mLvl 1002
#define mExit 1003
#define mSave 1004
#define mLoad 1005
#define mHoF 1006

#define no_record 2001
#define record 2002
#define first_record 2003

/////////////////////////////////////////////

WNDCLASS bWinClass;
HINSTANCE bIns = nullptr;
HWND bHwnd = nullptr;
HCURSOR mainCursor = nullptr;
HCURSOR outCursor = nullptr;
HICON bIcon = nullptr;
HMENU bBar = nullptr;
HMENU bMain = nullptr;
HMENU bStore = nullptr;
HDC PaintDC = nullptr;

MSG bMsg;
BOOL bRet = 0;
POINT cursor_pos = { 0,0 };
PAINTSTRUCT bPaint;

RECT but1R = { 0,0,0,0 };
RECT but2R = { 0,0,0,0 };
RECT but3R = { 0,0,0,0 };

////////////////////////////////////////

bool pause = false;
bool show_help = false;
bool sound = true;
bool set_name = false;
bool in_client = true;

bool b1_hglt = false;
bool b2_hglt = false;
bool b3_hglt = false;

bool game_winner = false;

int client_width = 0;
int client_height = 0;
int score = 0;
int level = 1;
int lifes = 3;
wchar_t current_player[16] = L"PLAYERCHE";
int name_size = 10;
int level_bricks = 72;

//////////////////////////////////////////

ID2D1Factory* iFactory = nullptr;
ID2D1HwndRenderTarget* Draw = nullptr;

IDWriteFactory* iWriteFactory = nullptr;
IDWriteTextFormat* nrmText = nullptr;
IDWriteTextFormat* bigText = nullptr;

ID2D1RadialGradientBrush* FieldBrush = nullptr;
ID2D1RadialGradientBrush* ButBackBrush = nullptr;
ID2D1SolidColorBrush* TextBrush = nullptr;
ID2D1SolidColorBrush* StatusBrush = nullptr;
ID2D1SolidColorBrush* hgltTextBrush = nullptr;
ID2D1SolidColorBrush* inactiveTextBrush = nullptr;

ID2D1Bitmap* bmpBall = nullptr;
ID2D1Bitmap* bmpBigPad = nullptr;
ID2D1Bitmap* bmpPad = nullptr;
ID2D1Bitmap* bmpNet = nullptr;
ID2D1Bitmap* bmpShootPad = nullptr;
ID2D1Bitmap* bmpBullet = nullptr;
ID2D1Bitmap* bmpFireBall[4] = { nullptr,nullptr,nullptr,nullptr };

ID2D1Bitmap* bmpBorder = nullptr;
ID2D1Bitmap* bmpStone = nullptr;
ID2D1Bitmap* bmpNormal = nullptr;
ID2D1Bitmap* bmpGold = nullptr;
ID2D1Bitmap* bmpSilver = nullptr;
ID2D1Bitmap* bmpFall = nullptr;
ID2D1Bitmap* bmpCerr = nullptr;

///////////////////////////////////////////////////////////////////

std::vector<BrickObj> vBricks;
std::vector<BallObj>vBullets;
BrickObj Border[16];
PadObj Net = nullptr;

PadObj Pad = nullptr;
BallObj Ball = nullptr;


///////////////////////////////////////////////////////////////////

void ReleaseCOM()
{
    if (iFactory)iFactory->Release();
    if (Draw)Draw->Release();
    if (iWriteFactory)iWriteFactory->Release();
    if (nrmText)nrmText->Release();
    if (bigText)bigText->Release();

    if (FieldBrush)FieldBrush->Release();
    if (ButBackBrush)ButBackBrush->Release();
    if (TextBrush)TextBrush->Release();
    if (StatusBrush)StatusBrush->Release();
    if (hgltTextBrush)hgltTextBrush->Release();
    if (inactiveTextBrush)inactiveTextBrush->Release();

    if (bmpBall)bmpBall->Release();
    if (bmpBigPad)bmpBigPad->Release();
    if (bmpBorder)bmpBorder->Release();
    if (bmpBullet)bmpBullet->Release();
    if (bmpCerr)bmpCerr->Release();
    if (bmpFall)bmpFall->Release();
    for (int i = 0; i < 4; i++) if (bmpFireBall[i])bmpFireBall[i]->Release();
    if (bmpGold)bmpGold->Release();
    if (bmpNet)bmpNet->Release();
    if (bmpNormal)bmpNormal->Release();
    if (bmpPad)bmpPad->Release();
    if (bmpShootPad)bmpShootPad->Release();
    if (bmpSilver)bmpSilver->Release();
    if (bmpStone)bmpStone->Release();
}
void ErrExit(int which_error)
{
    MessageBeep(MB_ICONERROR);
    MessageBox(NULL, ErrHandle(which_error), L"Критична грешша !", MB_OK | MB_APPLMODAL | MB_ICONERROR);
    std::remove(temp_file);
    ReleaseCOM();
    exit(1);
}

void GameOver()
{
    PlaySound(NULL, NULL, NULL);


    std::remove(temp_file);
    ReleaseCOM();
    bMsg.message = WM_QUIT;
    bMsg.wParam = 0;
}
void InitLevel()
{
    if (Pad)Pad->Release();
    Pad = nullptr;
    Pad = new PAD(static_cast<float>(client_width / 2 - 50), 470.0f, pads::normal);
   

    if (Ball)Ball->Release();
    Ball = nullptr;
    
    if (Net)Net->Release();
    Net = nullptr;

    vBricks.clear();
    vBullets.clear();

    if (level > 1)
    {
        if(sound)mciSendString(L"play .\\res\\snd\\levelup.wav", NULL, NULL, NULL);
        Draw->BeginDraw();
        Draw->FillRectangle(D2D1::RectF(0.0f, 0.0f, (float)(client_width), 50.0f), ButBackBrush);
        Draw->FillRectangle(D2D1::RectF(0.0f, 50.0f, (float)(client_width), (float)(client_height)), FieldBrush);
        if (TextBrush && nrmText)
        {
            Draw->DrawText(L"ИМЕ НА ИГРАЧ", 13, nrmText, D2D1::RectF((float)(but1R.left + 15), 10.0f, (float)(but1R.right),
                50.0f), TextBrush);
            Draw->DrawText(L"ЗВУЦИ ON / OFF", 15, nrmText, D2D1::RectF((float)(but2R.left + 5), 10.0f, (float)(but2R.right),
                50.0f), TextBrush);
            Draw->DrawText(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, D2D1::RectF((float)(but3R.left + 20), 10.0f, (float)(but3R.right),
                50.0f), TextBrush);
        }
        if (TextBrush && bigText)
            Draw->DrawText(L"НИВОТО ПОЧИСТЕНО !", 19, bigText, D2D1::RectF(5.0f, (float)(client_height / 2 - 50),
                (float)(client_width), (float)(client_height)), TextBrush);
        Draw->EndDraw();
        Sleep(3500);
    }
    switch (level)
    {
    case 1:
        level_bricks = 72;
        for (int ty = 0; ty < 4; ty++)
        {
            for (int tx = 0; tx < 18; tx++)
            {
                int ttype = rand() % 4 + 1;
                BrickObj OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), static_cast<bricks>(ttype));
                vBricks.push_back(OneBrick);
            }
        }
        break;

    case 2:
        level_bricks = 68;
        for (int ty = 0; ty < 4; ty++)
        {
            for (int tx = 0; tx < 18; tx++)
            {
                BrickObj OneBrick = nullptr;
                int ttype = rand() % 4 + 1;
                if (ty == 3 && (tx == 3 || tx == 7 || tx == 11 || tx == 15))
                    OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), bricks::stone);
                else OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), static_cast<bricks>(ttype));
                vBricks.push_back(OneBrick);
            }
        }
        break;

    case 3:
        level_bricks = 64;
        for (int ty = 0; ty < 4; ty++)
        {
            for (int tx = 0; tx < 18; tx++)
            {
                BrickObj OneBrick = nullptr;
                int ttype = rand() % 4 + 1;
                if ((ty == 3 && (tx == 3 || tx == 7 || tx == 11 || tx == 15))
                    || (ty == 1 && (tx == 2 || tx == 6 || tx == 10 || tx == 14)))
                    OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), bricks::stone);
                else OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), static_cast<bricks>(ttype));
                vBricks.push_back(OneBrick);
            }
        }
        break;

    case 4:
        level_bricks = 64;
        for (int ty = 0; ty < 4; ty++)
        {
            for (int tx = 0; tx < 18; tx++)
            {
                BrickObj OneBrick = nullptr;
                int ttype = rand() % 4 + 1;
                if ((ty == 3 && (tx == 1 || tx == 4 || tx == 7 || tx == 11))
                    || (ty == 0 && (tx == 5 || tx == 8 || tx == 11 || tx == 14)))
                    OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), bricks::stone);
                else OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), static_cast<bricks>(ttype));
                vBricks.push_back(OneBrick);
            }
        }
        break;

    case 5:
        level_bricks = 60;
        for (int ty = 0; ty < 4; ty++)
        {
            for (int tx = 0; tx < 18; tx++)
            {
                BrickObj OneBrick = nullptr;
                int ttype = rand() % 4 + 1;
                if ((tx == 8 && (ty == 0 || ty == 1 || ty == 2 || ty == 3))
                    || (ty == 1 && (tx == 4 || tx == 5 || tx == 6 || tx == 7 || tx == 8 || tx == 9 || tx == 10 || tx == 11)))
                    OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), bricks::stone);
                else OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), static_cast<bricks>(ttype));
                vBricks.push_back(OneBrick);
            }
        }
        break;

    case 6:
        level_bricks = 58;
        for (int ty = 0; ty < 4; ty++)
        {
            for (int tx = 0; tx < 18; tx++)
            {
                BrickObj OneBrick = nullptr;
                int ttype = rand() % 4 + 1;
                if ((tx == 8 && (ty == 0 || ty == 1 || ty == 2 || ty == 3)) || tx == 0 || tx == 17
                    || (ty == 1 && (tx == 4 || tx == 5 || tx == 6 || tx == 7 || tx == 8 || tx == 9 || tx == 10 || tx == 11)))
                    OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), bricks::stone);
                else OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), static_cast<bricks>(ttype));
                vBricks.push_back(OneBrick);
            }
        }
        break;

    case 7:
        level_bricks = 64;
        for (int ty = 0; ty < 4; ty++)
        {
            for (int tx = 0; tx < 18; tx++)
            {
                BrickObj OneBrick = nullptr;
                int ttype = rand() % 4 + 1;
                if ((ty == 0 && (tx == 1 || tx == 16)) || (ty == 1 && (tx == 3 || tx == 14))
                    || (ty == 2 && (tx == 5 || tx == 12)) || (ty == 3 && (tx == 6 || tx == 10)))
                    OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), bricks::stone);
                else OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), static_cast<bricks>(ttype));
                vBricks.push_back(OneBrick);
            }
        }
        break;

    case 8:
        level_bricks = 60;
        for (int ty = 0; ty < 4; ty++)
        {
            for (int tx = 0; tx < 18; tx++)
            {
                BrickObj OneBrick = nullptr;
                int ttype = rand() % 4 + 1;
                if (tx == 1 || tx == 16 || (ty == 3 && ((tx == 2 || tx == 3 || tx == 14 || tx == 15))))
                    OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), bricks::stone);
                else OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), static_cast<bricks>(ttype));
                vBricks.push_back(OneBrick);
            }
        }
        break;

    case 9:
        level_bricks = 52;
        for (int ty = 0; ty < 4; ty++)
        {
            for (int tx = 0; tx < 18; tx++)
            {
                BrickObj OneBrick = nullptr;
                int ttype = rand() % 4 + 1;
                if (tx == 1 || tx == 4 || tx == 13 || tx == 16 || (ty == 3 && ((tx == 2 || tx == 3 || tx == 14 || tx == 15))))
                    OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), bricks::stone);
                else OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), static_cast<bricks>(ttype));
                vBricks.push_back(OneBrick);
            }
        }
        break;

    case 10:
        level_bricks = 51;
        for (int ty = 0; ty < 4; ty++)
        {
            for (int tx = 0; tx < 18; tx++)
            {
                BrickObj OneBrick = nullptr;
                int ttype = rand() % 4 + 1;
                if (tx == 1 || tx == 16 || (ty == 0 && ((tx == 8 || tx == 9)))
                    || (ty == 1 && ((tx == 7 || tx == 10))) || (ty == 2 && ((tx == 6 || tx == 11)))
                    || (ty == 3 && ((tx == 2 || tx == 3 || tx == 4 || tx == 12 || tx == 13 || tx == 14 || tx == 15))))
                    OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), bricks::stone);
                else OneBrick = iCreateBrick((float)(tx * 40 + 40), (float)(ty * 40 + 90), static_cast<bricks>(ttype));
                vBricks.push_back(OneBrick);
            }
        }
        break;

    default: game_winner = true;
    }

}
void InitGame()
{
    score = 0;
    level = 1;
    lifes = 3;
    wcscpy_s(current_player, L"PLAYERCHE");
    set_name = false;

    if (Pad)Pad->Release();
    Pad = nullptr;

    if (Ball)Ball->Release();
    Ball = nullptr;

    vBricks.clear();
    vBullets.clear();
    
    Pad = new PAD(static_cast<float>(client_width / 2 - 50), 470.0f, pads::normal);

    for (int i = 0; i < 16; i++)
    {
        BrickObj OneBrick = iCreateBrick((float)(i * 50), 550.0f, bricks::border);
        Border[i] = OneBrick;
    }

    if (Net)Net->Release();
    Net = nullptr;
    
    InitLevel();
}


INT_PTR CALLBACK DlgProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_INITDIALOG:
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)bIcon);
        return (INT_PTR)TRUE;
        break;

    case WM_CLOSE:
        EndDialog(hwnd, IDCANCEL);
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        case IDOK:
            name_size = GetDlgItemText(hwnd, IDC_NAME, current_player, 15);
            if (name_size < 1)
            {
                if (sound)MessageBeep(MB_ICONASTERISK);
                MessageBox(bHwnd, L"Ха, ха, ха ! Забрави си името !", L"Забраватор !", MB_OK | MB_APPLMODAL | MB_ICONEXCLAMATION);
                wcscpy_s(current_player, L"PLAYERCHE");
                name_size = 10;
                EndDialog(hwnd, IDCANCEL);
                break;
            }
            set_name = true;
            EndDialog(hwnd, IDOK);
            break;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
LRESULT CALLBACK WinProc(HWND hwnd, UINT ReceivedMsg, WPARAM wParam, LPARAM lParam)
{
    switch (ReceivedMsg)
    {
    case WM_CREATE:
    {
        RECT clr = { 0,0,0,0 };
        GetClientRect(hwnd, &clr);
        client_width = clr.right;
        client_height = clr.bottom;

        but1R.right = 200;
        but1R.bottom = 50;

        but2R.left = 250;
        but2R.right = 450;
        but2R.bottom = 50;

        but3R.left = 500;
        but3R.right = 800;
        but3R.bottom = 50;

        srand((unsigned int)time(0));

        bBar = CreateMenu();
        bMain = CreateMenu();
        bStore = CreateMenu();

        AppendMenu(bBar, MF_POPUP, (UINT_PTR)bMain, L"Основно меню");
        AppendMenu(bBar, MF_POPUP, (UINT_PTR)bStore, L"Меню за данни");

        AppendMenu(bMain, MF_STRING, mNew, L"Нова игра");
        AppendMenu(bMain, MF_STRING, mLvl, L"Следващо ниво");
        AppendMenu(bMain, MF_SEPARATOR, NULL, NULL);
        AppendMenu(bMain, MF_STRING, mExit, L"Изход");

        AppendMenu(bStore, MF_STRING, mSave, L"Запази игра");
        AppendMenu(bStore, MF_STRING, mLoad, L"Зареди игра");
        AppendMenu(bStore, MF_SEPARATOR, NULL, NULL);
        AppendMenu(bStore, MF_STRING, mHoF, L"Зара на славата");

        SetMenu(hwnd, bBar);
        InitGame();
    }
    break;

    case WM_CLOSE:
        pause = true;
        if (sound)MessageBeep(MB_ICONASTERISK);
        if (MessageBox(hwnd, L"Ако излезеш, ще загубиш тази игра !\n\nНаистина ли излизаш ?",
            L"Предупреждение !", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
        {
            pause = false;
            break;
        }
        GameOver();
        break;

    case WM_PAINT:
        PaintDC = BeginPaint(hwnd, &bPaint);
        FillRect(PaintDC, &bPaint.rcPaint, CreateSolidBrush(RGB(100, 100, 100)));
        EndPaint(hwnd, &bPaint);
        break;

    case WM_SETCURSOR:
        GetCursorPos(&cursor_pos);
        ScreenToClient(hwnd, &cursor_pos);
        if (LOWORD(lParam) == HTCLIENT)
        {
            if (!in_client)
            {
                in_client = true;
                pause = false;
            }

            if (cursor_pos.x >= but1R.left && cursor_pos.x <= but1R.right && cursor_pos.y >= but1R.top
                && cursor_pos.y <= but1R.bottom)
            {
                if (!b1_hglt)
                {
                    b1_hglt = true;
                    if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                }
            }
            else if (b1_hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1_hglt = false;
            }

            if (cursor_pos.x >= but2R.left && cursor_pos.x <= but2R.right && cursor_pos.y >= but2R.top
                && cursor_pos.y <= but2R.bottom)
            {
                if (!b2_hglt)
                {
                    b2_hglt = true;
                    if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                }
            }
            else if (b2_hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b2_hglt = false;
            }

            if (cursor_pos.x >= but3R.left && cursor_pos.x <= but3R.right && cursor_pos.y >= but3R.top
                && cursor_pos.y <= but3R.bottom)
            {
                if (!b3_hglt)
                {
                    b3_hglt = true;
                    if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                }
            }
            else if (b3_hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b3_hglt = false;
            }


            if (cursor_pos.y <= 50)SetCursor(outCursor);
            else SetCursor(mainCursor);

            return true;
        }
        else
        {
            if (in_client)
            {
                in_client = false;
                pause = true;
            }

            if (b1_hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b1_hglt = false;
            }

            if (b2_hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b2_hglt = false;
            }

            if (b3_hglt)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
                b3_hglt = false;
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
            if (sound)MessageBeep(MB_ICONASTERISK);
            if (MessageBox(hwnd, L"Ако продължиш, ще загубиш тази игра !\n\nНаистина ли рестартираш ?",
                L"Предупреждение !", MB_YESNO | MB_APPLMODAL | MB_ICONQUESTION) == IDNO)
            {
                pause = false;
                break;
            }
            InitGame();
            pause = false;
            break;

        case mExit:
            SendMessage(hwnd, WM_CLOSE, NULL, NULL);
            break;


        }
        break;

    case WM_LBUTTONDOWN:
        if (b1_hglt)
        {
            if (set_name)
            {
                if (sound)mciSendString(L"play .\\res\\snd\\negative.wav", NULL, NULL, NULL);
                break;
            }
            DialogBox(bIns, MAKEINTRESOURCE(IDD_PLAYER), hwnd, &DlgProc);
            break;
        }

        break;

    case WM_KEYDOWN:
        if (pause)break;
        if (Pad)
        {
            if (LOWORD(wParam == VK_LEFT))
            {
                Pad->Move(dirs::left);
                break;
            }
            if (LOWORD(wParam == VK_RIGHT))
            {
                Pad->Move(dirs::right);
                break;
            }
            if (LOWORD(wParam) == VK_SHIFT)
            {
                if (Pad && Pad->type == pads::shooter)
                {
                    vBullets.push_back(new BALL(Pad->x + 20, Pad->y, balls::bullet));
                    break;
                }
                break;
            }
        }
        break;


    default: return DefWindowProc(hwnd, ReceivedMsg, wParam, lParam);
    }

    return (LRESULT)FALSE;
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    bIns = hInstance;
    if (!bIns)ErrExit(eWindow);

    int result = 0;
    CheckFile(temp_fileL, &result);
    if (result == FILE_EXIST)ErrExit(eStarted);
    else
    {
        std::wofstream temp(temp_file);
        temp << L"Играта работи !";
        temp.close();
    }

    if (GetSystemMetrics(SM_CXSCREEN) < scr_width + 100 || GetSystemMetrics(SM_CYSCREEN) < scr_height + 50)ErrExit(eScreen);

    bIcon = (HICON)LoadImage(NULL, L".\\res\\bicon.ico", IMAGE_ICON, 128, 128, LR_LOADFROMFILE);
    if (!bIcon)ErrExit(eIcon);

    mainCursor = LoadCursorFromFile(L".\\res\\bcursor.ani");
    outCursor = LoadCursorFromFile(L".\\res\\out.ani");
    if (!mainCursor || !outCursor)ErrExit(eCursor);

    ZeroMemory(&bWinClass, sizeof(WNDCLASS));

    bWinClass.lpszClassName = bWinClassName;
    bWinClass.hInstance = bIns;
    bWinClass.lpfnWndProc = &WinProc;
    bWinClass.hbrBackground = CreateSolidBrush(RGB(100, 100, 100));
    bWinClass.hIcon = bIcon;
    bWinClass.hCursor = mainCursor;
    bWinClass.style = CS_DROPSHADOW;

    if (!RegisterClass(&bWinClass))ErrExit(eClass);

    bHwnd = CreateWindow(bWinClassName, L"ТУХЛИЧКИ 3.0", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, 100, 50,
        scr_width, scr_height, NULL, NULL, bIns, NULL);
    if (!bHwnd)ErrExit(eWindow);
    else ShowWindow(bHwnd, SW_SHOWDEFAULT);
    ////////////////////////////////////////////////////////////////////////////////

    HRESULT hr = S_OK;

    D2D1_GRADIENT_STOP gStop[2] = { 0,0,0,0 };
    ID2D1GradientStopCollection* gStopCollection = nullptr;

    //////////////////////////////////////////////////////////////////////////////

    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &iFactory);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"iFactory is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    if (bHwnd)
        hr = iFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(bHwnd, D2D1::SizeU(client_width, client_height)), &Draw);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"Draw hwnd render target is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    gStop[0].position = 0.0f;
    gStop[0].color = D2D1::ColorF(D2D1::ColorF::CornflowerBlue);
    gStop[1].position = 1.0f;
    gStop[1].color = D2D1::ColorF(D2D1::ColorF::DarkBlue);

    Draw->CreateGradientStopCollection(gStop, 2, &gStopCollection);
    if (gStopCollection)
        Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F((float)(client_width / 2),
            (float)(client_height / 2)), D2D1::Point2F(0, 0), (float)(client_width / 2), (float)(client_height / 2)), gStopCollection,
            &FieldBrush);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"FieldBrush is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    gStop[0].position = 0.0f;
    gStop[0].color = D2D1::ColorF(D2D1::ColorF::BlueViolet);
    gStop[1].position = 1.0f;
    gStop[1].color = D2D1::ColorF(D2D1::ColorF::DarkViolet);

    Draw->CreateGradientStopCollection(gStop, 2, &gStopCollection);
    if (gStopCollection)
        Draw->CreateRadialGradientBrush(D2D1::RadialGradientBrushProperties(D2D1::Point2F((float)(client_width / 2), 25.0f),
            D2D1::Point2F(0, 0), (float)(client_width / 2), 25.0f), gStopCollection, &ButBackBrush);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"ButBackBrush is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::AliceBlue), &TextBrush);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"TextBrush is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkGreen), &StatusBrush);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"StatusBrush is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::YellowGreen), &hgltTextBrush);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"hgltTextBrush is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::BurlyWood), &inactiveTextBrush);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"inactiveTextBrush is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }


    /////////////////////////////////////////////////////////////////
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&iWriteFactory));
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"iWriteFactory is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    hr = iWriteFactory->CreateTextFormat(L"COURIER", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_OBLIQUE,
        DWRITE_FONT_STRETCH_EXTRA_EXPANDED, 18.0f, L"", &nrmText);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"nrmText is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    hr = iWriteFactory->CreateTextFormat(L"COURIER", NULL, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_OBLIQUE,
        DWRITE_FONT_STRETCH_EXTRA_EXPANDED, 54.0f, L"", &bigText);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"bigText is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }
    ///////////////////////////////////////////////////////////////////////

    bmpBall = Load(L".\\res\\img\\ball.png", Draw);
    if (bmpBall == nullptr)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"bmpBall is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpBigPad = Load(L".\\res\\img\\big_pad.png", Draw);
    if (bmpBigPad == nullptr)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"bmpBigPad is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpBorder = Load(L".\\res\\img\\border_block.png", Draw);
    if (bmpBorder == nullptr)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"bmpBorder is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpBullet = Load(L".\\res\\img\\bullet.png", Draw);
    if (bmpBullet == nullptr)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"bmpBullet is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpCerr = Load(L".\\res\\img\\cerramic.png", Draw);
    if (bmpCerr == nullptr)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"bmpCerr is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpFall = Load(L".\\res\\img\\fallbrick.png", Draw);
    if (bmpFall == nullptr)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"bmpFall is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    for (int i = 0; i < 4; i++)
    {
        wchar_t path[75] = L".\\res\\img\\fireball\\";
        wchar_t add[5] = L"\0";
        wsprintf(add, L"%d", i);
        wcscat_s(path, add);
        wcscat_s(path, L".png");

        bmpFireBall[i] = Load(path, Draw);
        if (bmpFireBall[i] == nullptr)
        {
            std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
            log << L"bmpFireballBall[" << i << L"] is NULL !" << std::endl;
            log.close();
            ErrExit(eD2D);
        }
    }

    bmpGold = Load(L".\\res\\img\\gold.png", Draw);
    if (bmpGold == nullptr)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"bmpGold is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpNet = Load(L".\\res\\img\\net.png", Draw);
    if (bmpNet == nullptr)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"bmpNet is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpNormal = Load(L".\\res\\img\\normal.png", Draw);
    if (bmpNormal == nullptr)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"bmpNormal is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpPad = Load(L".\\res\\img\\normal_pad.png", Draw);
    if (bmpPad == nullptr)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"bmpPad is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpShootPad = Load(L".\\res\\img\\shoot_pad.png", Draw);
    if (bmpShootPad == nullptr)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"bmpShootPad is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpSilver = Load(L".\\res\\img\\silver.png", Draw);
    if (bmpSilver == nullptr)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"bmpSilver is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }

    bmpStone = Load(L".\\res\\img\\stone.png", Draw);
    if (bmpStone == nullptr)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"bmpStone is NULL !" << std::endl;
        log.close();
        ErrExit(eD2D);
    }
    ///////////////////////////////////////////////////////////////////////

    
    wchar_t logo[28] = L"ТУХЛИЧКИ 3.0\n\ndev. Daniel";
    wchar_t logo_to_show[28] = L"\0";

    for (int i = 0; i < 28; i++)
    {
        mciSendString(L"play .\\res\\snd\\click.wav", NULL, NULL, NULL);
        Draw->BeginDraw();
        Draw->Clear(D2D1::ColorF(D2D1::ColorF::DarkViolet));
        logo_to_show[i] = logo[i];
       
        if (bigText && TextBrush)
            Draw->DrawText(logo_to_show, 28, bigText, D2D1::RectF(150.0f, (float)(client_height / 2 - 50), 800.0f, 500.0f), TextBrush);
        Draw->EndDraw();
        Sleep(200);
    }
    Sleep(3000);

    //MAIN MESSAGE LOOP ***********************************

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
            Draw->FillRectangle(D2D1::RectF(0.0f, 0.0f, (float)(client_width), 50.0f), ButBackBrush);
            Draw->FillRectangle(D2D1::RectF(0.0f, 50.0f, (float)(client_width), (float)(client_height)), FieldBrush);
            if (TextBrush && nrmText)
            {
                Draw->DrawText(L"ИМЕ НА ИГРАЧ", 13, nrmText, D2D1::RectF((float)(but1R.left + 15), 10.0f, (float)(but1R.right),
                    50.0f), TextBrush);
                Draw->DrawText(L"ЗВУЦИ ON / OFF", 15, nrmText, D2D1::RectF((float)(but2R.left + 5), 10.0f, (float)(but2R.right),
                    50.0f), TextBrush);
                Draw->DrawText(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, D2D1::RectF((float)(but3R.left + 20), 10.0f, (float)(but3R.right),
                    50.0f), TextBrush);
            }
            if (TextBrush && bigText)
                Draw->DrawText(L"ПАУЗА", 6, bigText, D2D1::RectF((float)(client_width / 2 - 50), (float)(client_height / 2 - 50),
                    (float)(client_width), (float)(client_height)), TextBrush);
            Draw->EndDraw();
            continue;
        }

        if (Ball)
        {
            if (Pad)
            {
                if (!(Pad->x >= Ball->ex || Pad->ex <= Ball->x || Pad->y >= Ball->ey || Pad->ey <= Ball->y))
                {
                    if (Pad->type == pads::normal)
                    {
                        if (Ball->x >= Pad->x && Ball->x <= Pad->x + 5.0f)
                        {
                            Ball->lambda = 4;
                            Ball->dir = dirs::pad_dir_left;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 5 && Ball->x <= Pad->x + 15.0f)
                        {
                            Ball->lambda = 3;
                            Ball->dir = dirs::pad_dir_left;
                            Ball->Move();
                        }
                        
                        else if (Ball->x > Pad->x + 15 && Ball->x <= Pad->x + 40.0f)
                        {
                            Ball->lambda = 2;
                            Ball->dir = dirs::pad_dir_left;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 40 && Ball->x <= Pad->x + 50.0f)
                        {
                            Ball->lambda = 1;
                            Ball->dir = dirs::pad_dir_center;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 50 && Ball->x <= Pad->x + 75.0f)
                        {
                            Ball->lambda = 2;
                            Ball->dir = dirs::pad_dir_right;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 75 && Ball->x <= Pad->x + 85.0f)
                        {
                            Ball->lambda = 3;
                            Ball->dir = dirs::pad_dir_right;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 85 && Ball->x <= Pad->x + 90.0f)
                        {
                            Ball->lambda = 4;
                            Ball->dir = dirs::pad_dir_right;
                            Ball->Move();
                        }
                    }

                    if (Pad->type == pads::big)
                    {
                        if (Ball->x >= Pad->x && Ball->x <= Pad->x + 10.0f)
                        {
                            Ball->lambda = 4;
                            Ball->dir = dirs::pad_dir_left;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 10 && Ball->x <= Pad->x + 25.0f)
                        {
                            Ball->lambda = 3;
                            Ball->dir = dirs::pad_dir_left;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 25 && Ball->x <= Pad->x + 60.0f)
                        {
                            Ball->lambda = 2;
                            Ball->dir = dirs::pad_dir_left;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 60 && Ball->x <= Pad->x + 90.0f)
                        {
                            Ball->lambda = 1;
                            Ball->dir = dirs::pad_dir_center;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 90 && Ball->x <= Pad->x + 110.0f)
                        {
                            Ball->lambda = 2;
                            Ball->dir = dirs::pad_dir_right;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 110 && Ball->x <= Pad->x + 130.0f)
                        {
                            Ball->lambda = 3;
                            Ball->dir = dirs::pad_dir_right;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 130 && Ball->x <= Pad->x + 150.0f)
                        {
                            Ball->lambda = 4;
                            Ball->dir = dirs::pad_dir_right;
                            Ball->Move();
                        }

                    }

                    if (Pad->type == pads::shooter)
                    {
                        if (Ball->x >= Pad->x && Ball->x <= Pad->x + 10.0f)
                        {
                            Ball->lambda = 4;
                            Ball->dir = dirs::pad_dir_left;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 10 && Ball->x <= Pad->x + 20.0f)
                        {
                            Ball->lambda = 3;
                            Ball->dir = dirs::pad_dir_left;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 20 && Ball->x <= Pad->x + 45.0f)
                        {
                            Ball->lambda = 2;
                            Ball->dir = dirs::pad_dir_left;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 45 && Ball->x <= Pad->x + 55.0f)
                        {
                            Ball->lambda = 1;
                            Ball->dir = dirs::pad_dir_center;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 55 && Ball->x <= Pad->x + 80.0f)
                        {
                            Ball->lambda = 2;
                            Ball->dir = dirs::pad_dir_right;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 80 && Ball->x <= Pad->x + 90.0f)
                        {
                            Ball->lambda = 3;
                            Ball->dir = dirs::pad_dir_right;
                            Ball->Move();
                        }

                        else if (Ball->x > Pad->x + 90 && Ball->x <= Pad->x + 100.0f)
                        {
                            Ball->lambda = 4;
                            Ball->dir = dirs::pad_dir_right;
                            Ball->Move();
                        }
                    }
                }
            }

            if (!Ball->Move())
            {
                Pad->Release();
                Pad = nullptr;
                lifes--;
                Ball->Release();
                Ball = nullptr;
            }
        }
        else
        {
            Pad = new PAD(static_cast<float>(client_width / 2 - 50), 470.0f, pads::normal);
            Ball = new BALL(Pad->x + 20.0f, Pad->y - 20.0f);
        }

        if (!vBricks.empty() && Ball)
        {
            for (std::vector<BrickObj>::iterator brick = vBricks.begin(); brick < vBricks.end(); ++brick)
            {
                if ((*brick)->type != bricks::fall &&
                    !(Ball->x >= (*brick)->ex || Ball->ex <= (*brick)->x || Ball->y >= (*brick)->ey || Ball->ey <= (*brick)->y))
                {
                    if ((*brick)->type != bricks::stone)
                    {
                        if (sound)mciSendString(L"play .\\res\\snd\\clear.wav", NULL, NULL, NULL);
                        if (Ball->type == balls::fire)(*brick)->lifes -= 3;
                        else (*brick)->lifes--;

                        if ((*brick)->lifes <= 0)
                        {
                            score += 10 + level * 2;
                            level_bricks--;
                            if (level_bricks <= 0)
                            {
                                level++;
                                InitLevel();
                                break;
                            }
                            if (rand() % 5 == 2)
                            {
                                BrickObj aFall = iCreateBrick((*brick)->x, (*brick)->y, bricks::fall);
                                vBricks.push_back(aFall);
                                (*brick)->Release();
                                vBricks.erase(brick);
                                
                            }
                            else
                            {
                                (*brick)->Release();
                                vBricks.erase(brick);
                            }
                        }
                    }

                    switch (Ball->dir)
                    {
                        case dirs::up:
                            {
                                int chance = rand() % 3;
                                if (chance == 0)Ball->dir = dirs::down;
                                else if(chance==1)Ball->dir = dirs::down_right;
                                else Ball->dir = dirs::down_left;
                            }
                            break;

                        case dirs::up_right:
                        {
                            int chance = rand() % 3;
                            if (chance == 0)Ball->dir = dirs::down;
                            else if (chance == 1)Ball->dir = dirs::down_right;
                            else Ball->dir = dirs::down_left;
                        }
                        break;

                        case dirs::up_left:
                        {
                            int chance = rand() % 3;
                            if (chance == 0)Ball->dir = dirs::down;
                            else if (chance == 1)Ball->dir = dirs::down_right;
                            else Ball->dir = dirs::down_left;
                        }
                        break;

                        case dirs::pad_dir_right:
                        {
                            int chance = rand() % 3;
                            if (chance == 0)Ball->dir = dirs::down;
                            else if (chance == 1)Ball->dir = dirs::down_right;
                            else Ball->dir = dirs::down_left;
                        }
                        break;

                        case dirs::pad_dir_left:
                        {
                            int chance = rand() % 3;
                            if (chance == 0)Ball->dir = dirs::down;
                            else if (chance == 1)Ball->dir = dirs::down_right;
                            else Ball->dir = dirs::down_left;
                        }
                        break;

                        case dirs::down:
                        {
                            int chance = rand() % 3;
                            if (chance == 0)Ball->dir = dirs::up;
                            else if (chance == 1)Ball->dir = dirs::up_right;
                            else Ball->dir = dirs::up_left;
                        }
                        break;

                        case dirs::down_right:
                        {
                            int chance = rand() % 3;
                            if (chance == 0)Ball->dir = dirs::up;
                            else if (chance == 1)Ball->dir = dirs::up_right;
                            else Ball->dir = dirs::up_left;
                        }
                        break;

                        case dirs::down_left:
                        {
                            int chance = rand() % 3;
                            if (chance == 0)Ball->dir = dirs::up;
                            else if (chance == 1)Ball->dir = dirs::up_right;
                            else Ball->dir = dirs::up_left;
                        }
                        break;
                    }

                    break;
                }
            }
        }

        if (!vBricks.empty())
        {
            for (std::vector<BrickObj>::iterator falling = vBricks.begin(); falling < vBricks.end(); ++falling)
            {
                if ((*falling)->type == bricks::fall)
                {
                    if (!(*falling)->Move())
                    {
                        (*falling)->Release();
                        vBricks.erase(falling);
                        break;
                    }
                    else if(Pad)
                    {
                        if (!(Pad->x >= (*falling)->ex || Pad->ex <= (*falling)->x 
                            || Pad->y >= (*falling)->ey || Pad->ey <= (*falling)->y))
                        {
                            switch (rand() % 7)
                            {
                                case 0:
                                    if (Pad->type == pads::normal)
                                        Pad->Transform(pads::big);
                                    else score += 30;
                                    break;

                                case 1:
                                    if (Pad->type == pads::big)
                                        Pad->Transform(pads::shooter);
                                    else score += 30;
                                    break;

                                case 3:
                                    if (Pad->type == pads::shooter)
                                        Pad->Transform(pads::normal);
                                    else score += 30;
                                    break;

                                case 4:
                                    if (!Net)Net = new PAD(0, 520.0f, pads::net);
                                    else score += 30;
                                    break;

                                case 5:
                                    if (rand() % 3 == 1)lifes++;
                                    else score += 50;
                                    break;

                                case 6:
                                    Ball->Transform();
                                    break;

                                
                            }

                            (*falling)->Release();
                            vBricks.erase(falling);
                            break;
                        }
                    }
                }
            }
        }

        if (Net)
        {
            if (Ball)
            {
                if (!(Net->x >= Ball->ex || Net->ex <= Ball->x || Net->y >= Ball->ey || Net->ey <= Ball->y))
                {
                    switch (rand() % 3)
                    {
                        case 0:
                            Ball->dir = dirs::up;
                            break;

                        case 1:
                            Ball->dir = dirs::up_right;
                            break;

                        case 2:
                            Ball->dir = dirs::up_left;
                            break;
                    }
                }
            }
            
            Net->net_counter--;
            if (Net->net_counter <= 0)
            {
                Net->Release();
                Net = nullptr;
            }
        }

        if (!vBullets.empty())
        {
            
            for (std::vector<BallObj>::iterator bul = vBullets.begin(); bul < vBullets.end(); bul++)
            {
                if (!(*bul)->Move())
                {
                    (*bul)->Release();
                    vBullets.erase(bul);
                    break;
                }
            }
        }

        if (!vBullets.empty() && !vBricks.empty())
        {
            for (std::vector<BrickObj>::iterator brick = vBricks.begin(); brick < vBricks.end(); brick++)
            {
                bool killed = false;

                for (std::vector<BallObj>::iterator bul = vBullets.begin(); bul < vBullets.end(); ++bul)
                {
                    if (!((*bul)->x >= (*brick)->ex || (*bul)->ex <= (*brick)->x
                        || (*bul)->y >= (*brick)->ey || (*bul)->ey <= (*brick)->y))
                    {
                        (*bul)->Release();
                        vBullets.erase(bul);
                        if ((*brick)->type != bricks::stone && (*brick)->type != bricks::fall)
                        {
                            (*brick)->lifes -= 2;

                            if (sound)mciSendString(L"play .\\res\\snd\\clear.wav", NULL, NULL, NULL);
                         
                            if ((*brick)->lifes <= 0)
                            {
                                killed = true;
                                score += 10 + level * 2;
                                level_bricks--;
                                if (level_bricks <= 0)
                                {
                                    level++;
                                    InitLevel();
                                    break;
                                }
                                if (rand() % 5 == 2)
                                {
                                    BrickObj aFall = iCreateBrick((*brick)->x, (*brick)->y, bricks::fall);
                                    vBricks.push_back(aFall);
                                    (*brick)->Release();
                                    vBricks.erase(brick);
                                }
                                else
                                {
                                    (*brick)->Release();
                                    vBricks.erase(brick);
                                }
                            }

                        }

                        break;
                    }
                }
                if (killed)break;
            }
        }


        //DRAW THINGS ************************************************

        Draw->BeginDraw();
        Draw->FillRectangle(D2D1::RectF(0.0f, 0.0f, (float)(client_width), 50.0f), ButBackBrush);
        Draw->FillRectangle(D2D1::RectF(0.0f, 50.0f, (float)(client_width), (float)(client_height)), FieldBrush);
        if (TextBrush && nrmText && hgltTextBrush && inactiveTextBrush)
        {
            if (set_name)
                Draw->DrawText(L"ИМЕ НА ИГРАЧ", 13, nrmText, D2D1::RectF((float)(but1R.left + 15), 10.0f, (float)(but1R.right),
                    50.0f), inactiveTextBrush);
            else if (b1_hglt)
                Draw->DrawText(L"ИМЕ НА ИГРАЧ", 13, nrmText, D2D1::RectF((float)(but1R.left + 15), 10.0f, (float)(but1R.right),
                    50.0f), hgltTextBrush);
            else
                Draw->DrawText(L"ИМЕ НА ИГРАЧ", 13, nrmText, D2D1::RectF((float)(but1R.left + 15), 10.0f, (float)(but1R.right),
                    50.0f), TextBrush);
            if (b2_hglt)
                Draw->DrawText(L"ЗВУЦИ ON / OFF", 15, nrmText, D2D1::RectF((float)(but2R.left + 5), 10.0f, (float)(but2R.right),
                    50.0f), hgltTextBrush);
            else
                Draw->DrawText(L"ЗВУЦИ ON / OFF", 15, nrmText, D2D1::RectF((float)(but2R.left + 5), 10.0f, (float)(but2R.right),
                    50.0f), TextBrush);
            if (b3_hglt)
                Draw->DrawText(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, D2D1::RectF((float)(but3R.left + 20), 10.0f, (float)(but3R.right),
                    50.0f), hgltTextBrush);
            else
                Draw->DrawText(L"ПОМОЩ ЗА ИГРАТА", 16, nrmText, D2D1::RectF((float)(but3R.left + 20), 10.0f, (float)(but3R.right),
                    50.0f), TextBrush);
        }
        
        for (int i = 0; i < 16; i++)
        {
            if (Border[i])Draw->DrawBitmap(bmpBorder, D2D1::RectF(Border[i]->x, Border[i]->y, Border[i]->ex, Border[i]->ey));
        }

        wchar_t status[500] = L"\0";
        wchar_t add[10] = L"\0";
        int status_size = 0;
       
        wcscpy_s(status, current_player);
        
        wcscat_s(status, L", ниво: ");
        wsprintf(add, L"%d", level);
        wcscat_s(status, add);

        wcscat_s(status, L", животи: ");
        wsprintf(add, L"%d", lifes);
        wcscat_s(status, add);

        wcscat_s(status, L", резултат: ");
        wsprintf(add, L"%d", score);
        wcscat_s(status, add);

        if (Net)
        {
            wcscat_s(status, L", мрежа: ");
            wsprintf(add, L"%d", Net->net_counter);
            wcscat_s(status, add);
        }

        for (int i = 0; i < 500; ++i)
        {
            if (status[i] != 0)status_size++;
            else break;
        }
        if (nrmText && TextBrush)
            Draw->DrawText(status, status_size, nrmText, D2D1::RectF(5.0f, 570.0f, (float)(client_width), (float)(client_height)), 
                StatusBrush);

        if (Pad)
        {
            switch (Pad->type)
            {
                case pads::normal:
                    Draw->DrawBitmap(bmpPad, D2D1::RectF(Pad->x, Pad->y, Pad->ex, Pad->ey));
                    break;

                case pads::big:
                    Draw->DrawBitmap(bmpBigPad, D2D1::RectF(Pad->x, Pad->y, Pad->ex, Pad->ey));
                    break;

                case pads::shooter:
                    Draw->DrawBitmap(bmpShootPad, D2D1::RectF(Pad->x, Pad->y, Pad->ex, Pad->ey));
                    break;
            }
        }
        if (Net)
            Draw->DrawBitmap(bmpNet, D2D1::RectF(Net->x, Net->y, Net->ex, Net->ey));

        
        if (Ball)
        {
            switch (Ball->type)
            {
                case balls::normal:
                    Draw->DrawBitmap(bmpBall, D2D1::RectF(Ball->x, Ball->y, Ball->ex, Ball->ey));
                    break;

                case balls::fire:
                    Draw->DrawBitmap(bmpFireBall[Ball->Frame()], D2D1::RectF(Ball->x, Ball->y, Ball->ex, Ball->ey));
                    break;

                case balls::bullet:
                    break;
            }
        }

        if (!vBullets.empty())
        {
            for (int i = 0; i < vBullets.size(); ++i)
                Draw->DrawBitmap(bmpBullet, D2D1::RectF(vBullets[i]->x, vBullets[i]->y, vBullets[i]->ex, vBullets[i]->ey));

        }
        
        
        if (!vBricks.empty())
        {
            for (int i = 0; i < vBricks.size(); i++)
            {
                switch (vBricks[i]->type)
                {
                    case bricks::cerramic:
                        Draw->DrawBitmap(bmpCerr, D2D1::RectF(vBricks[i]->x, vBricks[i]->y, vBricks[i]->ex, vBricks[i]->ey));
                        break;

                    case bricks::fall:
                        Draw->DrawBitmap(bmpFall, D2D1::RectF(vBricks[i]->x, vBricks[i]->y, vBricks[i]->ex, vBricks[i]->ey));
                        break;

                    case bricks::gold:
                        Draw->DrawBitmap(bmpGold, D2D1::RectF(vBricks[i]->x, vBricks[i]->y, vBricks[i]->ex, vBricks[i]->ey));
                        break;

                    case bricks::normal:
                        Draw->DrawBitmap(bmpNormal, D2D1::RectF(vBricks[i]->x, vBricks[i]->y, vBricks[i]->ex, vBricks[i]->ey));
                        break;

                    case bricks::silver:
                        Draw->DrawBitmap(bmpSilver, D2D1::RectF(vBricks[i]->x, vBricks[i]->y, vBricks[i]->ex, vBricks[i]->ey));
                        break;

                    case bricks::stone:
                        Draw->DrawBitmap(bmpStone, D2D1::RectF(vBricks[i]->x, vBricks[i]->y, vBricks[i]->ex, vBricks[i]->ey));
                        break;
                }
            }
        }

        
        
        /////////////////////////////////////////////////
        Draw->EndDraw();



    }

    ///////////////////////////////////////////////////////////////////////
    std::remove(temp_file);
    ReleaseCOM();
    return (int)bMsg.wParam;
}