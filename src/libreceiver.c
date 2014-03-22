#include "libreceiver.h"

//  o/p:
//      -1  : Error
//      0   : Success and there's no data left in the buffer.
//      1   : Success and there still has data in the buffer.
int recvSingleConn(struct RECV_DATA_INFO_S *pstReceive, int iSockClient, unsigned int nRetry)
{
    unsigned int    uiRetry = 0;
    int             nDataLeft;
    
    pstReceive->RecvTime = time(NULL);
    while (1)
    {
        memset (pstReceive->szData, 0, MAX_BUF_LEN);
        pstReceive->uiDataNum = recv (iSockClient, pstReceive->szData, MAX_BUF_LEN, 0);
        if (pstReceive->uiDataNum == 0)
        {
            //  No data
            DEBUG_PRINTF("%s(): %d, Receive no data.\n", __FUNCTION__, __LINE__);
            return 0;
        }
        else if (pstReceive->uiDataNum < 0)
        {
            if ((errno == EINTR) && (uiRetry < nRetry))
            {
                //  Error occur, retry!
                DEBUG_PRINTF("%s:%d, Retry: %d\n", __FILE__, __LINE__, uiRetry);
                uiRetry++;
                continue;
            }
            else
            {
                DEBUG_PRINTF("%s(): %d, errno: %d\n", __FUNCTION__, __LINE__, errno);
                pstReceive->Ret = RECV_RET_ERR_SOCKET_RECV;
                pstReceive->Errno = errno;
                return -1;
            }
        }
        else
        {
            if (pstReceive->uiDataNum == MAX_BUF_LEN)
            {
                DEBUG_PRINTF("%s(): %d\n", __FUNCTION__, __LINE__);
                ioctl(iSockClient, FIONREAD, &nDataLeft);
                if (nDataLeft)
                    return 1;
                else
                    return 0;
            }
            else
            {
                DEBUG_PRINTF("%s(): %d\n", __FUNCTION__, __LINE__);
                return 0;
            }
        }
    }   //  End of receive while loop.
}

//  The client socket file description doesn't be closed 
//  because the caller may needs it to send the data back that 
//  the caller must remember to close it.
void* recvThread(void *pArgs)
{
    int                     iRet;
    int                     iMaxFd;
    int                     iSelect;
    struct sockaddr_in      ClientAddr;
    int                     nClientAddrLen = sizeof(struct sockaddr_in);
    struct RECV_DATA_INFO_S stReceive;
    int                     iTempVal;
    struct RECV_S           *pstSess;
    struct RECV_ATTR_S      *pstAttr;
    
    //  Set the thread to cancelable.
    if (pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &iTempVal) != 0)
        pthread_exit(&errno);
    
    //  It uses PTHREAD_CANCEL_DEFERRED because the document said even using
    //  PTHREAD_CANCEL_ASYNCHRONOUS the system can't guarantee the thread can
    //  be terminal immediately. According to this I choose PTHREAD_CANCEL_DEFERRED
    //  because of the safity.
    if (pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &iTempVal) != 0)
        pthread_exit(&errno);
     
    if (pArgs == NULL)
    {
        RECV_RET    Ret = RECV_RET_ERR_PARAM;
        pthread_exit(&Ret);
    }
    pstSess = (struct RECV_S *)pArgs;
    pstAttr = &pstSess->stAttr;
    
	FD_SET(pstSess->iSocket, &pstSess->fdRecv);
	iMaxFd = pstSess->iSocket;
    
    while (1)
    {
        memset (&stReceive, 0, sizeof(struct RECV_DATA_INFO_S));
        
        //  Select
        iSelect = select(iMaxFd + 1, &pstSess->fdRecv, NULL, NULL, NULL);
        if (iSelect < 0)
        {
            DEBUG_PRINTF("%s: %d, errno:%d\n", __FILE__, __LINE__, errno);
            stReceive.Ret = RECV_RET_ERR_FILE_SELECT;
            stReceive.Errno = errno;
            ((FN_CALLBACK)pstAttr->Callback.pfn)(&stReceive, pstAttr->Callback.pUserdata);
            continue;
        }

        //  Accept
        stReceive.iSockClient = accept(pstSess->iSocket, (struct sockaddr *) &stReceive.ClientAddr, (int *)&nClientAddrLen);
        if (stReceive.iSockClient == -1)
        {
            DEBUG_PRINTF("%s: %d, errno:%d\n", __FILE__, __LINE__, errno);
            stReceive.Ret = RECV_RET_ERR_SOCKET_ACCEPT;
            stReceive.Errno = errno;
            ((FN_CALLBACK)pstAttr->Callback.pfn)(&stReceive, pstAttr->Callback.pUserdata);
            continue;
        }
        
        //  Set receive timeout
#ifdef OS_LINUX
        if (setsockopt(pstSess->iSocket, SOL_SOCKET, SO_RCVTIMEO, 
                      (void *)&pstAttr->tmRecvTO, (int)sizeof(struct timeval)) != 0)
#endif
#ifdef OS_WINDOWS
		if (setsockopt(pstSess->iSocket, SOL_SOCKET, SO_RCVTIMEO, 
                      (char *)&pstAttr->tmRecvTO, (int)sizeof(struct timeval)) != 0)
#endif
        {
            DEBUG_PRINTF("%s: %d, errno:%d\n", __FILE__, __LINE__, errno);
            closesocket(stReceive.iSockClient);
            stReceive.Ret = RECV_RET_ERR_SOCKET_OPTION;
            stReceive.Errno = errno;
            ((FN_CALLBACK)pstAttr->Callback.pfn)(&stReceive, pstAttr->Callback.pUserdata);
            continue;
        }

        iRet = 1;
        while (iRet > 0)
        {
            iRet = recvSingleConn(&stReceive, stReceive.iSockClient, pstAttr->uiRetry);
            if (iRet == 0)
                stReceive.bPacketEnd = TRUE;
            else if (iRet > 0)
                stReceive.bPacketEnd = FALSE;
            else
//              closesocket(stReceive.iSockClient);
            
            ((FN_CALLBACK)pstAttr->Callback.pfn)(&stReceive, pstAttr->Callback.pUserdata);
        }
    }   //  End of main loop.
}

