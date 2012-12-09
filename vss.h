#ifndef VSS_H
#define VSS_H

/*
 ���� ��� �������� ������ - ��������
 13��� - �������� vssEQ � ����� �����
       ��������� ������� vssCMP
*/


#include "vtypes.h"


#define VSS(A) (A).len,(A).len,(A).data
#define VSS2BUF(Vss,Buf) { int tmpLen=(Vss).len; if (tmpLen>=sizeof(Buf)) tmpLen=sizeof(Buf)-1; memcpy(Buf,(Vss).data,tmpLen); Buf[tmpLen]=0; }

typedef struct { // ������ ��������� http-������� -)))
    vss R,H,B; // ���� �������, ������ Headers, � Body (���� ���� Post)
    vss M,U,V; // Method (GET/PUT), Url, HTTP-Version
    vss page,args; // page - till '?' & = after '?'
    vss file; // ����������� ��� (����� ���������� ����)
    vss reqID; // X-Request-ID (or 0,0)
    int keepAlive; // -1 = undefined, 0 = no KeepAlive (close&other), 1 = keepAlive
    } vssHttp;

vss vssCreate(char *data, int len);

vss vssGetLen(vss *src, int len); // ��������� len bytes �� �������
int vssTrim(vss *src); // ltrim + rtirm
vss vssGetTillStr(vss *src, char *div, int dl, int remDiv); // Get TillDivider
vss vssGetCol(vss *src); // ������� - ��� ���������
vss vssGetReportPart(vss *D, int *typ); // <?  ?> - � ������� !!!

// SQL functions
vss vssGetSQLParam(vss *SQL);  // ���������� ��������� (:name) � ����� ������
vss vssGetPropValue(vss *PROP, char *name, int nl); // ����������� ��� name\tvalue\n

vss vssMimeSection(vss *src, vss *del);
vss getMimeField(vss *src, char *name, int nl);
vss vssExtractFileName(uchar *s, int len);
int vssHasPropValue(vss *PR, char *name, int nl,vss *Res);
int getMimeHeader(vss  *src, vss *name, vss *val);
int vssFindMimeHeader(vss *srv,uchar *name, vss *val); // ���� � ��������� val �� ����� ������


vss vssVSP(vss *src, vss *code);
int vssLcmp(vss *src, char *name);

vss vssGetRow(vss *src);

void *vsChildAddStringVSS(void *obj, vss *name, vss *val);
int vssEQ(vss *x, uchar *str);
int vssCMP(vss *n1,vss *n2);

int vssvss(vss *str,vss *substr); // check - if we have it as substr???
int vssHasOne(vss *str,vss patt,uchar *del);

vss vssGetWord(vss *src) ;
uchar *vss2str(uchar *buf, int size, vss *v) ;
#define vss2buf(A,V) vss2str(A,sizeof(A),V)


// vss ini declarations ---)))

int iniFindSection(vss src, uchar *name,vss *v) ; // FindSectionByName
int iniFindSection0(vss src, vss name,vss *v);
int iniNextSection(vss *src, vss *name, vss *data);
int iniFindValue(vss  src, uchar *name, vss *v);

uchar *iniFindStringDef(vss sec, uchar *name, uchar *def);
int iniFindIntDef(vss sec, uchar *name, int def);
int iniFindStringBuf(vss sec, uchar *name, uchar *buf,int size);

int vssGetMimeHeader(vss  *src, vss *name, vss *val) ; // ����������� MimeHeader

// -- vss added

int vsslcmp(vss *src, char *name); // chage a shift ( like vssLcmp+)
int VSSVSS(vss v1,vss v2); // finds occurence v2 in v1 (returns position or <0

#endif //VSS_H
