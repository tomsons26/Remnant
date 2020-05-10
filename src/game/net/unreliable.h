#pragma once
#include "protocol.h"
#include "sockets.h"

class UnreliableProtocolClass : public ProtocolClass
{
public:
    UnreliableProtocolClass();

    //all these cause Error! E006: col(34) syntax error; probable cause: missing ';'
    //virtual void Data_Received() override {}
    //virtual void Connected_To_Server(int status) override {}
    //virtual void Connection_Requested() override {}
    //virtual void Closed() override {}
    //virtual void Name_Resolved() override {}
    //virtual ~UnreliableProtocolClass() override {}
};

// is in sole 1.0
class UnreliableCommClass
{
public:
    void Create_Window();
    void Destroy_Window();
    bool Open_Socket(unsigned short port);
    void Close_Socket();

    static long __stdcall Window_Proc(HWND hwnd, unsigned int message, unsigned int wParam, long lParam);

private:
    SOCKET m_Socket;
    ProtocolClass *m_Protocol;
    HWND m_Window;
    int m_Async;
    char m_Hbuf[1024];
    char m_Destination[30];
    short word42E;
    int m_DestAddr;
    char m_DotAddr[16];
    char m_HostName[255];
    short word543;
    int m_Buffer;
    int m_MaxPacketSize;
    char byte54D;
    char gap54E[29];
    short word56B;
    char m_Bitfield;
    char gap56E[3];
};