RECV_RET Recv_open(struct RECV_S *pstSess, 
                   char *pszBindAddr,
                   unsigned long ulPort,
                   struct RECV_ATTR_S *pstAttr)
{
    if (pstSess == NULL)
        return RECV_RET_ERR_PARAM;
    
    if (pstAttr == NULL)
        return RECV_RET_ERR_PARAM;
    
    if ((pszBindAddr == NULL) || (ulPort == 0))
        return RECV_RET_ERR_PARAM;
    
    pstSess->iSocket = -1;
    pstSess->tThread = -1;
    pstSess->bStart = FALSE;
    FD_ZERO(&pstSess->fdRecv);
    
    memcpy(&pstSess->stAttr, pstAttr, sizeof(struct RECV_ATTR_S));
    pstSess->stAttr.RecvType = RECV_T_BIND;
    pstSess->stAttr.stBindaddr.sin_family = AF_INET;
    pstSess->stAttr.stBindaddr.sin_addr.s_addr = inet_addr(pszBindAddr);
    pstSess->stAttr.stBindaddr.sin_port = htons(ulPort);

#ifdef OS_WINDOWS
    WSADATA	WSAData = { 0 };
    if (WSAStartup(WSA_VERSION, &WSAData) != 0)
    {
        WSACleanup();
        return RECV_RET_ERR_SOCKET_INIT;
    }
#endif

    return RECV_RET_SUCCESS;
}

RECV_RET Recv_close(struct RECV_S *pstSess)
{
    RECV_RET    Ret;
    
    if (pstSess == NULL)
        return RECV_RET_ERR_PARAM;
    
    if (pstSess->bStart == TRUE)
        return RECV_RET_ERR_PROCEDURE;

#ifdef OS_WINDOWS
    WSACleanup();
#endif

    return RECV_RET_SUCCESS;
}

