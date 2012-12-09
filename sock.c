#include "sock.h"

#define MAX_INT 0x7FFFFFFF

int counterValue(Counter *c) {
if (!c) return 0;
return (c->pValue+c->ppValue)/2;
}

int counterLimit(Counter *c, int now) {
int sz;
if (!c) return MAX_INT; // OK
if (c->Modified<now) {
    if (c->Modified==now-1) c->ppValue = c->pValue;  else c->ppValue=0;
    c->pValue = c->Value; c->Value = 0;
    sz = c->Limit*2 - (c->Value+c->pValue); // ������� ����� ������� -)))
    c->Modified = now;
    } else sz = c->Limit*2 - (c->Value+c->pValue); // ������� ����� ������� -)))
if (!c->Limit) return MAX_INT; // OK anyway ???
return sz; // �� �� ������� -)))
}

void counterAdd(Counter *c,int sz) { // ����������� ������� �������� � ����� �������
if (!c) return;
c->Total+=sz; c->Value+=sz;
}

// ����� ��� - ���� ����??? ???

void SocketDone(Socket *sock) { //
if (sock->sock>0) { // ���� ���� ���������� �����
         //printf("SocketDone, close %d handle here...\n",sock->sock);
         send(sock->sock,(void*)&sock,0,MSG_NOSIGNAL); // send ZERO byte - EndOfConnect
         sock_close(sock->sock); sock->sock = 0; sock->state = sockNone;
         }  // ������� ����������
strClear(&sock->in); strClear(&sock->out); // ������� ������
}


void SocketPoolDone(SocketPool *p) {
    printf("Done nothing with socket pool\n");

}


VS0OBJ0(Socket,SocketDone); // ����������� ��� ������
VS0OBJ0(SocketPool,SocketPoolDone); // ����������� ��� ������


/*
 ���������� - �� ���� ������� ������� ��� ����� ����� - ��� ��� � ��� - ��� �� ��� ��������, � ��� ��� ???
*/

int socketReadPush0(Socket *sock,uchar *buf,int len) { // Try push it now ->>>
SocketPool *srv = sock->pool;
int (*fun)();
if (buf) strCat(&sock->in,buf,len); // ok - added -- now - check for end of data?
sock->recvBytes+=len;// Add a counter
fun = sock->checkPacket;
if (!fun) return 0; // Do Nothing
len = fun(sock->in,strLength(sock->in),sock); // Do We???
if (len<0) { // Protocol Error
         //printf("checkPacket :len=%d in_data=%d sock.h=%d sock.n=%d\n",len,strLength(sock->in),
            //sock->sock,sock->N);
         //hex_dump("DATA:",sock->in,strLength(sock->in));
         SocketDie(sock,"protocolError (checkPacket)");
         //printf("DieDone\n");
         return 0;
         }
if (len==0) return 0; // not yet
if (srv) srv->requests++; // New One
if (sock->readPacket) counterAdd(sock->readPacket,1);
        sock->state = sockConnected; // Remove a @read from a here -)))
        strDelete(&sock->in,0,len); // Remove from a source
        sock->recvNo++;
return 1; // do nothing
}

int socketReadPush(Socket *sock,uchar *buf,int len) { // Try push it now ->>>
if (socketReadPush0(sock,buf,len)<=0) return 0;
while(socketReadPush0(sock,0,0)>0); // Call All Packets
return 1;
}

int SocketDie(Socket *sock,char *msg) {
SocketPool *srv = sock->pool;
//printf("die msg=%s\n",msg);
if (sock->onDie) sock->onDie(sock,msg); // Alert it
sock->state = sockNone; sock->connectStatus = connectNone; sock->modified = TimeNow;
if (srv) { // Unreg from a pool -> some spec destructors ???
    CLOG(srv,2,"Socket#%d handle:%d, (%s) die, msg:%s\n",sock->N,sock->sock,sock->szip,msg);
    //if (srv->onDie) srv->onDie(sock); // Call on close
    sock->pool = 0;
    //printf("Remove!!?? from pool=%p ref=%d?\n",srv,obj2head(sock)->ref);
    vecRemove((void***)&srv->sock,sock); // Remove from an queue
    //printf("Remove!\n");
    }
//printf("done die\n");
return 1;
}

uchar *ip2szbuf(int ip,uchar *buf) {
uchar *i=(void*)&ip;
sprintf((char*)buf,"%d.%d.%d.%d",i[0],i[1],i[2],i[3]);
return buf;
}

