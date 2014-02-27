#include "libreceiver.h"

void recvRequest(void *pArgs)
{
    int                 iErrCode;
    struct SESSION_S    *pstSess;
    int                 iMaxFd;
    int                 iSelect;
    int                 iSockClient;
    struct sockaddr_in  ClientAddr;
    int                 nClientAddrLen = sizeof(struct sockaddr_in);
    int                 iRetry;
    char                szRecvBuf[MAX_BUF_LEN];
    int                 nRecv, nTotal;
    struct RECEIVE_S    stReceive;
    int                 iTempVal;
    
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
        return NET_RET_ERR_PARAM;
    pstSess = (struct SESSION_S *)pArgs;
    
    FD_ZERO(&pstSess->fdRecv);
	FD_SET(pstSess->iSocket, &pstSess->fdRecv);
	iMaxFd = pstSess->iSocket;
    
    while (1)
    {
        memset (&stReceive, 0, sizeof(struct RECEIVE_S));
        
        //  Select
        iSelect = select(iMaxFd + 1, &pstSess->fdRecv, NULL, NULL, NULL);
        stReceive.RecvTime = time(NULL);
        if (iSelect < 0)
        {
            DEBUG_PRINTF("%s(): select() error, errno:%d\n", __FUNCTION__, errno);
            stReceive.Ret = NET_RET_ERR_SOCKET;
            stReceive.Errno = errno;
            ((FN_CALLBACK)pstSess->pfnCallback)(&stReceive, pstSess->pUserdata);
            continue;
        }

        //  Accept
        iSockClient = accept(pstSess->iSocket, (struct sockaddr *) &stReceive.ClientAddr, (int *)&nClientAddrLen);
        if (iSockClient == -1)
        {
            DEBUG_PRINTF("%s(): accept() error, errno:%d\n", __FUNCTION__, errno);
            stReceive.Ret = NET_RET_ERR_SOCKET;
            stReceive.Errno = errno;
            ((FN_CALLBACK)pstSess->pfnCallback)(&stReceive, pstSess->pUserdata);
            continue;
        }
        
        //  Set receive timeout
#ifdef OS_LINUX
        if (setsockopt(pstSess->iSocket, SOL_SOCKET, SO_RCVTIMEO, 
                      (void *)&pstSess->tmRecvTO, (int)sizeof(struct timeval)) != 0)
#endif
#ifdef OS_WINDOWS
		if (setsockopt(pstSess->iSocket, SOL_SOCKET, SO_RCVTIMEO, 
                      (char *)&pstSess->tmRecvTO, (int)sizeof(struct timeval)) != 0)
#endif
        {
            DEBUG_PRINTF("%s(): setsockopt() error, errno:%d\n", __FUNCTION__, errno);
            stReceive.Ret = NET_RET_ERR_SOCKET;
            stReceive.Errno = errno;
            ((FN_CALLBACK)pstSess->pfnCallback)(&stReceive, pstSess->pUserdata);
            continue;
        }

        iRetry = 0;
        while (1)
        {
            memset (stReceive.szData, 0, MAX_BUF_LEN);
            stReceive.uiDataNum = recv (iSockClient, stReceive.szData, MAX_BUF_LEN, 0);
            if (stReceive.uiDataNum == 0)
            {
                //  No data
                DEBUG_PRINTF("%s(): Receive no data\n", __FUNCTION__);
                break;
            }
            else if (stReceive.uiDataNum < 0)
            {
                if ((errno == EINTR) && (iRetry < pstSess->uiRecvRetry))
                {
                    //  Error occur, retry!
                    iRetry++;
                    DEBUG_PRINTF("%s(): The %dth Retry\n", __FUNCTION__, iRetry);
                    continue;
                }
                else
                {
                    stReceive.Ret = NET_RET_ERR_SOCKET;
                    stReceive.Errno = errno;
                    ((FN_CALLBACK)pstSess->pfnCallback)(&stReceive, pstSess->pUserdata);
                    DEBUG_PRINTF("%s(): recv() error, errno:%d\n", __FUNCTION__, errno);
                    break;
                }
            }
            else
            {
                if (stReceive.uiDataNum == MAX_BUF_LEN)
                {
                    ioctl(iSockClient, FIONREAD, &iTempVal);
                    if (iTempVal)
                        stReceive.bPacketEnd = FALSE;
                    else
                        stReceive.bPacketEnd = TRUE;
                    
                    iRetry = 0;
                    ((FN_CALLBACK)pstSess->pfnCallback)(&stReceive, pstSess->pUserdata);
                    DEBUG_PRINTF("%s(): %d\n", __FUNCTION__, __LINE__);
                }
                else
                {
                    stReceive.bPacketEnd = TRUE;
                    ((FN_CALLBACK)pstSess->pfnCallback)(&stReceive, pstSess->pUserdata);
                    DEBUG_PRINTF("%s(): %d\n", __FUNCTION__, __LINE__);
                    break;
                }
            }
        }   //  End of receive while loop.

        closesocket(iSockClient);
    }   //  End of main loop.
}

