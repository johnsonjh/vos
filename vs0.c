#include <fcntl.h>

#include "vs0.h"
#include "vos.h"
//#include "vtypes.h"
//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <fcntl.h>
//#include <sys/stat.h>

//#define debug 1


#ifndef WIN32
int filelength(int file) {
struct stat st;
if (fstat(file,&st)!=0) return -1;
return st.st_size;
}
#endif



// memcpy->memmove
//#define printf

memDisp defMem = { // ��������� �� ���������
    0, (void*)malloc, (void*)realloc, (void*)free,
    "defMem", // ����
    0,0,0 /* empty stat */
    };

/*  -- ���������� ���� -- */
typedef unsigned char Byte;
VS0OBJ0(Byte,0); // ��� ������ �� �������
VS0OBJ0(Integer,0);
VS0OBJ0(Number,0);
//VS0OBJ0(Date,0);

/* --  ���������� ������� -- */
VS0ARRAY(String,Byte); // ����������� ����� ��� ������ ��������� byte
VS0ARRAY(Blob,  Byte); // ����������� ����� ��� ������ ��������� byte

void *objAddRef(void *obj) { // Gets a new ref to object
objHead *h = obj2head(obj); memDisp *md;
if (!obj) return 0;
//printf("ObjAddRef: h=%p md=%p ref=%d sizeof(head)=%d\n",h,h->mem,h->ref,sizeof(objHead));
md = h->mem;
if (md) md->ref++;
h->ref++;
return obj; //
}

objHead *objHeadNew(memDisp *md, int size1, int size) { // �������� ������ �� ��������� ������
objHead *h; int sz = size1 * size + sizeof(objHead);
if (!md) md = &defMem;
//printf("...Malloc %d bytes size=%d\n",sz,size);
h = md->malloc(sz);
if (!h) return 0; // fail?
memset(h,0,sz);
h->size1 = size1; h->size = size; h->ref = 1; h->mem = md; h->len = 0;
md->count++; md->ref++; md->size+=sz; // update stat
return h;
}

void *objNew(objClass *cls,int size) {
objHead *h;
h = objHeadNew(0,cls->size1,size);
if (!h) return 0;
h->cls = cls; h->len = size;
return head2obj(h);
}

//void *objClassNew(objClass *cls) { return objClassNew2(cls,1);} // Struct Create


void *objObjectNew() { objHead *h; h = objHeadNew(0,0,0); if (!h) return 0; return head2obj(h);}

void objHeadZero(objHead *h) { // zero unused mem
uchar *data =  head2obj(h);
memset(data+h->len*h->size1,0,(h->size-h->len)*h->size1);
}

int objHeadSize(objHead **h, int newSize) { // check - if we have enough size
objHead *H=*h; memDisp *md = H->mem;
if (H->size>=newSize) return 1; // nothing to do
if (!md) return 0; // cant manage static
if (newSize<H->len*2) newSize = H->len*2; // �� ����� ��������
H = realloc(H,sizeof(objHead)+H->size1*(newSize)); // NewSize request
if (!H) return 0; // Realloc Failed
md->size+=(newSize-H->size)*H->size1; // correct Stat
H->size = newSize; *h=H;
return 2; // realloced
}

uchar *strNew(uchar *data, int len) { // ������� ������ �� ���������� �������
uchar *d;
if (len<0) { len = data?strlen((char*)data):0; }; // �������������� �����
d = arrayNew(StringClass,len+1);
if (!d) return 0;
obj2head(d)->len=len; // remove 0 trailer
if (data) memcpy(d,data,len); else memset(d,0,len); // copy it !!!
d[len]=0;
return d;
}

#include <fcntl.h>

uchar *strLoad(uchar *filename) { // ������� ������, �������� �� �� �����
int file,len; uchar *str;
file = open((char*)filename,O_RDONLY | O_BINARY,0);
if (file<=0) return 0;
len = filelength(file);
//printf("filename=%s has %d bytes\n",filename,len);
str = strNew(0,len);
if (!str) { close(file); return 0; }
if (read(file,str,len)!=len) { strClear(&str); close(file); return 0;};
//printf("Loaded:%s\n",str);
close(file);
return str;
}

int buf2file(uchar *data,int len,uchar *filename) {
int file;
file = open((char*)filename,O_WRONLY | O_BINARY | O_TRUNC | O_CREAT,S_IREAD|S_IWRITE);
if (file<=0) return -1;
if (write(file,data,len)!=len) { close(file); return -1; }
close(file);
return len;
}

int strSave(uchar *str, uchar *filename) {
return buf2file(str,strLength(str),filename);
}