uchar *ip2sz(int ip) {
static unsigned char buf[80];
return ip2szbuf(ip,buf);
}


Socket *SocketPoolAccept(SocketPool *srv, int isock, int ip) {
Socket *sock;
sock = SocketNew();
sock->sock = isock;
sock->N = ++srv->connects; // first is 1
sock->state = sockConnected;
sock->connectStatus = connectTCP;
sock->modified = TimeNow;
ip2szbuf(ip,sock->szip);
CLOG(srv,6,"SocketPoolAccept:{n:%d,ip:'%s',sock:%d}\n",sock->N,sock->szip,sock->sock);
Socket2Pool(sock,srv); // AddIt
return sock;
}

int SocketRun(Socket *sock) {
int cnt =0;
SocketPool *srv = sock->pool;
    sock->readable = sock_readable(sock->sock);
    sock->writable = sock_writable(sock->sock);

    //sock->readable=1; // ZU!

    //printf("\nSock: %d R:%d W:%d listen:%d stte:%d\n",sock->sock,sock->readable,sock->writable,sockListen,sock->state);
/*    if (0) if (sock->stopReadTill && sock->stopReadTill>TimeNow) { // Resume reading
        sock->stopReadTill = 0;
        socketReadPush(sock,0,0);
        }*/
    //printf("sock:%d, readable:%d writable:%d\n",sock->sock,sock->readable,sock->writable);
    //printf("readable:%d listen:%d\n",sock->readable>0,sock->state==sockListen);
    if ( (sock->readable>0) && (sock->state == sockListen) ) { // New connection here... how to accept???
        int s,ip; int (*onConnect)();
           //printf("Accepting a new connection on socket %d\n",sock->sock);
        s = sock_accept(sock->sock,&ip);
           //printf("Acceped %d\n",sock);
           if (s<0) return 0; // Fail?
        sock->modified = TimeNow;
        cnt++;
        //printf("Accepted %d socket for listenSocket=%x\n",s,sock);
        // call on connect -- ���� �� ���������� ���� �������???
        onConnect = sock->onConnect;
        if (onConnect && onConnect(sock,s,ip)) { // ok - new connection handled & aded to a pool???
            //if (srv) srv->connects++;
            sock->readPackets++; // IncIt
            //printf("OK, connection handled, proceed...\n");
            } else {
                //SocketClosed(sock,sp,"accept error");
                printf("TCP/sock:decline connection...\n"); // ZU - Never see it???
                send(s,(void*)&s,0,MSG_NOSIGNAL); // Send disconnect...
                sock_close(s);
                }
        }
    else if (sock->readable) { // ZU? && sock->state == sockRead) { // Can read...the end ???
        char buf[8*1024];  int len=sizeof(buf);
        //printf("TryRead socket %d\n",sock->sock);
        if //(sock->readPacket.Limit &&
             (counterLimit(sock->readPacket,TimeNow)<=0) {
            len = -1; // Cant Do That !!!
            //printf("PacketLimit failure!!!\n");
            }
        if (len>0 && sock->readCounter.Limit) { // ��������� - ���� �������������???
            len = counterLimit(&sock->readCounter,TimeNow);
            //printf("LimitedTo:%d\n",len);
            }
        //printf("After limiter %d\n",len);//sock->sock);
        if (len>0) { // ����� ������ ���� - ����
        if (len>sizeof(buf)) len = sizeof(buf); // �� �� ����� �������� ������
        len = recv(sock->sock,buf,len,0); // Try it
        cnt++;
        sock->modified = TimeNow;
        if (len==0) SocketDie(sock,"client close connection");
        else if (len<0) SocketDie(sock,"read error");
            else { // OK!
            //printf("recv %d bytes\n", len);
            counterAdd(&sock->readCounter,len);
            socketReadPush(sock,buf,len); // Try push it now ->>>
            //printf("done push\n");
            //counterAdd(&sock->readCounter,len);
            }
           }
        }
    else if (sock->writable && strLength(sock->out)>0) { // && sock->state == sockSend) {
        int len,maxSz;
        len = strLength(sock->out); maxSz = len;
        if (sock->writeBytes.Limit) maxSz = counterLimit(&sock->writeBytes,TimeNow);
        //printf("WR, SZ=%d\n",maxSz);
        if (len>0 && maxSz>0) {
        len = send(sock->sock,sock->out,maxSz,MSG_NOSIGNAL);
        sock->modified = TimeNow;
        cnt++;
        //printf("len=%d\n",len);
        if (len<=0) {
            SocketDie(sock,"write error");
            } else {
            counterAdd(&sock->writeBytes,len); // Add a counter
            strDelete(&sock->out,0,len);
            }
        //printf("!!!\n");
        }
        len = strLength(sock->out);
        if (len==0) {
            sock->state = sockConnected;
            if (sock->onDataSent) sock->onDataSent(sock,srv);
             else if (srv && srv->onDataSent) srv->onDataSent(sock,srv); // Notify
            if (sock->dieOnSend) SocketDie(sock,"dieOnSend");
            }
        //printf("2!!\n");
        }
return cnt;
}

