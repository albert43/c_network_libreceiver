#ifndef	__ALRIGHT_NETWORK_H__
#define	__ALRIGHT_NETWORK_H__

#ifdef OS_LINUX
#include <sys/socket.h>           //  Internet Protocol family
#include <netdb.h>                //  definitions for network database operations 
#include <netinet/in.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>

#define INVALID_SOCKET				-1
#define SOCKET_ERROR				-1
#define	closesocket(sock)			close(sock)
#endif
#ifndef	__ALRIGHT_COMMON_H__
#include "common.h"
#endif
#ifdef OS_WINDOWS
#include <WinSock2.h>
#define WSA_VERSION MAKEWORD(2, 2)	// using winsock 2.2
#endif // OS_WINDOWS

typedef BYTE				NET_MAC[6];

// IPv4
typedef	struct _NET_IPv4
{
	union
	{
		BYTE				a_bIP [4];
		DWORD				dwIP;
		struct	in_addr		ip;
	};
} NET_IPv4;

typedef enum
{
	NET_RET_ERR_SYSTEM = -100,
    NET_RET_ERR_THREAD,
	NET_RET_ERR_SOCKET,
    NET_RET_ERR_INTERNAL,
	NET_RET_ERR_NOTFOUND,
    NET_RET_ERR_PARAM,
    NET_RET_ERR_PROCEDURE,
	NET_RET_SUCCESS = 0
}NET_RET;

#define	MAX_IF_LEN		8

#pragma pack(1)
typedef struct _ETHERNET_H
{
	NET_MAC					macDest;
	NET_MAC					macSrc;
	WORD					wType;
}ETHERNET_H;

typedef struct _IPv4_H
{
	BYTE					btV_IHL;
	BYTE					btTOS;		//	Type of Service
	WORD					wLength;	//	Total length
	WORD					wIdentify;
	WORD					wFlag_Frag;	//	4 bits Flags and 12 bits Fragment offset.
	BYTE					btTTL;		//	Time to Live
	BYTE					btProtocol;
	WORD					wChecksum;	//	Header checksum
	NET_IPv4				ipv4Src;
	NET_IPv4				ipv4DestIp;
}IPv4_H;
#pragma pack()

#define	MAX_BUF_LEN		2048
#endif	//	__ALRIGHT_NETWORK_H__