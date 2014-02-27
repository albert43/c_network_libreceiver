#include "network.h"

struct SESSION_S
{
    int					iSocket;    //  If iSocket is -1 means the receiver is stop, otherwise it's start.
	struct sockaddr_in	Bindaddr;
    struct timeval		tmRecvTO;
    unsigned int        uiRecvRetry;
    pthread_t           tThread;
	fd_set				fdRecv;
    void                *pfnCallback;
    void                *pUserdata;
};

struct RECEIVE_S
{
    time_t              RecvTime;
    struct sockaddr_in  ClientAddr;
    NET_RET             Ret;
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
typedef void (*FN_CALLBACK)(struct RECEIVE_S *pRecv, void *pUserdata);

NET_RET Recv_open(struct SESSION_S *pstSess, 
                  char *pszIpv4Addr, 
                  unsigned long ulPort, 
                  struct timeval *ptmRecvTO, 
                  unsigned int uiRetry,
                  void *pfnCallback,
                  void *pUserdata);
NET_RET Recv_close(struct SESSION_S *pstSess);
NET_RET Recv_start(struct SESSION_S *pstSess);
NET_RET Recv_stop(struct SESSION_S *pstSess);
