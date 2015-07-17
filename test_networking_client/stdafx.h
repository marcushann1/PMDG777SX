// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#ifndef WINVER
#define WINVER 0x0600
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif
#ifndef NTDDI_VERSION
#define NTDDI_VERSION 0x06000000
#endif

#pragma once

#include <SdkDdkver.h>

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: reference additional headers your program requires here
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <process.h>
//#include <winsock.h>
//#include <In6addr.h>
#include <Ws2tcpip.h>
#include <WinSock2.h>