uchar *strCatFile(uchar **Str,uchar *filename) { // ������� ������, �������� �� �� �����
int file,len; uchar *str;
file = open((char*)filename,O_RDONLY | O_BINARY,0);
if (file<=0) return 0;
len = filelength(file);
//printf("filename=%s has %d bytes\n",filename,len);
str = strCat(Str,0,len);
if (!str) { close(file); return 0; }
if (read(file,str,len)!=len) { strClear(&str); close(file); return 0;};
//printf("Loaded:%s\n",str);
close(file);
return str;
}


uchar *strCat(uchar **str, uchar *data, int len) { // �������� � ������
uchar *d;
if (!str) return 0;
if (len<0) { len = data?strlen((char*)data):0; }; // �������������� �����
if (!*str) { *str = strNew(data,len); return *str;} // ������ ������ - ������������
//printf("CAT %d bytes to %d bytes <%s>\n",len,obj2head(*str)->len,*str);
d = arrAddN((void**)str,len+1); // check - if ok?
//printf("DoneMalloc %d bytes res='%s' strlen=%d,len=%d\n",len+1,*str,strlen(*str),obj2head(*str)->len);
//printf("P=%p D=%p SHIFT=%d\n",*str,d,*str-d);
if (!d) return 0;
if (data) memcpy(d,data,len); else memset(d,0,len);
d[len]=0;
//printf("RESLEN=%d RES='%s', ADD='%s; D-1=%s\n",obj2head(*str)->len,*str,d,d-8);
obj2head(*str)->len--;
return d;
}

uchar *strCatD(uchar **str, uchar *src, int len, int (*decoder)()) {
int sz; uchar *d;
if (len<0) len = strlen((char*)src); // ����� ��� �����������
sz = decoder(0,src,len); // ���������� ����� ��������� ������
if (sz<0) return 0; // ������ �������������
//printf("HereBufZ=%d\n",sz);
d = strCat(str,0,sz); // ������������ �����
if (!d) return 0; // ������ �������������
decoder(d,src,len); // ���������� �����
return d;
}

int strSetLength(uchar **str, int newLength) {
objHead *h;
if (newLength<0) { newLength=0; if (*str) newLength=strlen((char*)*str); }
if (!*str) {*str = strNew(0,newLength); return 1;}
 else arrSetLength((void**)str,newLength+1); // SetIt... Zero Terminated???
h = obj2head(*str); h->len--;
(*str)[newLength]=0; // ZeroTerm
return 1;
}

int strEnsureSize(uchar **str,int newLength) {
int len = 0;
if (str && *str) len = strLength(*str); // remember a new length
if (!strSetLength(str,newLength)) return 0;
strSetLength(str,len); // fix back
return 1;
}

uchar *strEnsureMore(uchar **str,int newLength) { // ensure more data to the end of it
int len = 0;
if (str && *str) len = strLength(*str); // remember a new length
if (!strSetLength(str,newLength+len)) return 0;
if (!*str) return 0; // ??? ZU???
strSetLength(str,len); // fix back
return (*str)+len; // end of it
}


char *clsName(void *obj) {
objHead *h;
if (!obj) return "null";
h = obj2head(obj);
if (!h->cls) return "undefined";
return h->cls->name;
}

void objClear(void **obj) { // ��������� ���������� ������ �� 1 � ������� ������ ���� 0
objHead *h; memDisp *md; objClass *cls;
int chCount;
if (!obj || !*obj) return;
h = obj2head(*obj);  // �������� ����������, ������ ������
md = h->mem; cls = h->cls;
if (!h->ref) return ; // circle
chCount = arrLength(h->att);
h->ref--;
//printf("objClear: %s:%p ref:%d len:%d done=%p\n",clsName(*obj),*obj,h->ref,h->len,cls?cls->done:0);
*obj = 0;
if (md) { // ������ ��� ������������ ��������
    md->ref--;
    if (h->ref==chCount) { // ��� ���� ������ - ��� ���� !!!
        char *data = head2obj(h);
        h->mem = 0; h->ref-=chCount; md->ref-=chCount;// no more deleting
        if (h->att) _attClear(h);
        // may be done ?
        if (cls && cls->done) { // ������ �������� ������� - )))
            int i;
            for(i=0;i<h->len;i++,data=ptrMove(data,h->size1))  cls->done(data,cls); // �������� �����.
            }
        md->count--; md->size-=h->size1*h->size+sizeof(objHead); // ������ ����������
        md->free(h); // ������� ��� �����
        }
    }
}