NET_RET Recv_open(struct SESSION_S *pstSess, 
                    char *pszIpv4Addr, 
                    unsigned long ulPort, 
                    struct timeval *ptmRecvTO, 
                    unsigned int uiRetry,
                    void *pfnCallback,
                    void *pUserdata)
{
#ifdef OS_WINDOWS
	WSADATA	WSAData = { 0 };
    if (WSAStartup(WSA_VERSION, &WSAData) != 0)
    {
        WSACleanup();
        return NET_RET_ERR_SOCKET;
    }
#endif

    if (pstSess == NULL)
        return NET_RET_ERR_PARAM;
    
    //  Bind ip in ipv4.
    pstSess->Bindaddr.sin_family = AF_INET;
    if ((pszIpv4Addr == NULL) || (strlen(pszIpv4Addr) < 8))
        return NET_RET_ERR_PARAM;
    pstSess->Bindaddr.sin_addr.s_addr = inet_addr(pszIpv4Addr);
    
    if (ulPort == 0)
        return NET_RET_ERR_PARAM;
    pstSess->Bindaddr.sin_port = htons(ulPort);
    
    //  Receive timeout
    if (ptmRecvTO != NULL)
    {
        pstSess->tmRecvTO.tv_sec = ptmRecvTO->tv_sec;
        pstSess->tmRecvTO.tv_usec = ptmRecvTO->tv_usec;
    }
    else
        memset(ptmRecvTO, 0, sizeof(struct timeval));
    
    pstSess->iSocket = -1;
    pstSess->uiRecvRetry = uiRetry;
    pstSess->pfnCallback = pfnCallback;
    pstSess->pUserdata = pUserdata;
    
    return NET_RET_SUCCESS;
}

NET_RET Recv_close(struct SESSION_S *pstSess)
{
    if (pstSess == NULL)
        return NET_RET_ERR_PARAM;
    
    if (pstSess->iSocket != -1)
        closesocket(pstSess->iSocket);

#ifdef OS_WINDOWS
	WSACleanup();
#endif

    return NET_RET_SUCCESS;
}

NET_RET Recv_start(struct SESSION_S *pstSess)
{
    NET_RET         Ret;
    int             iRet;
    BOOL	        bReuseAddr = TRUE;
    pthread_attr_t  stThreadAttr;
    
    if (pstSess == NULL)
        return NET_RET_ERR_PARAM;

    //  Create socket
    pstSess->iSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (pstSess->iSocket == -1)
        return NET_RET_ERR_SOCKET;
    
    //  Set socket option. Reuse address
    iRet = setsockopt(pstSess->iSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&bReuseAddr, sizeof(bReuseAddr));
    if (iRet == -1)
    {
        Ret = NET_RET_ERR_SOCKET;
        goto error_exit;
    }
    
    //  Bind the address and port
    if (bind(pstSess->iSocket, (struct sockaddr *)&pstSess->Bindaddr, sizeof(struct sockaddr_in)) == -1)
    {
        perror("bind():");
        Ret = NET_RET_ERR_SOCKET;
        goto error_exit;
    }
    
    //  Listen
    if (listen(pstSess->iSocket, SOMAXCONN) == -1)
    {
		Ret = NET_RET_ERR_SOCKET;
        goto error_exit;
    }
    
    //
    //  Create the thread.
    //  The thread is created as a detached thread with 512K stack size.
    //
    //  Initialize the thread attribute structure.
    if (pthread_attr_init(&stThreadAttr) != 0)
    {
        goto error_exit;
        Ret = NET_RET_ERR_THREAD;
    }
    
    //  Set thread stack size to 512K.
    if (pthread_attr_setstacksize(&stThreadAttr, 0X80000) != 0)
    {
        goto error_thread;
        Ret = NET_RET_ERR_THREAD;
    }
    
    //  Set thread attribute to detached.
    if (pthread_attr_setdetachstate(&stThreadAttr, PTHREAD_CREATE_DETACHED) != 0)
    {
        goto error_thread;
        Ret = NET_RET_ERR_THREAD;
    }
        
    if (pthread_create(&pstSess->tThread, &stThreadAttr, recvRequest, (void *)pstSess) != 0)
    {
        goto error_thread;
        Ret = NET_RET_ERR_THREAD;
    }
    
    return NET_RET_SUCCESS;
    
error_thread:
    pthread_attr_destroy(&stThreadAttr);
    
error_exit:
    if (pstSess->iSocket != -1)
        closesocket(pstSess->iSocket);
    
    return Ret;
}

NET_RET Recv_stop(struct SESSION_S *pstSess)
{
    if (pstSess == NULL)
        return NET_RET_ERR_PARAM;
    
    if (pstSess->iSocket == -1)
        return NET_RET_SUCCESS;
        
    pthread_cancel(pstSess->tThread);
    closesocket(pstSess->iSocket);
    
    return NET_RET_SUCCESS;
}
