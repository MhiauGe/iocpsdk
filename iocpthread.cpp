#include "iocpthread.h"

static NewConn m_nc = nullptr;
static disConn m_disc = nullptr;
static rData m_rd = nullptr;


#include "app_data.h"

using namespace std;

#pragma comment(lib, "Ws2_32.lib")		// Socket编程需用的动态链接库
#pragma comment(lib, "Kernel32.lib")	// IOCP需要用到的动态链接库


/**
 * 结构体名称：PER_IO_DATA
 * 结构体功能：重叠I/O需要用到的结构体，临时记录IO数据
 **/
const int DataBuffSize  = 2 * 1024;
const int TypeServerClose  = 200;
int FlagSendAll = 1; // 1 发送所有 0 发送某个

typedef struct
{
    OVERLAPPED overlapped;
    WSABUF databuff;
    char buffer[ DataBuffSize ];
    int BufferLen;
    int operationType;
}PER_IO_OPERATEION_DATA, *LPPER_IO_OPERATION_DATA, *LPPER_IO_DATA, PER_IO_DATA;

/**
 * 结构体名称：PER_HANDLE_DATA
 * 结构体存储：记录单个套接字的数据，包括了套接字的变量及套接字的对应的客户端的地址。
 * 结构体作用：当服务器连接上客户端时，信息存储到该结构体中，知道客户端的地址以便于回访。
 **/
typedef struct
{
    SOCKET socket;
    SOCKADDR_STORAGE ClientAddr;
}PER_HANDLE_DATA, *LPPER_HANDLE_DATA;

// 定义全局变量
//const int DefaultPort = 6000;
//QVector < PER_HANDLE_DATA* > clientGroup;
QMap<QString, SOCKET> clients;
//QMap<SOCKET, int> m_clientOvls;

// 记录客户端的向量组

HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);
HANDLE sendMutex = CreateMutex(NULL, FALSE, NULL);
DWORD WINAPI ServerWorkThread(LPVOID CompletionPortID);
DWORD WINAPI ServerSendThread(LPVOID IpParam);

// 开始服务工作线程函数
DWORD WINAPI ServerWorkThread(LPVOID IpParam)
{
    HANDLE CompletionPort = (HANDLE)IpParam;
    DWORD BytesTransferred;
    LPOVERLAPPED IpOverlapped;
    LPPER_HANDLE_DATA PerHandleData = NULL;
    LPPER_IO_DATA PerIoData = NULL;
    DWORD RecvBytes;
    DWORD Flags = 0;
    BOOL bRet = false;

    while(true)
    {
        bRet = GetQueuedCompletionStatus(CompletionPort, &BytesTransferred, (PULONG_PTR)&PerHandleData, (LPOVERLAPPED*)&IpOverlapped, INFINITE);

        if (0 == BytesTransferred)
        {
            char  *temp;
            temp = inet_ntoa(((sockaddr_in*)&PerHandleData->ClientAddr)->sin_addr);
            unsigned short  port;
            port = ((sockaddr_in*)&PerHandleData->ClientAddr)->sin_port;

            // 回调函数 ： 断开
            if (nullptr != m_disc)
            {
                qDebug() << "dis";
                clients.remove(temp);

                // TODO 解析sock_data中的数据
                m_disc(temp, port);
            }
        }

        if(bRet == 0){
            cerr << "GetQueuedCompletionStatus Error: " << GetLastError() << endl;
            return -1;
        }

        PerIoData = (LPPER_IO_DATA)CONTAINING_RECORD(IpOverlapped, PER_IO_DATA, overlapped);

        // 检查在套接字上是否有错误发生
        if(0 == BytesTransferred){
            closesocket(PerHandleData->socket);
            GlobalFree(PerHandleData);
            GlobalFree(PerIoData);
            continue;
        }

        // 开始数据处理，接收来自客户端的数据
        WaitForSingleObject(hMutex,INFINITE);
        //        cout << "A Client says: " << PerIoData->databuff.buf << endl;
        // 回调函数 ： 接收

        char  *temp;
        temp = inet_ntoa(((sockaddr_in*)&PerHandleData->ClientAddr)->sin_addr);
        unsigned short  port;
        port = ((sockaddr_in*)&PerHandleData->ClientAddr)->sin_port;

        if (nullptr != m_rd)
        {
            //            qDebug() << recvByteArray.toHex();
            QByteArray recvByteArray(PerIoData->databuff.buf, BytesTransferred);
            m_rd(recvByteArray, temp, port);
        }

        ReleaseMutex(hMutex);

        // 为下一个重叠调用建立单I/O操作数据
        ZeroMemory(&(PerIoData->overlapped), sizeof(OVERLAPPED)); // 清空内存
        PerIoData->databuff.len = 1024 * 2;
        PerIoData->databuff.buf = PerIoData->buffer;
        PerIoData->operationType = 0;	// read
        WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
    }

    return 0;
}