int strDelete(uchar **str,int pos,int len) {
    int L;
    L = arrLength(*str);
    if (!L) return 0; // ������ �������..
    if (pos<0) {
        pos = L+pos;    // ���� pos<0 - �� � �����
        if (pos<0) pos=0;
    }
    if (len<0|| len>L-pos) {
        len = L-pos;    // �� ����� ������
    }
    len = arrDelN((void**)str,pos,len);
    if (len) (*str)[strLength(*str)]=0;  // ������������� ����� ����
    return len;
}


inline void strClear(uchar **str) { objClear((void**)str); };
int  strLength(uchar *str) { return arrLength((void**)str);}

// Arrays

void *arrNew(int size1,int size) {
objHead *h;
if (size<2) size = 2;
h = objHeadNew(0,size1,size);
if (!h) return 0;
return head2obj(h);
}

/*
objClass *makeClassArrayOf(objClass *cls) {
objClass *c;
c = objClassCreate(0,clsArray,cls);
objClassDump(c,1);
getch();
//c->size1 = cls->size1; // ���� ������ �� ������� -)))
//c->cls = objAddRef(cls); // set base class!!!
return c;
}
*/


/* ������ ����� ���� ������ �� ������� ������ ��� �������
   ���� ������ ���������� �� ��������� - �� ��������� ������-����� (ArrayOf ---)
   ��� ������ ��������� � ���������� ������� �� ����
*/

void *arrayNew(objClass *cls, int sz) {
objHead *h; objClass *base = cls->cls; //void *obj; // ����������� ����
/*
if (0 == (cls->typ&clsArray)) { // create sub class for this
    printf("Class %p is not array, subclassing...\n",cls);
    cls = makeClassArrayOf(cls); // Create a new sub class for this
    printf("Created %p - must be array...\n",cls);
    obj = arrayNew(cls,sz);
    objClassClear(&cls);
    return obj;
    }
*/
if (!base) {
    printf("!!!! NoBaseClass in cls=%p\n",cls);
    return 0;
    }
if (sz<2) sz = 2; // ������ ���-����, ����� ���� ����������� ����� ���������� ����
//printf(">>>Create a head sz1=%d sz=%d...\n",base->size1,sz);
//return 0;
h = objHeadNew(0,base->size1,sz); // �������� size1 � ������� ������
if (!h) return 0;
h->cls = cls; // ������������� ����
//printf("ClassArray %s accessed %d bytes\n",cls->name,sz);
return head2obj(h);
}

int arrLength(void *arr) {
    if (!arr) return 0;
 //objHead *h = obj2head(arr);
 //printf("arrLength here: obj=%x head=%x sizeof=%d, h=%x\n",arr,obj2head(arr),sizeof(objHead),h);
 //printf("h->len=%d\n",h->len);
 //printf("obj2head(arr)->len=%d\n",obj2head(arr)->len);
 return obj2head(arr)->len; }

objRef **attClone(void *owner,objRef **ref) { // ��������� ���������
objRef **r; int i;
r = (void*)vecNew(arrLength(ref)); // ������� ������� �������
//printf("NewR=%d len=%d\n",r,arrLength(ref));
if (!r) return 0;
for(i=0;i<arrLength(ref);i++) { // ��������� ����� --))
    objRef *rr,*rs=ref[i];
    rr = objRefCreate(owner);
    rr->name = objAddRef(rs->name);
    rr->data = objAddRef(rs->data);
    _vecPush((void*)&r,(void*)rr); // save a data
    }
return r;
}

void *arrInsN(void **arr, int pos, int count) { // ��������� � ������ -)))
objHead *h; uchar *d,*data; int i;
if (!arr) return 0; // dont now size --)))
h = obj2head(*arr);
if (pos<0 ) pos = h->len+1+pos; // FromTheEnd
//printf("pos2=%d\n",pos);
if (pos<0 || count<0 || pos>h->len) return 0; // invalid
//printf("arrAddN %d bytes str=<%s>\n",h->size1*h->len,*arr);
if (h->ref>1+arrLength(h->att)) { // need copy - we have extra ref
    objHead *h2;
    d = arrNew(h->size1,h->len+count); // create a new one
    if (!d) return 0; h2 = obj2head(d);
    //printf("!!!COPY %d bytes str=<%s>\n",h->size1*h->len,*arr);
    memmove(d,*arr,h->size1*(pos)); // copy first elements
    memmove(ptrMove(d,h->size1*(pos+count)),
            ptrMove(*arr,h->size1*pos),h->size1*(h->len-pos));       // last elements
    h2->len = h->len+count; h2->cls = h->cls;  *arr = d;
    if (h->att) h2->att = attClone(d,h->att);
    //printf("ClonedSize=%d for %p\n",arrLength(h2->att),*arr);
    d+=h->len*h->size1; // to the end
    data=head2obj(h); objClear((void*)&data); // correct a data
    return d;
    }
if (!objHeadSize(&h,h->len+count)) return 0;
d = head2obj(h); *arr = d; // ������������ ������ ����� �����������������
memmove(ptrMove(d,h->size1*(pos+count)),
            ptrMove(d,h->size1*pos),h->size1*(h->len-pos));       // last elements
void *res;
res = ptrMove(d,h->size1*pos); // resulted Slot?
memset(res,0,h->size1*count); // ZeroNew
//..printf(" >>> ok - realloced newP=%p ...\n");
for(i=0;i<arrLength(h->att);i++) {
    objRef *r = h->att[i];
    r->owner = d; // reset owner (if cnagned)
    }
h->len+=count; // ������ ����� ������
return res;
}

