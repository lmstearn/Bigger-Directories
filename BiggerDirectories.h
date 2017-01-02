//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by BiggerDirectories.rc
//
#define arraysize                       40
#define IDD_SMALL                       101
#define IDD_768P                        102
#define IDD_1080P                       103
#define IDD_2160P                       104
#define IDD_4320P                       105
#define IDD_SMALLW                      106
#define IDD_768PW                       106
#define IDD_1080PW                      107
#define IDD_2160PW                      108
#define IDD_4320PW                      109
#define IDD_HELP768P                    110
#define IDD_HELP1080P                   111
#define IDD_HELP2160P                   112
#define IDD_HELP4320P                   113
#define IDB_PENCIL                      130
#define IDB_CRAYON                      132
#define IDB_MARKER                      134
#define IDB_PEN                         136
#define IDB_FORK                        138
#define IDC_TEXT                        1000
#define IDC_NUMBER                      1001
#define IDC_LIST                        1002
#define IDC_ADD                         1003
#define IDC_UP							1004
#define IDC_DOWN						1005
#define IDC_CREATE                      1006
#define IDC_CLEAR                       1007
#define IDC_REMOVE                      1008
#define IDC_LOGON                       1009
#define IDC_NOLOGON                     1010
#define IDC_HALP                        1011
#define IDC_SHOWCOUNT                   1012
#define IDC_LIST2                       1013
#define IDC_STATIC_SMALL                1014
#define IDC_STATIC_ZERO                 1015
#define IDC_STATIC_ONE                  1016
#define IDC_STATIC_TWO                  1017
#define IDC_STATIC_THREE                1018
#define IDC_STATIC_FOUR                 1019
#define IDC_STATIC_FIVE                 1020
#define IDC_OK                          1021
#define IDC_RES                         1022
#define IDI_APP_ICON                    1023
#define IDW_CLICK                       1024
#define IDC_STATIC                      -1

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

//for _TRUNCATE to wrok in _snwprintf_s
#ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#endif
#ifndef  _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#endif

// Start of HyperLink URL //
#define PROP_ORIGINAL_FONT      L"_Hyperlink_Original_Font_"
#define PROP_ORIGINAL_PROC      L"_Hyperlink_Original_Proc_"
#define PROP_STATIC_HYPERLINK   L"_Hyperlink_From_Static_"
#define PROP_UNDERLINE_FONT     L"_Hyperlink_Underline_Font_"
// End of HyperLink URL */

#pragma once
#pragma comment( lib, "Winmm.lib" )
//#pragma warning(disable: <warning_code>)

#ifndef NTDDI_WINXPSP3
#define NTDDI_WINXPSP3 0x05010300
#endif 



//check properties/c++/preprocessor defs are _Win64
#if _WIN32 || _WIN64
   #if _WIN64
     #define ENV64BIT
  #else
    #define ENV32BIT
  #endif
#endif

#ifndef UNICODE
#define UNICODE
#elif !defined(_UNICODE)
#define _UNICODE
#endif

//service pack info
#define PACKVERSION(major,minor) MAKELONG(minor,major)

//SEH
#define _CRT_SECURE_NO_WARNINGS

//G++ compatibility ...
#define MSGFLT_ALLOW                            (1)
#undef MSGFLT_ADD
#define MSGFLT_ADD                              (1)


// Next default values for new objects

#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        30
#define _APS_NEXT_COMMAND_VALUE         40001
#define _APS_NEXT_CONTROL_VALUE         1025
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif

//#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
//NT_ERROR(ntStatus)) bad practice to define consts here (performance)
#define STATUS_OBJECT_NAME_INVALID			((NTSTATUS)0xC0000033L)
#define STATUS_OBJECT_NAME_COLLISION		((NTSTATUS)0xC0000035L)
#define STATUS_OBJECT_PATH_INVALID			((NTSTATUS)0xC0000039L)
#define STATUS_OBJECT_PATH_SYNTAX_BAD		((NTSTATUS)0xC000003BL)
#define STATUS_DATA_OVERRUN					((NTSTATUS)0xC000003CL)
#define STATUS_DATA_LATE_ERROR				((NTSTATUS)0xC000003DL)
#define STATUS_DATA_ERROR					((NTSTATUS)0xC000003EL)
#define STATUS_CRC_ERROR					((NTSTATUS)0xC000003FL)
#define STATUS_NO_GUID_TRANSLATION			((NTSTATUS)0xC000010CL)
#define STATUS_OPEN_FAILED					((NTSTATUS)0xC0000136L)
#define STATUS_NOT_A_REPARSE_POINT			((NTSTATUS)0xC0000275L)
#define STATUS_DIRECTORY_IS_A_REPARSE_POINT	((NTSTATUS)0xC0000281L)
#define STATUS_CANNOT_MAKE					((NTSTATUS)0xC00002EAL)
					
						



