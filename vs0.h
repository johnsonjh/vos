#ifndef VS0
#define VS0

#include "vtypes.h" /* ����� ���� uchar,byte */

#define CHUNK_STEP 2 /* ��������� ������������ ���������� ������ - �������� ������ */

typedef void * Obj; // ��� ��� �������� ���=������

typedef struct { // �������������� ������ (vsMem.c)
    void *handle; // ����������� ������
    void* (*malloc)(int size);
    void* (*realloc)(void *p, int size);
    void  (*free)(void *p);
    char *name; // Static name of allocer
    int ref,count,size;  // ���������� -  ��������� ���������� ������, ������� ���������� � ������ ������ ...
    } memDisp; // ������ ���������

extern memDisp defMem; // ���������� ���������� ������ ���������

enum objClassTyp {
    clsDynamic = 16, // ����������� ��������� �����. ����� ������ ������ ������� ������ (�� ��������);
    clsDefine  = 1<<8, // ��� ��������������� - �����������
    clsStruct  = 3<<8, clsClass  = clsStruct, // ���� ���������
    clsUnion   = 4<<8, clsChoice = clsUnion,
    clsArray   = 8<<8,
    clsRef     = 32,  // ��������� ���� �������� - ��� ��������� - ���� ��������� ��� ������ ������ �� ���?
    //clsArray   = 1, // ����� - ������. cls->base - ��������� �� ��������� �������;
    clsVector  = 2, // ������ - ������ ���������� �� ��������� �������� ������;
    //clsStruct  = 3, // ����� - ���������, cls->base ��������� �� ������������ ��������;
    //clsChoice  = 4, // ����� = ���������� ��� REF?
    clsFunctor = 5, // ����� - �������. ���������� ������� call() � ������.
    };


typedef struct packed { // ���� ��������� ������
    int typ;     // ����� - �� objClassTyp
    uchar *name;  // ��� - ����������� ��� ����������� � ������������ ��� ������������
    int size1;   // ������� ������ ��� �������� ������
    void (*done)(); // ������ �� ���������� ������� �������
    void **par;    // ������ �������� ����� - ����������
    void *cls;   // ������� �����. ��� �������� - ������ ��������� �����, ��� �������� - ������� ���������
    void *extra; // �������������� ���� - ������������ ��� �������� ����-���������� � ������� ��������
    int  shift; // ������������ ��� ������ � �������� ����� (�������� �������� ���� � ������)
    int  maxSize; // ������������ ������� (������ ��� Array) - ���� ������ �� ����������� ����� ���
    } objClass;

/*
typedef struct { // ��������� ���� "����"
    int size,len; // ��� ������� ����������������� - �������� ������������� � db_blob_handle
    uchar *data;
    int handle;
    } blob;
*/


#define Integer int
#define Number double
//#define Date   double

typedef objClass *pClass;

extern pClass IntegerClass,ByteClass,NumberClass,DateClass,StringClass,BlobClass; // ����� ��� �����...

typedef struct { // C�������� ��� ���������� �����
    void  *owner; // �������� ������
    uchar *name; // ��� - ���� ������ �����������
    void  *data;  // ���� ����� �� �������� (child)
    } objRef;

typedef struct { // ��������� ����� - ����� ������ ���������� �����
    void *mem;   // ������ ���������, 0 = ����������� ����������
    int   ref;   // ���������� ������ �� ������ ������ (0 - ��������)
    void *proto; // ������, ���������� ���������� ��� ������� ������� (����������� �� ������������)
    objRef  **att;   //  void *ch;    //vsHash *ch;  // �������� �������, ������
    // -- ���� ��� ��������� � �������
    //int typ; // ��� ������� (�����������, ������, ������)
    int size1; // ������ ������� �������� � ������ (������� ����� ������� ������ ��� �������)
    objClass *cls;  //vsClass *cls; // ����� - ������ ���� ���������
    int  size,len; // -- Array -- ���������� � ������������ ������ (� ��������� = �������� cls)
    } objHead; // 8*4 = 32 ����� ---

