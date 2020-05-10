#include "reliable.h"
#include "listener.h"

#define GAME_SERVER 2

// temp
int GameToPlay;

ReliableProtocolClass::ReliableProtocolClass()
{
    m_Connected = 0;
    m_NameResolved = 0;

    if (GameToPlay == GAME_SERVER) {
        m_Queue = new CommBufferClass(200, 200, 422, 32);
    } else {
        m_Queue = new CommBufferClass(200, 600, 422, 32);
    }
}

ReliableCommClass::ReliableCommClass(int maxpacketsize)
{
    m_Socket = -1;
    m_Protocol = 0;
    m_Window = 0;
    m_Async = 0;
    m_Host.Addr = -1;
    m_Host.DotAddr[0] = 0;
    m_Host.Name[0] = 0;
    m_Host.Port = 0;
    m_SendEntry = 0;
    m_SendLen = 0;
    m_ReceiveBuf = 0;
    m_ReceiveLen = 0;
    m_IsConnected = false;
    m_MaxPacketSize = maxpacketsize;
    m_ReceiveBuf = new char[maxpacketsize];
    Create_Window();
}

ReliableCommClass::ReliableCommClass(ProtocolClass *protocol, int maxpacketsize)
{
    m_Socket = -1;
    m_Protocol = 0;
    m_Window = 0;
    m_Async = 0;
    m_Host.Addr = -1;
    m_Host.DotAddr[0] = 0;
    m_Host.Name[0] = 0;
    m_Host.Port = 0;
    m_SendEntry = 0;
    m_SendLen = 0;
    m_ReceiveBuf = 0;
    m_ReceiveLen = 0;
    m_IsConnected = false;
    m_MaxPacketSize = maxpacketsize;
    m_ReceiveBuf = new char[maxpacketsize];

    Create_Window();

    Register(protocol);
}

ReliableCommClass::~ReliableCommClass()
{
    if (m_IsConnected) {
        Disconnect();
    }

    Destroy_Window();
    delete[] m_ReceiveBuf;
}

void ReliableCommClass::Register(ProtocolClass *protocol)
{
    m_Protocol = protocol;
}

bool ReliableCommClass::Connect(void *addr, int len, int type)
{
    DestAddress *_addr = (DestAddress *)addr;

    if (!m_Protocol == nullptr) {
        return false;
    }

    if (len != sizeof(DestAddress)) {
        return false;
    }

    if (m_IsConnected & 1) {
        Disconnect();
    }

    if (!Open_Socket()) {
        return false;
    }

    if (WSAAsyncSelect(m_Socket, m_Window, 1127, 51) == SOCKET_ERROR) {
        Close_Socket();
        return false;
    }

    if (type == 0) // Sole 1.0 has this as UnreliableCommClass::Resolve_Address
    {
        m_Host.Port = _addr->Port;
        m_Host.Addr = inet_addr((char *)_addr);
        if (m_Host.Addr != -1) {
            strcpy(m_Host.DotAddr, (char *)_addr);
            m_Host.Name[0] = 0;
            m_Async = WSAAsyncGetHostByAddr(m_Window, 1125, (char *)this, 4, 2, m_Hbuf, 1024);
            if (!m_Async) {
                WSAAsyncSelect(m_Socket, m_Window, 1127, 0);
                Close_Socket();
                return false;
            }

            sockaddr_in name;
            name.sin_family = 2;
            name.sin_port = htons(m_Host.Port);
            name.sin_addr.S_un.S_addr = m_Host.Addr;
            if (connect(m_Socket, (sockaddr *)&name, 16) == SOCKET_ERROR && WSAGetLastError() != WSAEWOULDBLOCK) {
                WSACancelAsyncRequest(m_Async);
                m_Async = 0;
                WSAAsyncSelect(m_Socket, m_Window, 1127, 0);
                Close_Socket();
                return false;
            }
        } else {
            strcpy(m_Host.Name, _addr->Host);
            m_Async = WSAAsyncGetHostByName(m_Window, 1126, m_Host.Name, m_Hbuf, 1024);

            if (!m_Async) {
                WSAAsyncSelect(m_Socket, m_Window, 1127, 0);
                Close_Socket();
                return false;
            }
        }

        m_IsConnected = true;
        return true;
    }

    WSAAsyncSelect(m_Socket, m_Window, 1127, 0);
    Close_Socket();
    return false;
}

