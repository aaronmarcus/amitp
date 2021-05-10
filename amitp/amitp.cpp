#include "amitp.h"

//windows entry point
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{
    //allocate and show console for debugging messages
    AllocConsole();
    FILE* fDummy;
    freopen_s(&fDummy, "CONIN$", "r", stdin);
    freopen_s(&fDummy, "CONOUT$", "w", stderr);
    freopen_s(&fDummy, "CONOUT$", "w", stdout);
	
    MainWindow CMainWin; //create a main window object

	//try creating the main window, if failed then stop
    if (!CMainWin.Create(L"Main Window", WS_OVERLAPPEDWINDOW))
    {
        return 0;
    }

    ShowWindow(CMainWin.Window(), nCmdShow); //show the window on screen

	//create the renderer - give the width, height, bit depth and pointer to the main window
    Renderer renderer(1280, 720, 10, CMainWin.Window());
    renderer.startRTPReceiver(10001); //start receiving RTP packets on the port

    while (true)
    {
		std::unique_lock<std::mutex> lock(renderer.m); //lock mutex till the end of the function scope
        if (renderer.frameQueue.size() > 1) //if the frame queue has more than one frame then break
        {
            break;
        }
        lock.unlock(); //unlock the mutex
    	
    }
	renderer.startRenderer(); //start rendering the first frame in the queue
    //renderer.stopRTPReceiver();




	
    // Run the window message loop.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return 0;
	
}

//window message handling
LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY: //if the message is destroy, send a quit window message
        PostQuitMessage(0);
        return 0;

    case WM_PAINT: //repaint the window to the specified size and position
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(m_hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
        EndPaint(m_hwnd, &ps);
    }
    return 0;

    default:
        return DefWindowProc(m_hwnd, uMsg, wParam, lParam); //handle any default messages
    }
    return TRUE;
}