#define obj2head(obj) ((objHead*)(((char*)obj)-sizeof(objHead)))
#define head2obj(obj) ((void*)(((char*)obj)+sizeof(objHead)))

objHead *objHeadNew(memDisp *md, int size1, int size); // ����� ������+���������
int objHeadSize(objHead **h, int newSize); // check - if we have enough size
void *objObjectNew(); // �������� ����������� ���� "������ ������"
char *clsName(void *obj); // ��� ������ (��� �������)

void *objAddRef(void *obj); // Gets a new ref to object
objRef *objRefCreate(void *owner) ; // ������� ����� ������ �� ������? ��������� � ����
objRef **attClone(void *owner,objRef **ref) ; // ��������� ���������
void objClear(void **obj);  // ��������� ���������� ������ �� 1 � ������� ������ ���� 0

int * IntNew(int val); // �������� ������ �������� ���

uchar *strNew(uchar *data,int len); // �������� ����� ������
uchar *strLoad(uchar *filename); // ��������� ������ �� �����, NULL ��� ������
int    strSave(uchar *str, uchar *filename); // -1 ��� ������, ����� - ������ �����
int strDelete(uchar **str,int pos,int len); // ���������� - ���������� ���������
uchar *strCat(uchar **str, uchar *data, int len); // �������� � ������
uchar *strCatFile(uchar **Str,uchar *filename); // ��������� � ������, �������� �� �� �����
uchar *strCatD(uchar **str, uchar *src, int len, int (*decoder)()); // �������� � ��������������
int    strLength(uchar *str); // ����� ������
inline int    strSetLength(uchar **str, int len); // NewLength

int strEnsureSize(uchar **str,int newLength); // ensures that this string buffer can handle newLength
uchar *strEnsureMore(uchar **str,int addLength); // returns end of current string

inline void strClear(uchar **str);
inline uchar *strPush(uchar ***str, uchar *data);


int arrSetLength(void **arr,int size) ; // Results in length set - it changes arraya anyway!
void *arrayNew(objClass *cls, int sz);
void *arrNew(int size1,int size); // ����� �������� ������
int   arrLength(void *arr); // ����� �������
void *arrAddN(void **arr, int count); // ��������� � ������, ���������� ������ �������
int   arrDelN(void **arr,int pos,int count) ; // �������� ���������� ����������
void *arrAdd(void **arr, void *data); // �������� � ������
void *arrInsN(void **arr, int pos, int count) ; // ��������� � ������ � �������


//void *objNew(); // ������ ������ ������
void *objNew(objClass *cls,int size);
void _attClear(objHead *h);
objRef *_attAdd(void *obj); // ���� ���� ��� ����������� ����������
objRef *_attAddStr(void *obj,char *name,char *val); // ���������� ���������� ���������
void attClear(void *obj); // ������� ���� �����

void **vecNew(int size) ; // �������� ������� ������ �� ������� �������
void **_vecPush(void ***vec, void *data);  // ��������������� ����������
void ** vecPush(void ***vec, void *data);  // ����, �� � ����������� ������
void vecClear(void ***vec); // �������� ������� (� ���� ��� ���������)
int vecRemove(void ***vec,void *data); // �������� ������� (� ���� ��� ���������)

/* ��������� ������ - ������������ � ����������� �� ��������� */

typedef struct {
    objHead head;
    objClass cls;
    } StaticClass;

#define VS0OBJ0(fld,doneProc)  \
   StaticClass fld##Head = { .head={.mem=0,.ref=0},\
                             .cls={.typ=0,.name=#fld,.size1=sizeof(fld),.done=doneProc }};\
   objClass *fld##Class = &fld##Head.cls;\
   inline fld* fld##New() { return objNew(&fld##Head.cls,1); }\
   inline void fld##Clear(fld **f) { objClear((void**)f);}\
   inline fld*   fld##Array(int size) { return arrNew(sizeof(fld),size); };\
   inline fld*   fld##Add(fld **f, fld *data) { \
         if (!*f) *f = (void*) fld##Array(1);\
         return arrAdd((void**)f,data); };\
   inline fld**  fld##Vector(int size) { return (void*)vecNew(size); };\
   inline fld**  fld##Push(fld ***f, fld *data) { return (void*)vecPush((void***)f,(void*)data); };\
   inline void   fld##Clean(fld ***f) { objClear((void**)f);}\



