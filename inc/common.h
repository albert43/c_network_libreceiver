//	common.h - The common definition for all source.
//
//	Release	Note:
//		V-1.0.0:
//			Author : Albert Huang
//			Release Date: 2012-11-27
//				#	Windows like data type define.
//				#	Include useful header file.
//		V-2.0.0:
//			Author : Albert Huang
//			Release Date: 2012-11-27
//				#	Remove defition regarding network.

#ifndef	__ALRIGHT_COMMON_H__
#define	__ALRIGHT_COMMON_H__

 //////////////////////////////////////////////////////////////////////////////////////////
   ///   _____.....-------=========  Common Include  =========-------....._____      ///
    /////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>                //  standard buffered input/output 
#include <stdlib.h>               //  standard library definitions 
#include <string.h>               //  string operations 
#include <errno.h>                //  system error numbers
#include <fcntl.h>                //  file control options
#include <time.h>                 //  time types 
#ifdef OS_LINUX
#include <sys/types.h>            //  data types
#include <sys/stat.h>             //  data returned by the stat() function
#include <sys/wait.h>             //  declarations for waiting
#include <pthread.h>              //  threads
#include <semaphore.h>            //  semaphores (REALTIME)
#include <dlfcn.h>                //  dynamic linking
#include <sys/ioctl.h>
#include <sys/poll.h>
#endif
 //////////////////////////////////////////////////////////////////////////////////////////
   ////   _____.....-------=========  Common Define  =========-------....._____      ////
    /////////////////////////////////////////////////////////////////////////////////
//
//	Data Type Define.
//
typedef char *                          LPSTR;
typedef unsigned char                   BYTE;
typedef BYTE *                          LPBYTE;
typedef unsigned short int              WORD;
typedef WORD *                          LPWORD;
typedef unsigned long                   DWORD;
typedef DWORD *                         LPDWORD;
typedef void                            VOID;
typedef VOID *                          LPVOID;
typedef unsigned int                    UINT;
#ifdef OS_LINUX
typedef enum _BOOL
{
	FALSE = 0,
	TRUE
}BOOL;
#define DEBUG_PRINTF(fmt,ARG...)		printf(fmt,##ARG)
#else
#define DEBUG_PRINTF					printf
#endif
#endif	//	__ALRIGHT_COMMON_H__
