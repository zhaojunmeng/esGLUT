#include "esGLUT/esGLUT.h"

#ifdef ESGLUT_OS_WIN32

#include "esGLUT_internal.h"

static HCURSOR _curArrow = NULL;
static unsigned char _vkMap[0xFF];
static unsigned char _isDown[0xFF];
static void _InitOSWinResources() {
    static GLboolean _initialized = GL_FALSE;
    if (_initialized) return;
    _initialized = GL_TRUE;
    _curArrow = LoadCursor(NULL,IDC_ARROW);
    memset(_isDown,0,sizeof(_isDown));
    memset(_vkMap,0,sizeof(_vkMap));
    _vkMap[VK_UP] = GLUT_KEY_UP;
    _vkMap[VK_DOWN] = GLUT_KEY_DOWN;
    _vkMap[VK_RIGHT] = GLUT_KEY_RIGHT;
    _vkMap[VK_LEFT] = GLUT_KEY_LEFT;
    _vkMap[VK_INSERT] = GLUT_KEY_INSERT;
    _vkMap[VK_HOME] = GLUT_KEY_HOME;
    _vkMap[VK_END] = GLUT_KEY_END;
    _vkMap[VK_PRIOR] = GLUT_KEY_PAGE_UP;
    _vkMap[VK_NEXT] = GLUT_KEY_PAGE_DOWN;
    _vkMap[VK_F1] = GLUT_KEY_F1;
    _vkMap[VK_F2] = GLUT_KEY_F2;
    _vkMap[VK_F3] = GLUT_KEY_F3;
    _vkMap[VK_F4] = GLUT_KEY_F4;
    _vkMap[VK_F5] = GLUT_KEY_F5;
    _vkMap[VK_F6] = GLUT_KEY_F6;
    _vkMap[VK_F7] = GLUT_KEY_F7;
    _vkMap[VK_F8] = GLUT_KEY_F8;
    _vkMap[VK_F9] = GLUT_KEY_F9;
    _vkMap[VK_F10] = GLUT_KEY_F10;
    _vkMap[VK_F11] = GLUT_KEY_F11;
    _vkMap[VK_F12] = GLUT_KEY_F12;
    _vkMap[VK_NUMLOCK] = GLUT_KEY_NUM_LOCK;
    _vkMap[VK_RSHIFT] = GLUT_KEY_SHIFT_R;
    _vkMap[VK_LSHIFT] = GLUT_KEY_SHIFT_L;
    _vkMap[VK_RCONTROL] = GLUT_KEY_CTRL_R;
    _vkMap[VK_LCONTROL] = GLUT_KEY_CTRL_L;
    _vkMap[VK_RMENU] = GLUT_KEY_ALT_R;
    _vkMap[VK_LMENU] = GLUT_KEY_ALT_L;
}

static LRESULT WINAPI _glutWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
    case WM_CREATE:
        break;

    case WM_SIZE: {
        _glutWindowWidth = LOWORD( lParam );
        _glutWindowHeight = HIWORD( lParam );
        InvalidateRect(_glutHWND, NULL, FALSE);
        if (_callback_glutReshapeFunc) {
            _callback_glutReshapeFunc(_glutWindowWidth, _glutWindowHeight);
        }
        // fallthrough
    }

    case WM_PAINT: {
        if (_callback_glutDisplayFunc)
            _callback_glutDisplayFunc();
        if (_glutHWND)
            ValidateRect(_glutHWND, NULL);
        return TRUE;
    }

    case WM_DESTROY: {
        PostQuitMessage(0);
        return TRUE;
    }

    case WM_SETCURSOR: {
        if (LOWORD(lParam) == 1) { // mouse over client area ?
            SetCursor(_curArrow);
            return TRUE;
        }
        break;
    }

    case WM_KEYDOWN: {
        POINT      point;
        GetCursorPos(&point);
        _isDown[wParam] = TRUE;
        if (_vkMap[wParam]) {
            if (_callback_glutSpecialFunc) {
                _callback_glutSpecialFunc(
                    _vkMap[wParam],
                    (int)point.x,
                    (int)point.y);
            }
        }
        else {
            if (_callback_glutKeyboardFunc) {
                _callback_glutKeyboardFunc(
                    (unsigned char)wParam,
                    (int)point.x,
                    (int)point.y);
            }
        }
        return TRUE;
    }

    case WM_KEYUP: {
        POINT      point;
        GetCursorPos(&point);
        _isDown[wParam] = FALSE;
        if (_vkMap[wParam]) {
            if (_callback_glutSpecialUpFunc) {
                _callback_glutSpecialUpFunc(
                    _vkMap[wParam],
                    (int)point.x,
                    (int)point.y);
            }
        }
        else {
            if (_callback_glutKeyboardUpFunc) {
                _callback_glutKeyboardUpFunc(
                    (unsigned char)wParam,
                    (int)point.x,
                    (int)point.y);
            }
        }
        return TRUE;
    }

    default:
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

GLboolean _glutOSWinCreate(const char* title)
{
    WNDCLASS wndclass = {0};
    DWORD    wStyle   = 0;
    RECT     windowRect;
    HINSTANCE hInstance = GetModuleHandle(NULL);

    _InitOSWinResources();

    wndclass.style         = CS_OWNDC;
    wndclass.lpfnWndProc   = (WNDPROC)_glutWindowProc;
    wndclass.hInstance     = hInstance;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndclass.lpszClassName = TEXT("esGLUT");
    if (!RegisterClass(&wndclass))
        return FALSE;

    wStyle = WS_VISIBLE | WS_POPUP | WS_BORDER | WS_SYSMENU | WS_CAPTION | WS_SIZEBOX;

    // Adjust the window rectangle so that the client area has
    // the correct number of pixels
    windowRect.left = 0;
    windowRect.top = 0;
    windowRect.right = _glutWindowWidth;
    windowRect.bottom = _glutWindowHeight;

    AdjustWindowRect(&windowRect, wStyle, FALSE);

    _glutHWND = CreateWindow(
        TEXT("esGLUT"),
        title,
        wStyle,
        0,
        0,
        windowRect.right - windowRect.left,
        windowRect.bottom - windowRect.top,
        NULL,
        NULL,
        hInstance,
        NULL);
    if (_glutHWND == NULL)
        return GL_FALSE;

    ShowWindow(_glutHWND, TRUE);
    return GL_TRUE;
}

void _glutOSWinLoop()
{
    MSG msg = { 0 };
    int done = 0;
    while (!done) {
        int gotMsg = (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != 0);
        if (gotMsg) {
            if (msg.message == WM_QUIT) {
                done = 1;
            }
            else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else if (_glutHWND) {
            SendMessage(_glutHWND, WM_PAINT, 0, 0);
        }

        // Call update function if registered
        if (_callback_glutIdleFunc)
            _callback_glutIdleFunc();
    }
}

#endif // #ifdef ESGLUT_OS_WIN32
