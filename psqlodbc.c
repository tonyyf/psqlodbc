/*--------
 * Module:			psqlodbc.c
 *
 * Description:		This module contains the main entry point (DllMain)
 *					for the library.  It also contains functions to get
 *					and set global variables for the driver in the registry.
 *
 * Classes:			n/a
 *
 * API functions:	none
 *
 * Comments:		See "notice.txt" for copyright and license information.
 *--------
 */

#include "psqlodbc.h"
#include "dlg_specific.h"
#include "environ.h"

#ifdef WIN32
#include <winsock.h>
#endif

GLOBAL_VALUES globals;

RETCODE SQL_API SQLDummyOrdinal(void);

#ifdef WIN32
HINSTANCE NEAR s_hModule;		/* Saved module handle. */
#ifdef	WIN_MULTITHREAD_SUPPORT
extern	CRITICAL_SECTION	qlog_cs, mylog_cs, conns_cs;
#endif /* WIN_MULTITHREAD_SUPPORT */

/*	This is where the Driver Manager attaches to this Driver */
BOOL		WINAPI
DllMain(HANDLE hInst, ULONG ul_reason_for_call, LPVOID lpReserved)
{
	WORD		wVersionRequested;
	WSADATA		wsaData;

	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			s_hModule = hInst;	/* Save for dialog boxes */

			/* Load the WinSock Library */
			wVersionRequested = MAKEWORD(1, 1);

			if (WSAStartup(wVersionRequested, &wsaData))
				return FALSE;

			/* Verify that this is the minimum version of WinSock */
			if (LOBYTE(wsaData.wVersion) != 1 ||
				HIBYTE(wsaData.wVersion) != 1)
			{
				WSACleanup();
				return FALSE;
			}

			INIT_QLOG_CS;
			INIT_MYLOG_CS;
			INIT_CONNS_CS;
			getCommonDefaults(DBMS_NAME, ODBCINST_INI, NULL);
			break;

		case DLL_THREAD_ATTACH:
			break;

		case DLL_PROCESS_DETACH:
			DELETE_CONNS_CS;
			DELETE_QLOG_CS;
			DELETE_MYLOG_CS;
			WSACleanup();
			return TRUE;

		case DLL_THREAD_DETACH:
			break;

		default:
			break;
	}

	return TRUE;

	UNREFERENCED_PARAMETER(lpReserved);
}

#else							/* not WIN32 */

#ifndef TRUE
#define TRUE	(BOOL)1
#endif
#ifndef FALSE
#define FALSE	(BOOL)0
#endif

#ifdef __GNUC__

/* This function is called at library initialization time.	*/

static BOOL
__attribute__((constructor))
init(void)
{
	getCommonDefaults(DBMS_NAME, ODBCINST_INI, NULL);
	return TRUE;
}

#else							/* not __GNUC__ */

/*
 * These two functions do shared library initialziation on UNIX, well at least
 * on Linux. I don't know about other systems.
 */
BOOL
_init(void)
{
	getCommonDefaults(DBMS_NAME, ODBCINST_INI, NULL);
	return TRUE;
}

BOOL
_fini(void)
{
	return TRUE;
}
#endif   /* not __GNUC__ */
#endif   /* not WIN32 */


/*
 *	This function is used to cause the Driver Manager to
 *	call functions by number rather than name, which is faster.
 *	The ordinal value of this function must be 199 to have the
 *	Driver Manager do this.  Also, the ordinal values of the
 *	functions must match the value of fFunction in SQLGetFunctions()
 */
RETCODE		SQL_API
SQLDummyOrdinal(void)
{
	return SQL_SUCCESS;
}