#ifndef WINDOWS_NTSTATUS_H
#define WINDOWS_NTSTATUS_H

/**
* @author Roger Karlsson
* @since 2009-03-13
*/
//First (highest order) 2 bits of 32 bit NTSATUS = severity code
//3rd  bit is customer code
//4 - 16 is "Facility": what facility generated the error
//17-32 is actual error code of above facility
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0) //((NTSTATUS)(Status) >= 0) is function
#endif
#ifndef NT_INFORMATION
#define NT_INFORMATION(Status) (((ULONG)(Status) >> 30) == 1) //easiest to bitwise shift 30 to get severity code
#endif
#ifndef NT_WARNING
#define NT_WARNING(Status) (((ULONG)(Status) >> 30) == 2)
#endif
#ifndef NT_ERROR
#define NT_ERROR(Status) (((ULONG)(Status) >> 30) == 3)
#endif

#endif //WINDOWS_NTSTATUS_H





/*
The remainder of this header is for the IsWindowsVistaOrGreater function
...
...
...

Copyright (c) Microsoft Corporation. All rights reserved.
Module Name:
    winapifamily.h
Abstract:
    Master include file for API family partitioning.
*/

#ifndef _INC_WINAPIFAMILY
#define _INC_WINAPIFAMILY

#if defined(_MSC_VER) && !defined(MOFCOMP_PASS)
#pragma once
#endif // defined(_MSC_VER) && !defined(MOFCOMP_PASS)

/*
 *  Windows APIs can be placed in a partition represented by one of the below bits.   The 
 *  WINAPI_FAMILY value determines which partitions are available to the client code.
 */

#define WINAPI_PARTITION_DESKTOP   0x00000001
#define WINAPI_PARTITION_APP       0x00000002    

/*
 * A family may be defined as the union of multiple families. WINAPI_FAMILY should be set
 * to one of these values.
 */
#define WINAPI_FAMILY_APP          WINAPI_PARTITION_APP
#define WINAPI_FAMILY_DESKTOP_APP  (WINAPI_PARTITION_DESKTOP | WINAPI_PARTITION_APP)    

/*
 * A constant that specifies which code is available to the program's target runtime platform.
 * By default we use the 'desktop app' family which places no restrictions on the API surface. 
 * To restrict the API surface to just the App API surface, define WINAPI_FAMILY to WINAPI_FAMILY_APP.
 */
#ifndef WINAPI_FAMILY
#define WINAPI_FAMILY WINAPI_FAMILY_DESKTOP_APP
#endif

/* Macro to determine if a partition is enabled */
#define WINAPI_FAMILY_PARTITION(Partition)	((WINAPI_FAMILY & Partition) == Partition)

/* Macro to determine if only one partition is enabled from a set */
#define WINAPI_FAMILY_ONE_PARTITION(PartitionSet, Partition) ((WINAPI_FAMILY & PartitionSet) == Partition)