#define VS0OBJ00(fld,done)  \
   StaticClass fld##Head = { {0,0},\
                             {0, #fld,sizeof(fld),done }};\
   objClass *fld##Class = &fld##Head.cls;\
   inline fld* fld##New() { return objNew(&fld##Head.cls,1); }\
   inline void fld##Clear(fld **f) { objClear((void**)f);}\
   inline fld*   fld##Array(int size) { return arrNew(sizeof(fld),size); };\
   inline fld*   fld##Add(fld **f, fld *data) { \
         if (!*f) *f = (void*) fld##Array(1);\
         return arrAdd((void**)f,data); };\
   inline fld**  fld##Vector(int size) { return (void*)vecNew(size); };\
   inline fld**  fld##Push(fld ***f, fld *data) { return (void*)vecPush((void***)f,(void*)data); };\
   inline void   fld##Clean(fld ***f) { objClear((void**)f);}\

#define VS0OBJH(fld)  \
   extern objClass *fld##Class;\
   inline fld* fld##New();\
   inline void fld##Clear(fld **f);\
   inline fld*   fld##Array(int size);\
   inline fld*   fld##Add(fld **f, fld *data);\
   inline fld**  fld##Vector(int size);\
   inline fld**  fld##Push(fld ***f, fld *data);\
   inline void   fld##Clean(fld ***f);\

#define VS0OBJ(fld) VS0OBJ0(fld,fld##Done)
#define VS0ARR(fld) VS0OBJ(fld)

/*
//StaticClass fld##Head = { .head={.mem=0,.ref=0},\
  // ZUZUKA                           .cls={.typ=clsArray,.name=#fld,.size1=sizeof(fld),.done=doneProc }};\
*/

#define VS0ARRAY(fld,base1)  \
   StaticClass fld##Head = { .head = {.mem=0,.ref=0},\
                             .cls={.typ=clsArray,.name= #fld,.size1=sizeof(base1),.cls=&base1##Head.cls }};\
   objClass *fld##Class = &fld##Head.cls;\
   inline base1 * fld##New() { return arrayNew(&fld##Head.cls,2); }

#define VS0ARRAY01(fld,base)  \
   StaticClass fld##Head = { .head={.mem=0,.ref=0},\
                             .cls={.typ=clsArray, .name=#fld,.size1=sizeof(void*),.base=&base##Head.cls }};\
   objClass *fld##Class = &fld##Head.cls;\
   inline base * fld##New() { return arrayNew(&fld##Head.cls,2); }


   /*
   inline void fld##Clear(fld **f) { objClear((void**)f);}\
   inline fld*   fld##Add(fld **f, fld *data) { return arrAdd((void**)f,data); };\
   inline void   fld##Clean(fld ***f) { objClear((void**)f);}

   //
*/

int attCount(void *obj); // ���������� ����������
int buf2file(uchar *data,int len,uchar *filename); // some utils...




/*
// scaner c - code - )))

#include "vss.h"

typedef struct {
    char **src; // ����������� ����� -))
    vss  *fun;  // �������� ������ -))
    } scan;

#define public
//#define in
//#define out

int scanFile(scan *sc, char *filename);

// ������������ ������

objClass *objClassCreate(char *name, int typ, objClass *base); // �������� ������ ������
objClass *objClassAddParam(objClass *cls, objClass *fld); // �������� �������� ������ ������ ��������
uchar *objClassDDL(objClass *cls); // ��������� DDL �� ��������� ������-��������
int objClassDump(objClass *cls,int lev);

int *IntNew(int val); // ������� ����
*/

// child variables -)))

VS0OBJH(Obj)

#endif // VS0
