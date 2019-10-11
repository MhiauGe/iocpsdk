#include "app_data.h"


#include <QDebug>
Q_GLOBAL_STATIC(APP_DATA, m_app_data)

APP_DATA *APP_DATA::GetInstance()
{
    return  m_app_data;
}

APP_DATA::APP_DATA()
{

}

//void APP_DATA::enqueueData(const QByteArray &strData)
//{
//    mutexE.lock();
//    m_sendDataQueue.enqueue(strData);
//    mutexE.unlock();
//    g_SenderMsgSem.release();
//}

//QByteArray APP_DATA::dequeueData()
//{
//    mutexD.lock();
//    QByteArray data = m_sendDataQueue.dequeue();
//    mutexD.unlock();
//    return data;
//}

//QQueue<QByteArray> APP_DATA::getSendDataQueue()
//{
//    return m_sendDataQueue;
//}

void APP_DATA::enqueueDataToOne(const QByteArray &strData, const QString &ip)
{
    mutexEToOne.lock();
    QMap<QString, QByteArray> data;
    data.insert(ip, strData);
    m_sendDataToOneQueue.enqueue(data);
    mutexEToOne.unlock();
    g_SenderMsgSem.release();
}

QMap<QString, QByteArray> APP_DATA::dequeueDataToOne()
{
    mutexDToOne.lock();
    QMap<QString, QByteArray> data;
    data = m_sendDataToOneQueue.dequeue();
    mutexDToOne.unlock();
    return data;
}

QQueue<QMap<QString, QByteArray>> APP_DATA::getSendDataQueueToOne()
{
    return m_sendDataToOneQueue;
}