void *arrAddN(void **arr, int count) { // ��������� � ������ -)))
//objHead *h; uchar *d,*data; //int i;
return arrInsN(arr,-1,count); // �������� � �����
//if (!arr) return 0; // dont now size --)))
/*
h = obj2head(*arr);
//printf("arrAddN %d bytes str=<%s>\n",h->size1*h->len,*arr);
if (h->ref>1+arrLength(h->att)) { // need copy - we have extra ref
    objHead *h2;
    d = arrNew(h->size1,h->len+count); // create a new one
    if (!d) return 0; h2 = obj2head(d);
    //printf("!!!COPY %d bytes str=<%s>\n",h->size1*h->len,*arr);
    memmove(d,*arr,h->size1*h->len); // copy prev
    h2->len = h->len+count; h2->cls = h->cls;  *arr = d;
    if (h->att) h2->att = attClone(d,h->att);
    //printf("ClonedSize=%d for %p\n",arrLength(h2->att),*arr);
    d+=h->len*h->size1; // to the end
    data=head2obj(h); objClear((void*)&data); // correct a data
    return d;
    }
if (!objHeadSize(&h,h->len+count)) return 0;
d = head2obj(h); *arr = d; // ������������ ������ ����� �����������������
//..printf(" >>> ok - realloced newP=%p ...\n");
for(i=0;i<arrLength(h->att);i++) {
    objRef *r = h->att[i];
    r->owner = d; // reset owner (if cnagned)
    }
d+=h->len*h->size1; h->len+=count; // ������ ����� ������
return d;
*/
}


int arrDelN(void **arr,int pos,int count) { // �������� ���������� ����������
    objHead *h = obj2head(*arr);
    void *data;
    objClass *cls;
    int i;
// ��� �������� (���� ����� ����� ������) - �������� ������������ �������
    if (pos<0 || count<0 || pos+count>h->len) {
        //printf("arrDelN: pos:%d count:%d declined on len:%d arr:%p arrLen:%d\n",pos,count,h->len,*arr,arrLength(*arr));
        return 0; // ������ ����������
    }
//printf("addDelN: check changes arr=%p\n",*arr);
    if (!arrAddN(arr,0)) return 0; // ��������� ����������
//printf("addDelN: ok changes arr=%p\n",*arr);
    h = obj2head(*arr); // ������ ����� ���������� �� �� ������������ �������
    cls = h->cls;
//printf("head=%p cls=%p\n",h,cls);
//printf("delN:Call Des pos=%d count=%d len=%d done=%p \n",pos,count,arrLength(*arr),cls?cls->done:0);
    if (cls && cls->done)
        for (i=0,data = ptrMove(*arr, h->size1*pos);i<count;i++,data=ptrMove(data,h->size1)) { // �������� �����������
            cls->done(data,cls); // ������� �������
        }
//printf("delN:des done: BeginMoveData\n");
    data = ptrMove(*arr, h->size1*pos);
    memmove(data, ptrMove(data,h->size1*count), h->size1*(h->len-count-pos)); // ������� ������
    h->len-=count; // ��������� �������
    return count; // ������
}

int vecDataIndex(void **vec, void *data) {
int i;
for(i=0;i<arrLength(vec);i++) if (vec[i]==data) return i;
return -1;
}

int vecRemove(void ***vec, void *data) { // Removes this data from a vector
int cnt=0;
// ������� ���� ������ - ����� ������� ���
while(1) {
    //printf("FindIdx in %p\n",vec);
    int idx = vecDataIndex(*vec,data);
    if (idx<0) break;
    //printf("RemoveIdx=%d\n",idx);
    arrDelN((void**)vec,idx,1);
    cnt++;
    }
return cnt;
}




