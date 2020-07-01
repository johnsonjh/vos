#ifndef SOCK_H
#define SOCK_H

#include "vos.h"
#include "vs0.h"
#include "vss.h"
#include "logger.h"
#include "vtypes.h"

//#include "counter.h"

enum {
    sockNone = 0, // ������ ���������. ������ � �������� - ���� ��� -)))
    sockConnected = 1, // �������, ����������� �����...
    sockListen = 2, // ��������� � ������ �������� -)
    sockRead = 3, // � ������� ������ ����� ������ -)
    sockSend = 4, // sending a data -)))
    sockBusy = 5, // Running execution on external thread -)))
    sockKillMe = -8,
    };


enum {
    connectNone = 0, // Socket Not connected
    connectTCP = 1,  // Socket connected via TCP
    connectSSL = 2,  // SSL handShare OK
    connectApp = 3   // App connected OK
    };

typedef struct { // ��� ����� ��������� - �����
    int sock; // Handle -))
    int state; // ������� ��������� ������. ������������� - ���� ����???
    uchar *out,*in; // ���� � ����� - ������, ������ ������� ����� ������� ������ �� ��������
    void *ssl; // ���� ���� ������ - SSL engine - �� ��� ��� ��� -)))
    int N; // ���������������� ����� ����������
    int recvNo,recvBytes; // ����� ����������������� ������� � ���������� �������� ������
    int (*onConnect)(); // ������� - ���������� �� ������������� �������� ...
    int (*onDataSent)();  // When data done
    int (*checkPacket)(); // ����������� ������� ���������� ������ - ��� � ��� ������?
    int (*onDie)(); // ����� ����� ������� (����������)
    int writable,readable; // Transport flags - if we can read or write smth???
    Counter readCounter,writeBytes; // ������� ������ -)))
    Counter *readPacket; // ����� �� ������
    int readPackets,readBytes; // �������� ������ ������� �������
    // -- ����������� ������
    int maxBytes,Bytes0,Bytes1,Time0; // ���� Time0>=now �� ����� ������� maxByte*2
    int connectStatus; // None, TCP, SSL, App
    int connected; // time when it was connected -)))
    int  dieOnSend; // ���� - �������� ���������� ��� ��� ����� ������� ������???
    char szip[16]; // ip address
    //time_t stopReadTill; // ���� ��������� ������ - ��� ����, ���� �� �������� � ����� �� ������...
    time_t modified; // Last read or write time
    void *parent; // This is parent structure
    struct _SocketPool *pool; // if socket in a pool???
    char name[80]; int logLevel; void *log;
    char status[80]; // last line for text messages
    int   code; // status code (assoc)
    int   lineMode; // lineMode '\r\n' or '\r\n.\r\n'
    void *handle; // any use
    void *auth; // authinfo if need to atuhoriztions of session
    } Socket;

uchar *ip2szbuf(int ip,uchar *buf);
uchar *ip2sz(int ip);

VS0OBJH(Socket);
void SocketDone(Socket *sock);


typedef struct _SocketPool {
    Socket **sock; // ��� ������, ������� ������ ����... ����� ��� ���???
    char name[14]; int logLevel; // Name of a system & loglevel of it (for logf...)
    logger *log; // Must Be Here ???
    int reported; // TimeLastReported
    int seqN; // ���������������� ����� ����������
    Socket* (*onAccept)(void *srv,int sock,int ip); // How to process connection ???
    int (*DataIsReady)(); // Check - if data is ready for a socket???
    int (*onDataReady)(); // Check - if we have data enougth ???
    int (*onDataSent)();  // When data done
    int (*onDie)(); // When Socket Die called (socket removed from a pool
    //SocketMap **map; // ������ ������ �����
    int report; // ����-������ � ���������� �������???
    int requests; // Total requests processed
    int connects; // Total connects served
    void *handle; // �� ������������ - ������ ��� ���
    #ifdef VOS_SSL
    char *pem_file;
    #endif
    } SocketPool;

VS0OBJH(SocketPool);

int SocketRun(Socket *sock);
Socket *SocketPoolAccept(SocketPool *srv, int isock, int ip); // ��������� ���������� � ����������� � ���

int socketConnectNow(Socket *Sock, char *host, int defPort) ; // ���������� ������� ������
int SocketSendDataNow(Socket *sock, void *data, int len); // Adds data to out & try send() it now

int SocketSend(Socket *sock,void *data,int len);
int SocketSendf(Socket *sock,char *fmt,...);
int SocketPoolRun(SocketPool *p); // ��������� ����������� ������� - ������ ��� ������� ������
//int socketConnect(Socket *Sock, char *host, int defPort);
int SocketListen(Socket *Sock, int port) ; // ������� ���������...
int SocketRecvHttp(Socket *sock);
int SocketDie(Socket *sock,char *msg); // Notify & close this socket
void Socket2Pool(Socket *sock,SocketPool *srv); // Add To Pool

/// httpClient.c

/// gearConnect

#define GEAR_CALL_TIMEOUT 1*180 /* ������������ ����� ���������� �������� */
#define GEAR_CALL_NOOP    30 /* �������� ������ �� Noop ��� ������� �������� ����� ��� */

enum { rNew,rRun,rReady,rDone,rTimeOut}; // ������� ���������...

typedef struct { // ����������� ������ �� ���������� �������
    int N; // ����� �������...
    int status,code; // ������� ������ ������� � ��� ��������� (httpResponce)
    time_t sent,timeOut; // ����� ����� ������ ������� � ����� ����� �������� ��������� ����������...
    int (*onState)(/*req,connect*/);  void *handle; // ������ ��� ����������� ����������� � �������
    int (*onUI)(/*req,connect*/);  // ����� ��������� ���������� �� UserInteract...
    } gearRequest;

typedef struct { // �������� ������
    Socket sock;
    char name[24];int logLevel; void *log; // logger option
    int  usr; // ���������������� ID - ����������� ����� getMyInfo
    uchar cs[80],Host[80],Auth[80]; // connect string...
    int reqN; gearRequest *req; // ������� ������ � ���������������� ������...
    vss resp,heads,data,status,httpVer; // responce here Response,Heads
    int httpStatus; // last HTTP status
    Counter readPacket; // ��� ����������...
    int toNoop;  // timeOut for send NOOP (keep-alive requests)
    time_t checked,pinged; // �������� ����-����� � ������� ������� �� ������
    uchar userName[80]; // gearUserName
    int eof; // Flag EndOfFile...
    } gearClient;

VS0OBJH(gearRequest);
VS0OBJH(gearClient);

int gearClientRun(gearClient *gc);

int gearClientInit(gearClient *gc, uchar *cs); // �������� ������ � ������������� ���������� TCP �������...
gearRequest *gearClientRequest(gearClient *gc, uchar *url, uchar *data, int len); // ������� ������
gearRequest *gearClientReqByN(gearClient *gc,int reqN);
int gearCallf(gearClient *gc,uchar **data, uchar *fmt,...); // ���������� �����

// some ready functions
int httpReady(char *s);

// extender - opens a connector + pool...
int SocketListener(Socket *sock,int port,
   void *onConnect, void *checkPacket, void *onDie);  // makes a pool & connect it to listener -)))


#endif // SockH