/*
 * Macro examples:
 *    The following examples are typical macro usage for enabling/restricting access to header code based
 *    on the target runtime platform. The examples assume a correct setting of the WINAPI_FAMILY macro.
 *
 *      App programs:
 *          Explicitly set WINAPI_FAMILY to WINAPI_PARTITION_APP (cannot access DESKTOP partition)
 *      Desktop programs:
 *          Leave WINAPI_FAMILY set to the default above (currently WINAPI_FAMILY_DESKTOP_APP)
 *
 *      Note: Other families and partitions may be added in the future.
 *
 *
 * The WINAPI_FAMILY_PARTITION macro:
 *    Code-block is available to programs that have access to specified partition.
 *
 *      Example: Available to App and Desktop programs
 *          #if WINAPI_FAMILY_PARTITION( WINAPI_PARTITION_APP )
 *
 *      Example: Available to Desktop programs
 *          #if WINAPI_FAMILY_PARTITION( WINAPI_PARTITION_DESKTOP )
 *
 *
 * The WINAPI_FAMILY_ONE_PARTITION macro:
 *    Code-block is available to programs that have access to specified parition, but not others in the partition set.
 *
 *      Example: Available only to App programs
 *          #if WINAPI_FAMILY_ONE_PARTITION( WINAPI_FAMILY, WINAPI_PARTITION_APP )
 *
 *      Example: Available only to Desktop programs
 *          #if WINAPI_FAMILY_ONE_PARTITION( WINAPI_FAMILY, WINAPI_PARTITION_DESKTOP )
 *
 *      Example: Available to App, but not Desktop programs
 *          #if WINAPI_FAMILY_ONE_PARTITION( WINAPI_FAMILY_DESKTOP_APP, WINAPI_PARTITION_APP )
 */

#endif  /* !_INC_WINAPIFAMILY */
/******************************************************************
*                                                                 *
*  VersionHelpers.h -- This module defines helper functions to    *
*                      promote version check with proper          *
*                      comparisons.                               *
*                                                                 *
*  Copyright (c) Microsoft Corp.  All rights reserved.            *
*                                                                 *
******************************************************************/

#ifndef _versionhelpers_H_INCLUDED_
#define _versionhelpers_H_INCLUDED_

#ifndef XP_BUILD
//#include "winapifamily.h"
#define _WIN32_WINNT_WIN8                   0x0602
#else
#define WINAPI_PARTITION_DESKTOP   (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#define WINAPI_FAMILY WINAPI_FAMILY_DESKTOP_APP
#define WINAPI_FAMILY_PARTITION(Partitions)     (Partitions)

#define _WIN32_WINNT_NT4                    0x0400
#define _WIN32_WINNT_WIN2K                  0x0500
#define _WIN32_WINNT_WINXP                  0x0501
#define _WIN32_WINNT_WS03                   0x0502
#define _WIN32_WINNT_WIN6                   0x0600
#define _WIN32_WINNT_VISTA                  0x0600
#define _WIN32_WINNT_WS08                   0x0600
#define _WIN32_WINNT_LONGHORN               0x0600
#define _WIN32_WINNT_WIN7                   0x0601
#define _WIN32_WINNT_WIN8                   0x0602
#endif

#ifdef _MSC_VER
#pragma once
#endif  // _MSC_VER

#pragma region Application Family

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#include <specstrings.h>    // for _In_, etc.

#if !defined(__midl) && !defined(SORTPP_PASS)

#if (NTDDI_VERSION >= NTDDI_WINXP)

#ifdef __cplusplus

#define VERSIONHELPERAPI inline bool

#else  // __cplusplus

#define VERSIONHELPERAPI FORCEINLINE BOOL

#endif // __cplusplus

#define _WIN32_WINNT_WINBLUE                0x0603
#define _WIN32_WINNT_WIN10                  0x0A00

typedef NTSTATUS( NTAPI* fnRtlGetVersion )(PRTL_OSVERSIONINFOW lpVersionInformation);

VERSIONHELPERAPI
IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
{
    /*OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0 };
    DWORDLONG        const dwlConditionMask = VerSetConditionMask(
    VerSetConditionMask(
    VerSetConditionMask(
    0, VER_MAJORVERSION, VER_GREATER_EQUAL),
    VER_MINORVERSION, VER_GREATER_EQUAL),
    VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
    osvi.dwMajorVersion = wMajorVersion;
    osvi.dwMinorVersion = wMinorVersion;
    osvi.wServicePackMajor = wServicePackMajor;
    return VerifyVersionInfoW(&osvi, VER_MAJORVERSION | VER_MINORVERSION | VER_SERVICEPACKMAJOR, dwlConditionMask) != FALSE;*/

    RTL_OSVERSIONINFOEXW verInfo = { 0 };
    verInfo.dwOSVersionInfoSize = sizeof( verInfo );

    static auto RtlGetVersion = (fnRtlGetVersion)GetProcAddress( GetModuleHandleW( L"ntdll.dll" ), "RtlGetVersion" );

    if (RtlGetVersion != 0 && RtlGetVersion( (PRTL_OSVERSIONINFOW)&verInfo ) == 0)
    {
        if (verInfo.dwMajorVersion > wMajorVersion)
            return true;
        else if (verInfo.dwMajorVersion < wMajorVersion)
            return false;

        if (verInfo.dwMinorVersion > wMinorVersion)
            return true;
        else if (verInfo.dwMinorVersion < wMinorVersion)
            return false;

        if (verInfo.wServicePackMajor >= wServicePackMajor)
            return true;
    }

    return false;
}

