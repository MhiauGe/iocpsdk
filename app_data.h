#ifndef APP_DATA_H
#define APP_DATA_H

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QSemaphore>

#include "iocpsdk_global.h"

class IOCPSDKSHARED_EXPORT APP_DATA
{
public:
    static APP_DATA* GetInstance();

public:
    APP_DATA();

    //    void enqueueData(const QByteArray& strData);
    //    QByteArray dequeueData();
    //    QQueue<QByteArray> getSendDataQueue();
    QSemaphore g_SenderMsgSem;

    void enqueueDataToOne(const QByteArray& strData, const QString &ip);
    QMap<QString, QByteArray> dequeueDataToOne();
    QQueue<QMap<QString, QByteArray>> getSendDataQueueToOne();
    //    QSemaphore g_SenderMsgSemToOne;

private:
//    QQueue<QByteArray> m_sendDataQueue;
//    QMutex mutexE;
//    QMutex mutexD;

    QQueue<QMap<QString, QByteArray>> m_sendDataToOneQueue;
    QMutex mutexEToOne;
    QMutex mutexDToOne;
};

#endif // APP_DATA_H

#define APP_ALL_DATA APP_DATA::GetInstance()
