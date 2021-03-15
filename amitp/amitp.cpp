#include "amitp.h"

//windows entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    //allocate console for debugging
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
	
    MainWindow CMainWin;

    if (!CMainWin.Create(L"Main Window", WS_OVERLAPPEDWINDOW))
    {
        return 0;
    }

    ShowWindow(CMainWin.Window(), nCmdShow);
	
	//TODO run the renderer in a new thread
    Renderer renderer(1280, 720, 10, CMainWin.Window());
    renderer.startRTPReceiver(20000);
    std::this_thread::sleep_for(std::chrono::seconds(7));
    renderer.startRenderer();
    
    //renderer.stopRTPReceiver();




	
    // Run the message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
	
}

//message handling
LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(m_hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(m_hwnd, &ps);
    }
    return 0;

    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
    }
    return TRUE;
}