#ifndef IOCPTHREAD_H
#define IOCPTHREAD_H

#include <QObject>
#include <QThread>
#include <QDebug>
#include <WinSock2.h>
#include <Windows.h>
#include <vector>
#include <iostream>
#include <QDateTime>

#include "iocpsdk_global.h"

#define QDB(msg) qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "[DEBUG]" << __FUNCTION__ << __LINE__ << msg
#define QER(msg) qDebug() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") << "[ERROR]" << __FUNCTION__ << __LINE__ << msg

typedef void(*NewConn)(const QString& ip, const unsigned short& port);
typedef void(*disConn)(const QString& ip, const unsigned short& port);
typedef void(*rData)(QByteArray data , const QString& ip, const unsigned short& port);

class IOCPSDKSHARED_EXPORT IOCPThread : public QThread
{
public:
    IOCPThread();

    void run();

public:
    void NewConnectionArrive(NewConn nc);
    void DisConnectionHappen(disConn disc);
    void SendDataToAllPenbox(QByteArray data);
    void SendDataToOnePenbox(QByteArray data, QString ip, unsigned short port);
    void RecvDataToOnePenbox(rData rd);

    void setPort(unsigned short port);
    unsigned short getPort();

private:
    unsigned short m_port = 0;



//    QMap<SOCKET, PER_HANDLE_DATA*> m_clientSockets;
//    QTimer* timer;

private slots:
//    void timedone();
};

#endif // IOCPTHREAD_H
