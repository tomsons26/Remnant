#include "listener.h"
#include "commbuff.h"

ListenerClass::ListenerClass()
{
    m_Socket = -1;
    m_Protocol = 0;
    m_Window = 0;
    m_IsListening = false;
    Create_Window();
}

ListenerClass::~ListenerClass()
{
    if (m_IsListening) {
        Stop_Listening();
    }
    Destroy_Window();
}

void ListenerClass::Register(ProtocolClass *protocol)
{
    m_Protocol = protocol;
}

bool ListenerClass::Start_Listening(void *addr, int size, int a4)
{
    DestAddress *dest_addr = reinterpret_cast<DestAddress *>(addr);

    if (m_Protocol == nullptr) {
        return false;
    }

    if (size != sizeof(DestAddress)) {
        return false;
    }

    if (m_IsListening) {
        return false;
    }

    if (a4) {
        return false;
    }

    if (!Open_Socket(dest_addr->Port)) {
        return false;
    }

    if (listen(m_Socket, 3) == -1) {
        Close_Socket();
        return false;
    }

    if (WSAAsyncSelect(m_Socket, m_Window, 1125, 8) == SOCKET_ERROR) {
        Close_Socket();
        return false;
    }

    m_IsListening = true;
    return true;
}

void ListenerClass::Stop_Listening()
{
    if (m_IsListening) {
        WSAAsyncSelect(m_Socket, m_Window, 1125, 0);
        Close_Socket();
        m_IsListening = false;
    }
}

extern HINSTANCE hWSockInstance;

void ListenerClass::Create_Window()
{
    WNDCLASSA WndClass;

    WndClass.style = 3;
    WndClass.lpfnWndProc = ListenerClass::Window_Proc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hWSockInstance;
    WndClass.hIcon = LoadIconA(NULL, (LPCSTR)0x7F00);
    WndClass.hCursor = LoadCursorA(NULL, (LPCSTR)0x7F00);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(0);
    WndClass.lpszMenuName = 0;
    WndClass.lpszClassName = "ListenWin";
    RegisterClassA(&WndClass);
    m_Window = CreateWindowExA(0,
        "ListenWin",
        "Listening for Connections",
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

void ListenerClass::Destroy_Window()
{
    if (m_Window) {
        DestroyWindow(m_Window);
        m_Window = nullptr;
    }
}

bool ListenerClass::Open_Socket(unsigned short sock)
{
    m_Socket = socket(AF_INET, SOCK_STREAM, 0);

    if (m_Socket == -1) {
        return false;
    }

    struct sockaddr_in sockad;
    sockad.sin_family = AF_INET;
    sockad.sin_port = htons(sock);
    sockad.sin_addr.S_un.S_addr = htonl(0);

    if (bind(m_Socket, (sockaddr*)&sockad, sizeof(sockad)) == SOCKET_ERROR) {
        Close_Socket();
        return false;
    }

    return true;
}

void ListenerClass::Close_Socket()
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

long __stdcall ListenerClass::Window_Proc(HWND hwnd, unsigned int message, unsigned int wParam, long lParam)
{
    LRESULT v5; // [sp+4h] [bp-10h]@2

    ListenerClass *v6 = (ListenerClass *)GetWindowLongA(hwnd, -21);
    if (message < 0xF) {
        if (message < 1) {
            return DefWindowProcA((HWND)hwnd, message, wParam, lParam);
        }
        if (message <= 1) {
            v5 = 0;
        } else {
            if (message != 2) {
                return DefWindowProcA((HWND)hwnd, message, wParam, lParam);
            }
            v5 = 0;
        }
    } else if (message <= 0xF) {
        v5 = 0;
    } else {
        if (message < 0x111) {
            return DefWindowProcA((HWND)hwnd, message, wParam, lParam);
        }
        if (message <= 0x111) {
            v5 = 0;
        } else {
            if (message != 1125) {
                return DefWindowProcA((HWND)hwnd, message, wParam, lParam);
            }
            if (!(lParam >> 16)) {
                v6->m_Protocol->Connection_Requested();
            }
            v5 = 0;
        }
    }
    return v5;
}


ListenerProtocolClass::ListenerProtocolClass()
{
    m_ConnectionRequests = 0;

    m_Queue = new CommBufferClass(100, 100, 422, 32);
}
