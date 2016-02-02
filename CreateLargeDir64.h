//{{NO_DEPENDENCIES}}
// Microsoft Visual C++ generated include file.
// Used by CreateLargeDir64.rc
//
#define arraysize                       30
#define IDD_MAIN                        101
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
#define IDC_SHOWCOUNT                   1011
#define IDC_LIST2                       1012
#define IDC_STATIC_ONE                  1013
#define IDC_STATIC_TWO                  1014
#define IDC_STATIC                      -1

//check properties/c++/preprocessor defs are _Win64
#if _WIN32 || _WIN64
   #if _WIN64
     #define ENV64BIT
  #else
    #define ENV32BIT
  #endif
#endif


// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        102
#define _APS_NEXT_COMMAND_VALUE         40001
#define _APS_NEXT_CONTROL_VALUE         1015
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif


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
