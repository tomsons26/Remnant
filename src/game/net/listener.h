#pragma once

#include "always.h"
#include "protocol.h"
#include "sockets.h"


class ListenerClass
{
public:
    ListenerClass();
    virtual ~ListenerClass();
    virtual void Register(ProtocolClass *protocol);
    virtual bool Start_Listening(void *address, int size, int unkn1);
    virtual void Stop_Listening();

    void Create_Window();
    void Destroy_Window();
    bool Open_Socket(unsigned short socket);
    void Close_Socket();

    static long __stdcall Window_Proc(HWND hwnd, unsigned int message, unsigned int wParam, long lParam);

public:
    SOCKET m_Socket;
    ProtocolClass *m_Protocol;
    HWND m_Window;
    int m_IsListening : 1;
};

class ListenerProtocolClass : public ProtocolClass
{
public:
    ListenerProtocolClass();

    virtual void Data_Received() override {}
    virtual void Connected_To_Server(int status) override {}
    virtual void Connection_Requested() override { ++m_ConnectionRequests; }
    virtual void Closed() override {}
    virtual void Name_Resolved() override {}
    virtual ~ListenerProtocolClass() override {}

private:
    int m_ConnectionRequests;
};
