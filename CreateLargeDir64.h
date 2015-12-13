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
#define IDC_CREATE                      1004
#define IDC_CLEAR                       1005
#define IDC_REMOVE                      1006
#define IDC_LOGON                       1007
#define IDC_NOLOGON                     1008
#define IDC_SHOWCOUNT                   1009
#define IDC_LIST2                       1010
#define IDC_STATIC_ONE                  1011
#define IDC_STATIC_TWO                  1012
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
#define _APS_NEXT_CONTROL_VALUE         1013
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif

#ifndef WINDOWS_NTSTATUS_H
#define WINDOWS_NTSTATUS_H

/**
* @author Roger Karlsson
* @since 2009-03-13
*/

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#endif

#ifndef NT_ERROR
#define NT_ERROR(Status) ((ULONG)(Status) >> 30 == 3)
#endif

#endif //WINDOWS_NTSTATUS_H
