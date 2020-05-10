#include "unreliable.h"
#include "protocol.h"
#include "commbuff.h"

UnreliableProtocolClass::UnreliableProtocolClass()
{
    m_Queue = new CommBufferClass(50, 50, 207, 32);
}

extern HINSTANCE hWSockInstance;

void UnreliableCommClass::Create_Window()
{
    WNDCLASSA WndClass;

    WndClass.style = 3;
    WndClass.lpfnWndProc = UnreliableCommClass::Window_Proc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hWSockInstance;
    WndClass.hIcon = LoadIconA(NULL, (LPCSTR)0x7F00);
    WndClass.hCursor = LoadCursorA(NULL, (LPCSTR)0x7F00);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(0);
    WndClass.lpszMenuName = 0;
    WndClass.lpszClassName = "URCommWin";
    RegisterClassA(&WndClass);
    m_Window = CreateWindowExA(0,
        "URCommWin",
        "Data Transmission",
        0x80000,
        0x80000000,
        0x80000000,
        0x80000000,
        0x80000000,
        NULL,
        NULL,
        hWSockInstance,
        NULL);

    ShowWindow(m_Window, 0);

    UpdateWindow(m_Window);

    SetWindowLongA(m_Window, GWL_USERDATA, (long)this);
}

void UnreliableCommClass::Destroy_Window()
{
    if (m_Window) {
        DestroyWindow(m_Window);
        m_Window = 0;
    }
}

bool UnreliableCommClass::Open_Socket(unsigned short port)
{
    m_Socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (m_Socket == -1) {
        return false;
    }

    struct sockaddr_in sockad;
    sockad.sin_family = AF_INET;
    sockad.sin_port = htons(port);
    sockad.sin_addr.S_un.S_addr = htonl(0);

    if (bind(m_Socket, (sockaddr *)&sockad, sizeof(sockad)) == SOCKET_ERROR) {
        Close_Socket();
        return false;
    }

    return true;
}

void UnreliableCommClass::Close_Socket()
{
    short op[2];
    if (m_Socket != -1) {
        op[0] = 0;
        op[1] = 0;
        setsockopt(m_Socket, SOL_SOCKET, SO_LINGER, (const char *)&op, sizeof(op));
        closesocket(m_Socket);
        m_Socket = -1;
    }
}

// todo
long __stdcall UnreliableCommClass::Window_Proc(HWND hwnd, unsigned int message, unsigned int wParam, long lParam)
{
    return 0;
}
