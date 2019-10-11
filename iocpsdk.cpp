#include "iocpsdk.h"

#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"Mswsock.lib")

Q_GLOBAL_STATIC(Iocpsdk, iocpsdk)

Iocpsdk *Iocpsdk::GetInstance()
{
    return iocpsdk;
}

Iocpsdk::Iocpsdk()
{
    m_pIOCPThread = new IOCPThread;
}

Iocpsdk::~Iocpsdk()
{
    if (nullptr != m_pIOCPThread)
    {
        delete m_pIOCPThread;
        m_pIOCPThread = nullptr;
    }
}

void Iocpsdk::NewConnectionArrive(NewConn nc)
{
    if (nullptr != m_pIOCPThread)
    {
        m_pIOCPThread->NewConnectionArrive(nc);
    }
}

void Iocpsdk::DisConnectionHappen(disConn disc)
{
    if (nullptr != m_pIOCPThread)
    {
        m_pIOCPThread->DisConnectionHappen(disc);
    }
}

void Iocpsdk::SendDataToAllPenbox(QByteArray data)
{
    if (nullptr != m_pIOCPThread)
    {
        m_pIOCPThread->SendDataToAllPenbox(data);
    }
}

void Iocpsdk::SendDataToOnePenbox(QByteArray data, QString ip, unsigned short port)
{
    if (nullptr != m_pIOCPThread)
    {
        m_pIOCPThread->SendDataToOnePenbox(data, ip, port);
    }
}

void Iocpsdk::RecvDataToOnePenbox(rData rd)
{
    if (nullptr != m_pIOCPThread)
    {
        m_pIOCPThread->RecvDataToOnePenbox(rd);
    }
}

void Iocpsdk::StartIOCP()
{
    if (nullptr != m_pIOCPThread)
    {
        m_pIOCPThread->start();
    }
}

void Iocpsdk::StopIOCP()
{
    if (nullptr != m_pIOCPThread)
    {
        m_pIOCPThread->quit();
        m_pIOCPThread->wait();
        //        delete m_pIOCPThread;
        //        m_pIOCPThread = nullptr;
    }
}

void Iocpsdk::setPort(unsigned short port)
{
    if (port  < 0 || port > 65535)
    {
        QER("port is invalid! prot is" << port);
        return;
    }

    if (nullptr != m_pIOCPThread)
    {
        m_pIOCPThread->setPort(port);
    }
}

unsigned short Iocpsdk::getPort()
{
    if (nullptr != m_pIOCPThread)
    {
        return m_pIOCPThread->getPort();
    }
    else
    {
        QER("get port failed! because iocp thread is null!");
        return 0;
    }
}
