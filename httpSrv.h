#ifndef HTTP_SRV_H
#define HTTP_SRV_H

#define X_REQUEST_ID "X-Request-ID"

#include "vos.h"
#include "sock.h"
#include "vss.h"

typedef struct { // ������ ������������ ��� ������������ URL (��� SOAP)
    uchar *name; // ����� �������� ����
    uchar *data; // ������������ ������
    vss page; // == Name ��� ��������� ������
    int enabled; // ����� ��������-��������� "�� ����"
    int (*onRequest)(); // �����, ��� ��� - �������, ������� ����������
    } SocketMap;

VS0OBJH(SocketMap);

int httpReady(char *s);
int SocketSendHttp(Socket *sock, vssHttp *req, uchar *data, int len);
vssHttp httpReq(uchar *data, int len); // ���������

typedef struct {
    vss ext,mime;
    } httpMime;

typedef struct {
    SocketPool srv; // ��� ��� ������ (������� ���������)
    char name[14]; int logLevel; logger *log; // Logging
    SocketMap **map; // ������������� URL (�������� � �����������)
    vssHttp req; // ������� ������ �� ��������� - ???
    uchar *index; // ������������ ������ (�������� ��������� - ��� ������� map?)
    uchar *buf; // ��������� ���� (��� ������� ������)
    uchar *mimes;
    httpMime *mime; // ��������� ������ � ���������� - ����������� ����� ��� �������� ������
    Counter readLimit; // Limiter for incoming counts
    SocketPool cli; // ������� - ��� ���������???
    vss defmime; // DefaultMime for a page ???
    int keepAlive; // Disconenct after send???
    time_t created; // When it has beed created -)))
    time_t runTill;
    void *handle; // any user defined handle
    } httpSrv;


VS0OBJH(httpSrv); // ����� ����������� ������� ������� ...
VS0OBJH(httpMime);

httpSrv   *httpSrvCreate(char *ini); // �������� ������� http
SocketMap *httpSrvAddMap(httpSrv *srv, uchar *Name, void *proc, void *Data); // ����� ��� �������
SocketMap *httpSrvAddFS1(httpSrv *srv,uchar *url,uchar *path); // Add One FileMapping
SocketMap *httpSrvAddRedirect(httpSrv *srv,uchar *Name,uchar *Host); // Adds New Mapping for a host
int httpSrvAddFS(httpSrv *srv,uchar * url,...);
Socket *httpSrvListen(httpSrv *srv,int port); // �������� �� ���������

int SocketPrintHttp(Socket *sock, vssHttp *req,uchar *fmt,...) ; // Simple Print Here

int httpReady(char *s); // http - stopper detector

int onHttpFS(Socket *sock, vssHttp *req, SocketMap *map) ; // Default File Processor // ��������� �������� ��������� ��������
int httpGetDirList(vss url,char *dir,uchar **out); // Default Dir Generator

int httpSrvAddMimes(httpSrv *srv,char *mimes); // ���������� ������
httpMime *httpSrvGetMime(httpSrv *srv,vss file); // ����� ������ �����

int httpSrvProcess(httpSrv *srv) ;// Dead loop till the end???

/// -- client declrations

enum {
    httpDisconnect = -1, // server disconnected while run command
    httpTimeOut = -2, // TimeOut while run a command
    };

typedef struct { // ������� ��� ������������ ����������
    int  n; // ���������������� ����� (X-Request-Id)
    int  res_code; // ResultedCode
    int  (*onDone)(); // httpCmd *cmd, vssHttp *res, httpClient *cli); // ����������, ����� ������� �����������
    void  *handle; // ����� ������� ���������, ��������� � ������ ��������
    uchar *data; // ��� - ������ ������������ �����... ������� ������������� -)))
    void  *srcHandle; // ������������ �������������� ��������� - ���������� ����???
    time_t created; // ����� �������� �������. ��� ����� ����-�����???
    struct _httpClient *cli;
    char buf[512]; // ��������� ����������� ��������� ��� ��������� ������???
    } httpCmd;


typedef struct _httpClient { //
    Socket sock; // ��������� ����� ��� ������ ..
    char name[80]; logger *log; int logLevel; // MyLogLevel here
    uchar cs[128],proxy[128]; // Default @connect string@ = user/pass@host:port
    uchar *host,*srv,*page; // ConnectHost & PagePrefix & Authority (Basic64)
    uchar Heads[1024]; // Add heads
    vssHttp res; // ������� �������������� �����
    int num; // ������� ������ (�������� � 1)
    httpCmd *cmd; // ������ ������
    int (*onReply)(); // When server replyies on it
    int (*onDisconnect)(); // When a disconnects from a server
    } httpClient;

VS0OBJH(httpCmd);
VS0OBJH(httpClient);

int httpClientInit(httpClient *cli, uchar *cs, uchar *Proxy) ; // ������ �������...
int httpTestMain(int npar,char **par); // �������� ������
int httpClientRun(httpClient *cli) ; // ������� ��������� ������� httpClient

void httpClientClearCommands(httpClient *cli,int res_code);

httpCmd *httpClientPost(httpClient *cli, uchar *url, uchar *data, int len) ; // Flash a get string???
httpCmd *httpClientGet(httpClient *cli , uchar *url); // ����������� ������ ��������� �������

/// -- zu - microHttp

int MicroHttpMain(int npar,char **par); // ������ �����-����

#endif // HTTP_SRV_H

