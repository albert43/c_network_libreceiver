#include "libreceiver.h"

void Display(struct RECV_DATA_INFO_S *pRecv, void *pUserdata)
{
    printf ("Here is Display()\n");
    printf ("Thread Ret:%d\n", pRecv->Ret);
    printf ("Time: %u\n", pRecv->RecvTime);
    printf ("Client address: %s:%d, Socket:%d\n", 
                inet_ntoa(pRecv->ClientAddr.sin_addr), 
                pRecv->ClientAddr.sin_port);
    printf ("Received %d bytes data(%s):\n%s\n", 
                pRecv->uiDataNum, 
                pRecv->bPacketEnd == TRUE ? "End" : "Not End", 
                pRecv->szData);
}

int main (void)
{
    RECV_RET            Ret;
    struct RECV_S       stRecv;
    struct RECV_ATTR_S  stAttr;
    
    stAttr.tmRecvTO.tv_sec = 5;
    stAttr.tmRecvTO.tv_usec = 0;
    stAttr.uiRetry = 5;
    stAttr.Callback.pfn = Display;
    stAttr.Callback.pUserdata = NULL;
    stAttr.bKeepTargetSockOpen = FALSE;
    
    Ret = Recv_open(&stRecv, "192.168.0.103", 14000, &stAttr);
    if (Ret != RECV_RET_SUCCESS)
        printf ("Recv_open() Failure. Ret=%d\n", Ret);
    Ret = Recv_start(&stRecv);
    if (Ret != RECV_RET_SUCCESS)
        printf ("Recv_start() Failure. Ret=%d\n", Ret);
    while (1);
    return 0;
}