void *arrAdd(void **arr, void *data) { // ��������� � ������ -)))
objHead *h; uchar *d;
if (!arr) return 0; // dont now size --)))
d = arrAddN(arr,1);
if (!d) return 0;
h = obj2head(*arr);
if (data) memmove(d,data,h->size1); else memset(d,0,h->size1);
return d;
}
// ������� - ������ ������ �� ��������

objClass VarClass = {.typ=0,.name="var",.size1=0,.done=objClear}; // ��� ��������� ��� ���������� -)))

void **vecNew(int size) { // �������� ������� ������
void **v;
v = arrNew( sizeof(void*), size);
if (!v) return 0;
// attach a class
obj2head(v)->cls = &VarClass; // ������ ����� ����������
return v;
}

void **_vecPush(void ***vec, void *data) { // �������� ����, ���������� ����� ����� (���� ��)
if (!*vec) { *vec = vecNew(1);  if (!*vec) return 0; }
data = arrAdd((void**)vec,&data); // add a data to a vector -)))
return data;
}

void *vecPopIndex(void ***vec,int idx) {
void *res;
int len = arrLength(*vec); // Gets Current Length
if (idx<0) { idx = idx-len; }; // From a rest (-1 - is a last in array)
if (idx<0 || idx>=len) return 0; // No Such
res = objAddRef((*vec)[idx]); // getIt
arrDelN((void*)vec,idx,1);
return res;
}

void **vecPush(void ***vec, void *data) { // �������� ����, ���������� ����� ����� (���� ��)
void *slot;
slot = _vecPush(vec,data);
if (slot) objAddRef(data);
//printf("OK ADD data=%d to vec=%d zero=%d\n",data,vec,vec[0][0]);
return slot;
}

inline uchar *strPush(uchar ***str, uchar *data) { // �������� ����, ���������� ����� ����� (���� ��)
if (_vecPush((void*)str,data)) {
    objAddRef(data); return data;}
//printf("OK ADD data=%d to vec=%d zero=%d\n",data,vec,vec[0][0]);
return 0; // Failed???
}

inline void vecClear(void ***vec) { objClear((void**)vec);}

// ������ � ����

void objRefDone(objRef *r) { // ������� �������� - )))
//printf("RefDone!\n");
objClear((void**)&r->name);
objClear((void**)&r->owner);
objClear((void**)&r->data);
}

int arrSetLength(void **arr,int size) { // Results in length set - it changes arraya anyway!
    objHead *h = obj2head(*arr);
    int del;
    if (size<0) return 0;
    del = h->len-size;
    //printf("SetLength: DEL=%d\n",del);
    if (del<=0) return 0!=arrAddN(arr,-del); // Try Add to the end
    return arrDelN(arr,h->len-del,del);
}



VS0OBJ(objRef);

void _attClear(objHead *h) {
objRef **att = h->att; // int i;
h->att=0; // ����� ����� (���� ����� - ����������� ������)
objClear((void*)&att);
//printf("attClear: %d childs\n",arrLength(att));
//for(i=0;i<arrLength(att);i++) objRefClear(att+i);
//objClear((void*)&att);
}

void attClear(void *obj) { // clear all attributes
if (!obj) return;
_attClear(obj2head(obj));
}


objRef *objRefCreate(void *obj) { // ������� ����� ������ �� ������? ��������� � ����
objRef *r;
r = objNew(objRefClass,1);
r->owner = objAddRef(obj);
return r;
}

objRef *_attAdd(void *obj) { // ������� ������ � ������������ � ��������
objHead *h; objRef *r,*r2;
if (!obj) return 0;
r = objRefCreate(obj); if (!r) return 0;
//printf("CreatedRef r=%p\n",r);
h = obj2head(obj);
if (!h->att) { h->att = objRefVector(2);
  if (!h->att) { objRefClear(&r); return 0;} }
if (!objRefPush(&h->att,r)) { // ��������� ��� ����
    objRefClear(&r); return 0; }
r2 = r;
objRefClear(&r); // ��������������� ������
return r2;
}

objRef *_attAddStr(void *obj,char *name,char *val) {
objRef *r;
r = _attAdd(obj); if (!r)  return 0;
r->name = strNew((void*)name,-1);
r->data = strNew((void*)val,-1);
return r;
}

int attCount(void *obj) { if (!obj) return 0; return arrLength(obj2head(obj)->att);}

// ����� ����������� ������� �����

int    *IntNew(int val) { int *ret; ret = IntegerNew(); *ret = val; return ret;}
double *NumNew(double val) { double *ret; ret = NumberNew(); *ret = val; return ret;}
// Date
// Str
// Blob ???