VERSIONHELPERAPI
IsWindowsXPOrGreater()
{
    return IsWindowsVersionOrGreater( HIBYTE( _WIN32_WINNT_WINXP ), LOBYTE( _WIN32_WINNT_WINXP ), 0 );
}

VERSIONHELPERAPI
IsWindowsXPSP1OrGreater()
{
    return IsWindowsVersionOrGreater( HIBYTE( _WIN32_WINNT_WINXP ), LOBYTE( _WIN32_WINNT_WINXP ), 1 );
}

VERSIONHELPERAPI
IsWindowsXPSP2OrGreater()
{
    return IsWindowsVersionOrGreater( HIBYTE( _WIN32_WINNT_WINXP ), LOBYTE( _WIN32_WINNT_WINXP ), 2 );
}

VERSIONHELPERAPI
IsWindowsXPSP3OrGreater()
{
    return IsWindowsVersionOrGreater( HIBYTE( _WIN32_WINNT_WINXP ), LOBYTE( _WIN32_WINNT_WINXP ), 3 );
}

VERSIONHELPERAPI
IsWindowsVistaOrGreater()
{
    return IsWindowsVersionOrGreater( HIBYTE( _WIN32_WINNT_VISTA ), LOBYTE( _WIN32_WINNT_VISTA ), 0 );
}

VERSIONHELPERAPI
IsWindowsVistaSP1OrGreater()
{
    return IsWindowsVersionOrGreater( HIBYTE( _WIN32_WINNT_VISTA ), LOBYTE( _WIN32_WINNT_VISTA ), 1 );
}

VERSIONHELPERAPI
IsWindowsVistaSP2OrGreater()
{
    return IsWindowsVersionOrGreater( HIBYTE( _WIN32_WINNT_VISTA ), LOBYTE( _WIN32_WINNT_VISTA ), 2 );
}

VERSIONHELPERAPI
IsWindows7OrGreater()
{
    return IsWindowsVersionOrGreater( HIBYTE( _WIN32_WINNT_WIN7 ), LOBYTE( _WIN32_WINNT_WIN7 ), 0 );
}

VERSIONHELPERAPI
IsWindows7SP1OrGreater()
{
    return IsWindowsVersionOrGreater( HIBYTE( _WIN32_WINNT_WIN7 ), LOBYTE( _WIN32_WINNT_WIN7 ), 1 );
}

VERSIONHELPERAPI
IsWindows8OrGreater()
{
    return IsWindowsVersionOrGreater( HIBYTE( _WIN32_WINNT_WIN8 ), LOBYTE( _WIN32_WINNT_WIN8 ), 0 );
}

VERSIONHELPERAPI
IsWindows8Point1OrGreater()
{
    return IsWindowsVersionOrGreater( HIBYTE( _WIN32_WINNT_WINBLUE ), LOBYTE( _WIN32_WINNT_WINBLUE ), 0 );
}

VERSIONHELPERAPI
IsWindows10OrGreater()
{
    return IsWindowsVersionOrGreater( HIBYTE( _WIN32_WINNT_WIN10 ), LOBYTE( _WIN32_WINNT_WIN10 ), 0 );
}


VERSIONHELPERAPI
IsWindowsServer()
{
    OSVERSIONINFOEXW osvi = { sizeof( osvi ), 0, 0, 0, 0, { 0 }, 0, 0, 0, VER_NT_WORKSTATION };
    DWORDLONG        const dwlConditionMask = VerSetConditionMask( 0, VER_PRODUCT_TYPE, VER_EQUAL );

    return !VerifyVersionInfoW( &osvi, VER_PRODUCT_TYPE, dwlConditionMask );
}

#endif // NTDDI_VERSION

#endif // defined(__midl)

#endif /* WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP) */

#pragma endregion

#endif // _VERSIONHELPERS_H_INCLUDED_
