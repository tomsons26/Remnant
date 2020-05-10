#pragma once
#include "wtypes.h"

class CommBufferClass;


// todo move
HINSTANCE hWSockInstance;

// todo move
#pragma pack(push, 1)
struct DestAddress
{
    char Host[30];
    short Port;
};
#pragma pack(pop)

class ProtocolClass
{
public:
    ProtocolClass() {}

    virtual void Data_Received() = 0;
    virtual void Connected_To_Server(int status) = 0;
    virtual void Connection_Requested() = 0;
    virtual void Closed() = 0;
    virtual void Name_Resolved() = 0;
    virtual ~ProtocolClass() {}

public:
    CommBufferClass *m_Queue;
};