// 发送信息的线程执行函数
DWORD WINAPI ServerSendThread(LPVOID IpParam)
{
    while(1)
    {
        APP_ALL_DATA->g_SenderMsgSem.acquire();

        if (!APP_ALL_DATA->getSendDataQueueToOne().isEmpty())
        {
            QMap<QString, QByteArray> mapData = APP_ALL_DATA->dequeueDataToOne();

            QString ip = mapData.firstKey();
            QByteArray data = mapData.first();

            if ("all" == ip)
            {
                WaitForSingleObject(hMutex,INFINITE);
                for(auto i : clients.values())
                {
                    send(i, data, data.size(), 0);	// 发送信息
                }
                ReleaseMutex(hMutex);
            }
            else
            {
                WaitForSingleObject(hMutex,INFINITE);
                if (clients.contains(ip))
                {
                    send(clients.find(ip).value(), data, data.size(), 0);	// 发送信息
                }
                ReleaseMutex(hMutex);
            }
        }
    }

    return 0;
}


IOCPThread::IOCPThread()
{
    m_port = 16801;
}

void IOCPThread::run()
{
    //    for (auto i : m_clientSockets)
    //    {
    //        if (!i)
    //        {
    //            delete i;
    //            i = nullptr;
    //        }
    //    }

    //    m_clientSockets.clear();

    // 加载socket动态链接库
    WORD wVersionRequested = MAKEWORD(2, 2); // 请求2.2版本的WinSock库
    WSADATA wsaData;	// 接收Windows Socket的结构信息
    DWORD err = WSAStartup(wVersionRequested, &wsaData);

    if (0 != err){	// 检查套接字库是否申请成功
        cerr << "Request Windows Socket Library Error!\n";
        system("pause");
        return;
    }
    if(LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2){// 检查是否申请了所需版本的套接字库
        WSACleanup();
        cerr << "Request Windows Socket Version 2.2 Error!\n";
        system("pause");
        return;
    }

    // 创建IOCP的内核对象
    /**
     * 需要用到的函数的原型：
     * HANDLE WINAPI CreateIoCompletionPort(
     *    __in   HANDLE FileHandle,		// 已经打开的文件句柄或者空句柄，一般是客户端的句柄
     *    __in   HANDLE ExistingCompletionPort,	// 已经存在的IOCP句柄
     *    __in   ULONG_PTR CompletionKey,	// 完成键，包含了指定I/O完成包的指定文件
     *    __in   DWORD NumberOfConcurrentThreads // 真正并发同时执行最大线程数，一般推介是CPU核心数*2
     * );
     **/
    HANDLE completionPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (NULL == completionPort){	// 创建IO内核对象失败
        cerr << "CreateIoCompletionPort failed. Error:" << GetLastError() << endl;
        system("pause");
        return;
    }

    // 创建IOCP线程--线程里面创建线程池

    // 确定处理器的核心数量
    SYSTEM_INFO mySysInfo;
    GetSystemInfo(&mySysInfo);

    // 基于处理器的核心数量创建线程
    for(DWORD i = 0; i < (mySysInfo.dwNumberOfProcessors * 2); ++i){
        // 创建服务器工作器线程，并将完成端口传递到该线程
        HANDLE ThreadHandle = CreateThread(NULL, 0, ServerWorkThread, completionPort, 0, NULL);
        if(NULL == ThreadHandle){
            cerr << "Create Thread Handle failed. Error:" << GetLastError() << endl;
            system("pause");
            return;
        }
        CloseHandle(ThreadHandle);
    }

    // 建立流式套接字
    SOCKET srvSocket = socket(AF_INET, SOCK_STREAM, 0);

    // 绑定SOCKET到本机
    SOCKADDR_IN srvAddr;
    srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(m_port);
    int bindResult = bind(srvSocket, (SOCKADDR*)&srvAddr, sizeof(SOCKADDR));
    if(SOCKET_ERROR == bindResult){
        cerr << "Bind failed. Error:" << GetLastError() << endl;
        system("pause");
        return;
    }

    // 将SOCKET设置为监听模式
    int listenResult = listen(srvSocket, 10);
    if(SOCKET_ERROR == listenResult){
        cerr << "Listen failed. Error: " << GetLastError() << endl;
        system("pause");
        return;
    }

    // 开始处理IO数据
    //    cout << "本服务器已准备就绪，正在等待客户端的接入...\n";

    // 创建用于发送数据的线程
    HANDLE sendThread = CreateThread(NULL, 0, ServerSendThread, 0, 0, NULL);

    while(true){
        PER_HANDLE_DATA * PerHandleData = NULL;
        SOCKADDR_IN saRemote;
        int RemoteLen;
        SOCKET acceptSocket;

        // 接收连接，并分配完成端，这儿可以用AcceptEx()
        RemoteLen = sizeof(saRemote);
        acceptSocket = accept(srvSocket, (SOCKADDR*)&saRemote, &RemoteLen);
        if(SOCKET_ERROR == acceptSocket){	// 接收客户端失败
            cerr << "Accept Socket Error: " << GetLastError() << endl;
            system("pause");
            return;
        }

        // 创建用来和套接字关联的单句柄数据信息结构
        PerHandleData = (LPPER_HANDLE_DATA)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));	// 在堆中为这个PerHandleData申请指定大小的内存
        PerHandleData -> socket = acceptSocket;
        memcpy (&PerHandleData -> ClientAddr, &saRemote, RemoteLen);

        char  *temp;
        temp = inet_ntoa(((sockaddr_in*)&PerHandleData->ClientAddr)->sin_addr);
        unsigned short  port;
        port = ((sockaddr_in*)&PerHandleData->ClientAddr)->sin_port;

        QDB("clientip:" << temp << "port:" << port << "socket:" << PerHandleData->socket);

        // 如果同一个ip地址再次连接，则不做接收
        if (clients.contains(temp))
        {
            qDebug() << "close socket";
            // 扔掉该accept的scoket，并从新接收
            closesocket(acceptSocket);
            continue;
        }

        clients.insert(temp, PerHandleData->socket);
        QDB("clients inserted" << clients);

        // 回调函数 ： 新连接
        if (nullptr != m_nc)
        {
            // TODO 解析sock_data中的数据
            m_nc(temp, port);
        }

        // 将接受套接字和完成端口关联
        CreateIoCompletionPort((HANDLE)(PerHandleData -> socket), completionPort, (DWORD)PerHandleData, 0);


        // 开始在接受套接字上处理I/O使用重叠I/O机制
        // 在新建的套接字上投递一个或多个异步
        // WSARecv或WSASend请求，这些I/O请求完成后，工作者线程会为I/O请求提供服务
        // 单I/O操作数据(I/O重叠)
        LPPER_IO_OPERATION_DATA PerIoData = NULL;
        PerIoData = (LPPER_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(PER_IO_OPERATEION_DATA));
        ZeroMemory(&(PerIoData -> overlapped), sizeof(OVERLAPPED));
        PerIoData->databuff.len = 1024;
        PerIoData->databuff.buf = PerIoData->buffer;
        PerIoData->operationType = 0;	// read

        DWORD RecvBytes;
        DWORD Flags = 0;


        WSARecv(PerHandleData->socket, &(PerIoData->databuff), 1, &RecvBytes, &Flags, &(PerIoData->overlapped), NULL);
    }

    system("pause");
}


void IOCPThread::NewConnectionArrive(NewConn nc)
{
    m_nc = nc;
}

void IOCPThread::DisConnectionHappen(disConn disc)
{
    m_disc = disc;
}

void IOCPThread::SendDataToAllPenbox(QByteArray data)
{
    FlagSendAll = 1;
    APP_ALL_DATA->enqueueDataToOne(data, "all");
}

void IOCPThread::SendDataToOnePenbox(QByteArray data, QString ip, unsigned short port)
{
    Q_UNUSED(port);
    FlagSendAll = 0;
    APP_ALL_DATA->enqueueDataToOne(data, ip);
}

void IOCPThread::RecvDataToOnePenbox(rData rd)
{
    m_rd = rd;
}

void IOCPThread::setPort(unsigned short port)
{
    m_port = port;
}

unsigned short IOCPThread::getPort()
{
    return m_port;
}