bool ReliableCommClass::Connect(ListenerClass *listener)
{
    if (m_Protocol == nullptr) {
        return false;
    }

    if (m_IsConnected & 1) {
        Disconnect();
    }

    int addrlen = 16;
    sockaddr_in addr;
    m_Socket = accept(listener->m_Socket, (sockaddr *)&addr, &addrlen);

    if (m_Socket == -1) {
        return false;
    }

    if (WSAAsyncSelect(m_Socket, m_Window, 1127, 51) == SOCKET_ERROR) {
        Close_Socket();
        return false;
    }

    m_Host.Addr = addr.sin_addr.S_un.S_addr;
    strcpy(m_Host.DotAddr, inet_ntoa(addr.sin_addr));
    m_Host.Port = ntohs(addr.sin_port);
    m_Host.Name[0] = 0;

    m_Async = WSAAsyncGetHostByAddr(m_Window, 1125, (char*)this, 4, 2, m_Hbuf, 1024);

    if (!m_Async) {
        WSAAsyncSelect(m_Socket, m_Window, 1127, 0);
        Close_Socket();
        return false;
    }

    m_IsConnected = true;
    return true;
}

void ReliableCommClass::Disconnect()
{
    if (m_IsConnected) {
        if (m_Async) {
            WSACancelAsyncRequest(m_Async);
            m_Async = 0;
        }

        WSAAsyncSelect(m_Socket, m_Window, 1127, 0);
        Close_Socket();
        m_Host.Name[0] = 0;
        m_Host.DotAddr[0] = 0;
        m_Host.Addr = -1;
        m_Host.Port = 0;
        m_SendEntry = 0;
        m_SendLen = 0;
        m_IsConnected = false;
    }
}

int ReliableCommClass::Send()
{
    u_short *v1;

    if (m_Protocol == nullptr || !m_IsConnected) {
        return false;
    }

    if (m_SendEntry != nullptr) {
        return true;
    }

    m_SendEntry = m_Protocol->m_Queue->Get_Send(0);

    if (m_SendEntry) {
        m_SendLen = m_SendEntry->m_BufLen;
        v1 = (u_short *)m_SendEntry->m_Buffer;
        *v1 = htons(m_SendEntry->m_BufLen);
        PostMessageA(m_Window, 1127, 0, 2);
        return true;
    }

    return false;
}

unsigned short ReliableCommClass::Network_Short(unsigned short local_val)
{
    return htons(local_val);
}

unsigned short ReliableCommClass::Local_Short(unsigned short net_val)
{
    return ntohs(net_val);
}

unsigned long ReliableCommClass::Network_Long(unsigned long local_val)
{
    return htonl(local_val);
}

unsigned long ReliableCommClass::Local_Long(unsigned long net_val)
{
    return ntohl(net_val);
}

extern HINSTANCE hWSockInstance;

void ReliableCommClass::Create_Window()
{
    WNDCLASSA WndClass;

    WndClass.style = 3;
    WndClass.lpfnWndProc = ReliableCommClass::Window_Proc;
    WndClass.cbClsExtra = 0;
    WndClass.cbWndExtra = 0;
    WndClass.hInstance = hWSockInstance;
    WndClass.hIcon = LoadIconA(NULL, (LPCSTR)0x7F00);
    WndClass.hCursor = LoadCursorA(NULL, (LPCSTR)0x7F00);
    WndClass.hbrBackground = (HBRUSH)GetStockObject(0);
    WndClass.lpszMenuName = 0;
    WndClass.lpszClassName = "RCommWin";
    RegisterClassA(&WndClass);
    m_Window = CreateWindowExA(0,
        "RCommWin",
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

void ReliableCommClass::Destroy_Window()
{
    if (m_Window) {
        DestroyWindow(m_Window);
        m_Window = 0;
    }
}

bool ReliableCommClass::Open_Socket()
{
    m_Socket = socket(AF_INET, SOCK_STREAM, 0);

    if (m_Socket == -1) {
        return false;
    }

    struct sockaddr_in sockad;
    sockad.sin_family = AF_INET;
    sockad.sin_port = 0;
    sockad.sin_addr.S_un.S_addr = htonl(0);

    if (bind(m_Socket, (sockaddr *)&sockad, sizeof(sockad)) == SOCKET_ERROR) {
        Close_Socket();
        return false;
    }

    return true;
}

void ReliableCommClass::Close_Socket()
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
static long __stdcall ReliableCommClass::Window_Proc(HWND hwnd, unsigned int message, unsigned int wParam, long lParam)
{
    return 0;
}
