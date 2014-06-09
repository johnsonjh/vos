#ifndef VOS_H_INCLUDED
#define VOS_H_INCLUDED

#include "vtypes.h" // add my types
#include <fcntl.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/socket.h>

#define MAX_PATH PATH_MAX
#define _vsnprintf vsnprintf
#define closesocket close
#define stricmp(str1,str2) strcasecmp(str1,str2)
#define strnicmp(str1,str2,len) strncasecmp(str1,str2,len)


#ifdef __WIN32__ // Win32 or WinCE here
#include <windows.h>



#else // LINUX or UNIX
#include <unistd.h>
#define O_BINARY 0
int filelength(int file);

#endif


// threads
void *thread_create (void *proc, void *arg); // creates a new thread (and returns it handler), or return NULL (on ERROR)
//void  thread_suspend(void *thread); // win-only?
//void  thread_resume (void *thread); // win-only?
void  thread_destory(void *thread);

// mutex [ reentrant mutex ]
void *mutex_create ();
void  mutex_lock   (void *mutex);
void  mutex_unlock (void *mutex);
void  mutex_destroy(void * mutex);

// time & timers
void msleep(int msec); // msec { usleep(1000*msec); }
unsigned long os_ticks(); //GetTickCount()


// exec programs - to get out in a buffer
int os_exec(char *cmd, int (*on_out)(), void *arg); // on_out ret 0 = ok, continue

// anyc com-ports wrapping (vos_com_[OS].c
void *prt_open (char *name); // 0=error
int   prt_peek (void *com,void *buf, int bytes); // (AsyncRead) 0=not-ready, <0 error
int   prt_write(void *com,void *buf, int bytes); // <0 error
void  prt_close(void *com);

// term extenstions (win like)
int kbhit();

int os_mem_used();


// sockets

int net_init();
int sock_async(int sock);
int sock_listen(int port, int jobs);
int sock_accept(int lsock, int *ip);
int sock_readable(int sock);
int sock_writable(int sock);
int sock_close(int sock);

// timing

extern int sleepTime; // defined in exe.c


#endif // VOS_H_INCLUDED
