#ifndef IOCPSDK_H
#define IOCPSDK_H

#include "iocpsdk_global.h"

#include <QObject>

#include "iocpthread.h"

class IOCPSDKSHARED_EXPORT Iocpsdk
{
public:
    static Iocpsdk* GetInstance();

public:
    Iocpsdk();
    ~Iocpsdk();

    void NewConnectionArrive(NewConn nc);
    void DisConnectionHappen(disConn disc);
    void SendDataToAllPenbox(QByteArray data);
    void SendDataToOnePenbox(QByteArray data, QString ip, unsigned short port);
    void RecvDataToOnePenbox(rData rd);

    void StartIOCP();
    void StopIOCP();

    void setPort(unsigned short port);
    unsigned short getPort();

private:
    NewConn m_nc = nullptr;
    disConn m_disc = nullptr;
    rData m_rd = nullptr;

    IOCPThread* m_pIOCPThread;
};

#endif // IOCPSDK_H

#define IOCP_SDK Iocpsdk::GetInstance()

