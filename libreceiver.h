#ifndef __AL_NETWORK_RECEIVER_LIB_H__
#define __AL_NETWORK_RECEIVER_LIB_H__
//
//  Header files.
//
//  Common
#include <stdio.h>                //  standard buffered input/output 
#include <stdlib.h>               //  standard library definitions 
#include <string.h>               //  string operations 
#include <errno.h>                //  system error numbers
#include <fcntl.h>                //  file control options
#include <time.h>                 //  time types 

//  Linux
#ifdef OS_LINUX
#include <sys/types.h>            //  data types
#include <sys/stat.h>             //  data returned by the stat() function
#include <sys/wait.h>             //  declarations for waiting
#include <pthread.h>              //  threads
#include <semaphore.h>            //  semaphores (REALTIME)
#include <dlfcn.h>                //  dynamic linking
#include <sys/ioctl.h>
#include <sys/poll.h>

#include <sys/socket.h>           //  Internet Protocol family
#include <netdb.h>                //  definitions for network database operations 
#include <netinet/in.h>
#include <netinet/ether.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#endif

//  Windows
#ifdef OS_WINDOWS
#include <WinSock2.h>
#endif

//
//  Definition.
//
//  Common
#define MAX_BUF_LEN             2048
#define DEBUG_PRINTF(fmt,ARG...)		printf(fmt,##ARG)

//  Linux
#ifdef OS_LINUX
#define BOOL                    int
#define TRUE                    1
#define FALSE                   0

#define INVALID_SOCKET          -1
#define SOCKET_ERROR            -1
#define closesocket(sock)       close(sock)
#endif

//  Windows
#ifdef OS_WINDOWS
#include <WinSock2.h>
#define WSA_VERSION MAKEWORD(2, 2)  // using winsock 2.2
#endif // OS_WINDOWS

typedef struct _RECEIVER_VERSION_ST
{
    unsigned int                uiMajor, uiMinor, uiRelease, uiBuild;
} RECEIVER_VERSION_ST;
const static RECEIVER_VERSION_ST    gcs_ReceiverVersion = {2, 0, 1, 4};

typedef enum
{
    RECV_RET_ERR_START = -100,
    RECV_RET_ERR_SYSTEM,
    RECV_RET_ERR_FILE_SELECT,
    RECV_RET_ERR_THREAD_INIT,
    RECV_RET_ERR_THREAD_STACKSIZE,
    RECV_RET_ERR_THREAD_DETACHSTATE,
    RECV_RET_ERR_THREAD_CREATE,
    RECV_RET_ERR_SOCKET,
    RECV_RET_ERR_SOCKET_INIT,
    RECV_RET_ERR_SOCKET_OPTION,
    RECV_RET_ERR_SOCKET_BIND,       //  90
    RECV_RET_ERR_SOCKET_LISTEN,
    RECV_RET_ERR_SOCKET_ACCEPT,
    RECV_RET_ERR_SOCKET_RECV,
    RECV_RET_ERR_INTERNAL,
    RECV_RET_ERR_NOTFOUND,
    RECV_RET_ERR_PARAM,
    RECV_RET_ERR_PROCEDURE,
    RECV_RET_ERR_TYPE,
    RECV_RET_ERR_END,
    RECV_RET_SUCCESS = 0
}RECV_RET;

typedef enum
{
    RECV_T_START = 0,
    RECV_T_SINGLE,
    RECV_T_BIND,
    RECV_T_END
}RECV_TYPE_E;

struct RECV_ATTR_S
{
    struct timeval          tmRecvTO;
    unsigned int            uiRetry;
    struct
    {
        void                *pfn;
        void                *pUserdata;
    }Callback;
    
    //  bKeepTargetSockOpen = TRUE, the received socket file description won't be closed.
    //  Otherwise it will be close after the receive process.
    BOOL                    bKeepTargetSockOpen;
    
    //  Read only variables.
    //  The following variables are set by program.
    //  It is used to display the information only.
    RECV_TYPE_E             RecvType;
    struct sockaddr_in      stBindaddr;     //  Only available when RecvType is RECV_T_BIND.
};

struct RECV_S
{
    int                 iSocket;    
    BOOL                bStart;            //  Only available when RecvType is RECV_T_BIND.
    pthread_t           tThread;
    fd_set              fdRecv;
    struct RECV_ATTR_S  stAttr;
};

//
//  Callback
//
struct RECV_DATA_INFO_S
{
    time_t                      RecvTime;
    struct sockaddr_in          ClientAddr;
    int                         iSockClient;
    RECV_RET                    Ret;
    union
    {
        struct 
        {
            unsigned int        uiDataNum;
            unsigned char       szData[MAX_BUF_LEN];
            BOOL                bPacketEnd;
        };
        struct
        {
            int                 Errno;  //  The system error number.
        };
    };
};
typedef void (*FN_CALLBACK)(struct RECV_DATA_INFO_S *pRecv, void *pUserdata);

RECV_RET Recv_open(struct RECV_S *pstSess, 
                  char *pszBindAddr,
                  unsigned long ulPort,
                  struct RECV_ATTR_S *pstAttr);
RECV_RET Recv_close(struct RECV_S *pstSess);
RECV_RET Recv_start(struct RECV_S *pstSess);
RECV_RET Recv_stop(struct RECV_S *pstSess);

//  This function is used to receive data through iSocket and the received information
//  is saved in the pRecv structure. The pRecv->bPacketEnd flag tells the receive
//  process is finished or not.
//  Notes that, the iSocket won't be closed even the receive process is finished.
RECV_RET Recv_recv(struct RECV_S *pstSess, 
              int iSocket,
              struct RECV_ATTR_S *pstAttr,
              struct RECV_DATA_INFO_S *pRecv);
RECV_RET Recv_getAttr(struct RECV_S *pstSess, struct RECV_ATTR_S *pstAttr);
//  Description:
//      Get this software version.
//  i/p:
//      none
//  o/p:
//      The version.
const RECEIVER_VERSION_ST* Recv_GetVersion();
#endif  //  __AL_NETWORK_RECEIVER_LIB_H__