void Socket2Pool(Socket *sock,SocketPool *srv) { // Add To Pool
SocketPush(&srv->sock,sock); // Register it in a pool
sock->pool = srv; // Remember parent
SocketClear(&sock); // Already in a pool
}

int SocketPoolRun(SocketPool *srv) {
int cnt = 0,i; //Socket *sock;
SocketPool *sp = srv;
CLOG(srv,7,"SocketPool - run %d sockets",arrLength(sp->sock));
//printf("SocketPool - run %d sockets",arrLength(sp->sock));
//rep = srv->report && (now!=sp->reported);sp->reported=now;
for(i=0;i<arrLength(sp->sock);i++) cnt+=SocketRun(sp->sock[i]);
for(i=0;i<arrLength(sp->sock);i++) {
    Socket *sock = sp->sock[i];
    if (sock->state == sockKillMe) {
        CLOG(srv,5,"SocketPool - kill a socket handle %d",sock->sock);
        vecRemove((void***)&sp->sock,sock); // Remove from an queue
        break;
        }
    } //cnt+=SocketRun(sp->sock[i]);
return cnt;
}

//sock  *sockNew(); // ������� ����� (���� ���� -))) - ���  �� ��� ���������***

int SocketListen(Socket *Sock, int port) { // ������� ���������...
int sock;
sock = sock_listen(port,100); // Max 100 listerens
if (sock<=0) return sock; // Failed
sock_async(sock); // make it async???
SocketDone(Sock); // if we have -- )))
Sock->sock = sock;
Sock->state = sockListen; // Fix it
return 1; // OK - listen here!!!
}

int SocketSend(Socket *sock,void *data,int len) {
if (sock->state<=0) return 0; // Fail
if (!data) return 0;
if (len<0) len = strlen(data);
if (len<=0) return 0; // Do not send zero -)))
strCat(&sock->out,data,len); // Push it & Forget???
return len;
}

int SocketSendf(Socket *sock,uchar *fmt,...) {
char buf[1024];
BUF_FMT(buf,fmt);
return SocketSend(sock,buf,-1);
}

int SocketSendDataNow(Socket *sock, void *data, int len) {
SocketSend(sock,data,len);
len = send(sock->sock,sock->out,strLength(sock->out),MSG_NOSIGNAL); // Send Sync...
if (len<0) {
    SocketDone(sock); return 0; //Failed
    }
strDelete(&sock->out,0,len); // Delete Me ???
return 1; // OK ---
}

int socketConnectNow(Socket *Sock, char *host, int defPort) { // ���������� ������� ������
int sock;
SocketDone(Sock); // If we have smth before
sock=socket(AF_INET,SOCK_STREAM,0);
if (sock<0) return 0; // Fail Create a Socket - )))
if (!_sock_connect(sock, host, strlen(host), defPort)) {
    sock_close(sock); return 0; // Failed
    }
Sock->sock= sock; Sock->state = sockConnected; // Set It, now - you can send & Receive data???
Sock->connectStatus = connectTCP;
Sock->modified = TimeNow;
return 1; // ok
}


int SocketListener(Socket *sock,int port,
   void *onConnect, void *checkPacket, void *onDie) {  // makes a pool & connect it to listener -)))
SocketPool *ns;
if (!SocketListen(sock,port)) return 0; // failed - remove a pool???
ns = SocketPoolNew();
ns->log = sock->log; ns->logLevel = sock->logLevel;
sprintf(ns->name,"tcpPort:%d",port);
sock->onConnect = onConnect;
sock->onDie = onDie;
sock->checkPacket = checkPacket;
objAddRef(sock); Socket2Pool(sock,ns); // add it to pool???
return 1; // OK
}

