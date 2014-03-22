c_network_libreceiver
=====================

Release Note:
    V-1.0.0.0
        #    Release date: 2014-01-26
        #    Enhancements and Improvements:
            #    Implement TCP socket to receive data.

    V-1.0.1.0
        #    Release date: 2014-01-26
        #    Enhancements and Improvements:
            #    Support Windows OS platform.
                #    Add /D "OS_WINDOWS" to command line

    V-2.0.1.2
        #    Release date: 2014-02-27
        #    Enhancements and Improvements:
            #   Add thread function.
            #   Add callback function.
                #   The callback function returns the error code or the received data.
            #   Add Recv_recv(): To receive the single connection data.
            #   Add Recv_getAttr() : To get the receiver attribute.
    V-2.0.1.3
        #   Resolved BOOL type redefined complier error.
    V-2.0.1.4
        #   Resolve accepted socket description is closed that cause the caller callback function
            can not send package back to client issue.
        #   struct RECV_ATTR_S *pstAttr parameter can be NULL in Recv_recv() but can't in Recv_open().