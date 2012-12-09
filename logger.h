#ifndef LOGGER_H
#define LOGGER_H
#include "vs0.h"
#include <stdio.h>
#include <time.h>
#include "coders.h"

typedef struct {
    FILE *file; // ���� ���� ����� ���� - �� ����
    int level; // ��� ����� ������ �� ����, ����?
    void *mutex; // ����������� ������
    time_t now; // LastFlashed???
    uchar buf[2*1024]; //  ���� ������ ������
    } logger;

logger *logOpen(uchar *filename); // ���� - ���� ��� - ���������� ������
void    logWrite(logger *log,uchar *src,int lev,char *fmt,...) ; // ������� � ��� ��� ������� - ����� ��������
void logDump(logger *log,uchar *src,int lev,void *data,int dlen, uchar *fmt,...) ; // ������� � ��� ��� ������� - ����� ��������
void logUpdateNow(logger *log); // Updates log sztime info
void logClose(logger *log) ;

void hexDump(FILE *file, uchar *data, int len); // BinDumper utility
extern int log2stdout; // External int value
//#define LG(logger,lev) if (logger->level>=lev) logWrite

VS0OBJH(logger);

#define IFLOG(V,L,A...) if(V->log && V->log->level>=L) logWrite(V->log,#V,L, ##A ) /* Private Log */
#define CLOG(V,L,A...)  if((V)->log && (V)->logLevel>=L) logWrite((V)->log,(uchar*)(V)->name,L, ##A ) /* ComponentLog */
#define Logf(fmt...)  logWrite(0,"core",0, ##fmt ) /* ComponentLog */
#define DLOG(V,L,data,dlen,A...)  if(V->log && V->logLevel>=L) logDump(V->log,V->name,L,data,dlen, ##A ) /* ComponentLog */


#endif //LOGGER_H