RECV_RET Recv_start(struct RECV_S *pstSess)
{
    RECV_RET        Ret;
    int             iRet;
    BOOL            bReuseAddr = TRUE;
    pthread_attr_t  stThreadAttr;
    
    if (pstSess == NULL)
        return RECV_RET_ERR_PARAM;

    if (pstSess->stAttr.RecvType != RECV_T_BIND)
        return RECV_RET_ERR_TYPE;
        
    //  Create socket
    pstSess->iSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (pstSess->iSocket == -1)
        return RECV_RET_ERR_SOCKET;

    //  Set socket option. Reuse address
    iRet = setsockopt(pstSess->iSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&bReuseAddr, sizeof(bReuseAddr));
    if (iRet == -1)
    {
        closesocket(pstSess->iSocket);
        return RECV_RET_ERR_SOCKET_OPTION;
    }

    //  Bind the address and port
    if (bind(pstSess->iSocket, (struct sockaddr *)&pstSess->stAttr.stBindaddr, sizeof(struct sockaddr_in)) == -1)
    {
        DEBUG_PRINTF("%s(): %d, errno: %d\n", __FUNCTION__, __LINE__, errno);
        perror("bind()");
        closesocket(pstSess->iSocket);
        return RECV_RET_ERR_SOCKET_BIND;
    }
    
    //  Listen
    if (listen(pstSess->iSocket, SOMAXCONN) == -1)
    {
        closesocket(pstSess->iSocket);
        Ret = RECV_RET_ERR_SOCKET_LISTEN;
    }
    
    //
    //  Create the thread.
    //  The thread is created as a detached thread with 512K stack size.
    //
    //  Initialize the thread attribute structure.
    if (pthread_attr_init(&stThreadAttr) != 0)
    {
        closesocket(pstSess->iSocket);
        return RECV_RET_ERR_THREAD_INIT;
    }
    
    //  Set thread stack size to 512K.
    if (pthread_attr_setstacksize(&stThreadAttr, 0X80000) != 0)
    {
        closesocket(pstSess->iSocket);
        return RECV_RET_ERR_THREAD_STACKSIZE;
    }
    
    //  Set thread attribute to detached.
    if (pthread_attr_setdetachstate(&stThreadAttr, PTHREAD_CREATE_DETACHED) != 0)
    {
        closesocket(pstSess->iSocket);
        return RECV_RET_ERR_THREAD_DETACHSTATE;
    }
        
    if (pthread_create(&pstSess->tThread, &stThreadAttr, recvThread, (void *)pstSess) != 0)
    {
        closesocket(pstSess->iSocket);
        return RECV_RET_ERR_THREAD_CREATE;
    }
    
    pstSess->bStart = TRUE;
    
    return RECV_RET_SUCCESS;
}

RECV_RET Recv_stop(struct RECV_S *pstSess)
{
    if (pstSess == NULL)
        return RECV_RET_ERR_PARAM;
    
    if (pstSess->bStart == FALSE)
        return RECV_RET_SUCCESS;
        
    pthread_cancel(pstSess->tThread);
    closesocket(pstSess->iSocket);
    
    pstSess->bStart = FALSE;
    
    return RECV_RET_SUCCESS;
}

RECV_RET Recv_recv(struct RECV_S *pstSess, 
              int iSocket,
              struct RECV_ATTR_S *pstAttr,
              struct RECV_DATA_INFO_S *pRecv)
{
    int                         iRet;
    struct RECV_DATA_INFO_S     stReceive;
    
    if (pstSess == NULL)
        return RECV_RET_ERR_PARAM;
    
    if (pstAttr == NULL)
        return RECV_RET_ERR_PARAM;

    if (iSocket == INVALID_SOCKET)
        return RECV_RET_ERR_PARAM;
    
    if (pstAttr == NULL)
        memset(&pstSess->stAttr, 0, sizeof(struct RECV_ATTR_S));
    else
        memcpy(&pstSess->stAttr, pstAttr, sizeof(struct RECV_ATTR_S));
    pstSess->stAttr.RecvType = RECV_T_SINGLE;

    //
    //  Start to receive
    //
    iRet = recvSingleConn(pRecv, iSocket, pstAttr->uiRetry);
    if (iRet == 0)
        pRecv->bPacketEnd = TRUE;
    else if (iRet > 0)
        pRecv->bPacketEnd = FALSE;
    
    return RECV_RET_SUCCESS;
}

RECV_RET Recv_getAttr(struct RECV_S *pstSess, struct RECV_ATTR_S *pstAttr)
{
    if (pstSess == NULL)
        return RECV_RET_ERR_PARAM;
    
    if (pstAttr == NULL)
        return RECV_RET_ERR_PARAM;
    
    memcpy(pstAttr, &pstSess->stAttr, sizeof(struct RECV_ATTR_S));
    
    return RECV_RET_SUCCESS;
}
