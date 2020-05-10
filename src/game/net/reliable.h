#pragma once
#include "commbuff.h"
#include "protocol.h"
#include "sockets.h"

class ListenerClass;

class ReliableProtocolClass : public ProtocolClass
{
public:
    ReliableProtocolClass();

    virtual void Data_Received() override {}
    virtual void Connected_To_Server(int status) override
    {
        if (status == 1) {
            m_Connected = 1;
        } else {
            m_Connected = -1;
        }
    }
    virtual void Connection_Requested() override {}
    virtual void Closed() override { m_Connected = 0; }
    virtual void Name_Resolved() override { m_NameResolved = 1; }
    virtual ~ReliableProtocolClass() override {}

    int m_Connected;
    BOOL m_NameResolved;
};

struct TCPHost
{
    int Addr;
    char DotAddr[16];
    char Name[255];
    short Port;
};

class ReliableCommClass
{
public:
    ReliableCommClass(int maxpacketsize);
    ReliableCommClass(ProtocolClass *protocol, int maxpacketsize);

    virtual ~ReliableCommClass();
    virtual void Register(ProtocolClass *protocol);
    virtual bool Connect(void *, int, int);
    virtual bool Connect(ListenerClass *listener);
    virtual void Disconnect();
    virtual int Send();

    static unsigned short Network_Short(unsigned short);
    static unsigned short Local_Short(unsigned short);
    static unsigned long Network_Long(unsigned long);
    static unsigned long Local_Long(unsigned long);
    void Create_Window();
    void Destroy_Window();
    bool Open_Socket();
    void Close_Socket();
    static long __stdcall Window_Proc(HWND hwnd, unsigned int message, unsigned int wParam, long lParam);

private:
    TCPHost m_Host;
    int m_MaxPacketSize;
    SOCKET m_Socket;
    ProtocolClass *m_Protocol;
    HWND m_Window;
    HANDLE m_Async;
    char m_Hbuf[1024];
    CommBufferClass::SendQueueType *m_SendEntry;
    int m_SendLen;
    char *m_ReceiveBuf;
    int m_ReceiveLen;
    int m_IsConnected : 1;
};
