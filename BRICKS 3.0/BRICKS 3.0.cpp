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

int client_width = 0;
int client_height = 0;
int score = 0;
int level = 1;
wchar_t current_player[16] = L"PLAYERCHE";
int name_size = 10;

//////////////////////////////////////////

ID2D1Factory* iFactory = nullptr;
ID2D1HwndRenderTarget* Draw = nullptr;

IDWriteFactory* iWriteFactory = nullptr;
IDWriteTextFormat* nrmText = nullptr;
IDWriteTextFormat* bigText = nullptr;

ID2D1RadialGradientBrush* FieldBrush = nullptr;
ID2D1RadialGradientBrush* ButBackBrush = nullptr;
ID2D1SolidColorBrush* TextBrush = nullptr;
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

std::vector<BrickObj> Bricks;

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
void InitGame()
{
    score = 0;
    level = 1;
    wcscpy_s(current_player, L"PLAYERCHE");
    set_name = false;

    if (Pad)Pad->Release();
    Pad = nullptr;

    if (Ball)Ball->Release();
    Ball = nullptr;

    Bricks.clear();

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

    hr = Draw->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DeepSkyBlue), &TextBrush);
    if (hr != S_OK)
    {
        std::wofstream log(L".\\res\\data\\log.dat", std::ios::app);
        log << L"TextBrush is NULL !" << std::endl;
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
        Draw->EndDraw();



    }

    ///////////////////////////////////////////////////////////////////////
    std::remove(temp_file);
    ReleaseCOM();
    return (int)bMsg.wParam;
}