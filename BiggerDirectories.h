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
#define IDD_HELP768P                    106
#define IDD_HELP1080P                   107
#define IDD_HELP2160P                   108
#define IDD_HELP4320P                   109
#define IDB_PENCIL                      113
#define IDB_CRAYON                      115
#define IDB_MARKER                      117
#define IDB_PEN                         119
#define IDB_FORK                        121
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


// Start of HyperLink URL //
#define PROP_ORIGINAL_FONT      L"_Hyperlink_Original_Font_"
#define PROP_ORIGINAL_PROC      L"_Hyperlink_Original_Proc_"
#define PROP_STATIC_HYPERLINK   L"_Hyperlink_From_Static_"
#define PROP_UNDERLINE_FONT     L"_Hyperlink_Underline_Font_"
// End of HyperLink URL */

#pragma once
#pragma comment( lib, "Winmm.lib" )

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
//BOOL (WINAPI * ChangeWindowMessageFilterEx)(HWND,UINT,DWORD) = NULL;


// Next default values for new objects
// 
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
