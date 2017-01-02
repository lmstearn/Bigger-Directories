#include <stdio.h> //sprintf
#include <stdlib.h> //malloc
#include <fcntl.h>
#include <io.h> //setmode
#include <shlwapi.h>
#include <sddl.h>
#include <windows.h>
#include <strsafe.h> //safe string copy & StringCchPrintf
#include <tlhelp32.h> //Find process stuff
#include <winternl.h> //NtCreateFile
#include <errno.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "BiggerDirectories.h" //my file

//#include <afxwin.h>
//#include <ntstatus.h>
//#include <ntstrsafe.h>






wchar_t hrtext[256] = {'\0'}; //An array name is essentially a pointer to the first element in an array.
wchar_t hrWarn[8] = L"Warning";
WIN32_FIND_DATAW dw; // directory data this will use stack memory as opposed to LPWIN32_FIND_DATA
WIN32_FIND_DATAA da;
int const pathLength = 32759, maxDWORD = 32767, maxPathFolder = MAX_PATH - 3, treeLevelLimit = 2000, branchLimit = 1000;
const wchar_t BOM = L'\xFEFF'; //65279
wchar_t const *invalidPathName = L":\"/\\|?*<>";
wchar_t const eolFTA = L'\n';
wchar_t const separatorFTA = L'\\';
wchar_t const *lPref = L"\\\\?\\";
//wchar_t const * hrWarn = L"Warning";
wchar_t const APP_CLASS_NAME[]  = L"BiggerDirectories";

wchar_t driveInfo[26][2], driveIndex[2];
wchar_t driveIDBaseW[8], driveIDBaseWNT[8], driveIDBaseAW[4];
char driveIDBase[4];


UINT const WM_COPYGLOBALDATA = 0x0049; //Drop files filter

wchar_t rootDir [pathLength], dblclkPath [treeLevelLimit + 1][maxPathFolder], dblclkString [pathLength], reorgTmpW[pathLength];//maxPathFolder unless delete fails
wchar_t *pathToDeleteW, *currPathW, *findPathW, *tempDest, *thisexePath, *BiggerDirectoriesVAR; // directory pointers. cannot be initialised as a pointer
char *currPath;
//http://stackoverflow.com/questions/2516096/fastest-way-to-zero-out-a-2d-array-in-c
char dacfolders[branchLimit][MAX_PATH-3]; //[32768 / 257] [ MAX_PATH- 3] double array char is triple array
wchar_t dacfoldersW[branchLimit][MAX_PATH-3], dacfoldersWtmp[branchLimit][maxPathFolder], folderTreeArray[branchLimit][treeLevelLimit + 1][maxPathFolder] = { NULL };
wchar_t reorgTmpWFS[treeLevelLimit + 1][maxPathFolder], pathsToSave [branchLimit][pathLength];


int rootFolderCS, rootFolderCW, branchLevel, branchTotal, branchLevelCum, branchLevelClickOld, branchLevelClick, branchTotalSaveFile, branchLevelInc, branchLevelIncCum, branchSaveI, branchTotalCum, branchTotalCumOld, dblclkLevel = 0;
int i,j,k, errCode, verifyFail;
int idata, index, folderIndex, listTotal = 0, sendMessageErr = 0;
int treeLevel, trackFTA[branchLimit][2];
int resResult;
bool resWarned;
bool wideScr = true;
bool foundResolution = false;
bool pCmdLineActive = false;
bool secondTryDelete = false;
bool createFail = false;
bool setforDeletion = false;
bool removeButtonEnabled = true;
bool nologonEnabled = false;
bool logonEnabled = false;
bool wow64Functions = false;
BOOL weareatBoot = FALSE;
BOOL am64Bit, exe64Bit;
PVOID OldValue = nullptr; //Redirection
WNDPROC g_pOldProc;
HANDLE keyHwnd, hMutex, hdlNtCreateFile, hdlNTOut, exeHandle, ds;     // directory handle
HINSTANCE appHinstance;
HDROP hDropInfo = NULL; //shell drop handle

//struct FolderRepository
//{
//    char FT[treeLevelLimit][branchLimit][maxPathFolder];
	//char FB[1000];

//};

//NTDLLptr is a pointer to a function returning LONG or NTSTATUS
typedef NTSTATUS (__stdcall *NTDLLptr)(
	OUT PHANDLE FileHandle, 
	IN ACCESS_MASK DesiredAccess, 
	IN POBJECT_ATTRIBUTES ObjectAttributes, 
	OUT PIO_STATUS_BLOCK IoStatusBlock, 
	IN PLARGE_INTEGER AllocationSize,
	IN ULONG FileAttributes, 
	IN ULONG ShareAccess, 
	IN ULONG CreateDisposition, 
	IN ULONG CreateOptions, 
	IN PVOID EaBuffer, 
	IN ULONG EaLength );


//for NTcreatefile fileObject,  NTAPI is __stdcall
typedef VOID (__stdcall *PFN_RtlInitUnicodeString) (
    IN OUT PUNICODE_STRING  DestinationString,
    IN PCWSTR  SourceString );
typedef ULONG (__stdcall *PFN_RtlNtStatusToDosError) (
    IN NTSTATUS Status );
//static my_RtlInitUnicodeString rtlInitUnicodeString; //Makes no difference
//PFN_RtlNtStatusToDosError RtlNtStatusToDosError;


NTDLLptr foundNTDLL = nullptr; //points to return of NTStatus
OBJECT_ATTRIBUTES fileObject;
IO_STATUS_BLOCK ioStatus;
NTSTATUS ntStatus = NULL;
UNICODE_STRING fn;
const char createFnString[13] = "NtCreateFile"; //one extra for null termination
const char initUnicodeFnString[21] = "RtlInitUnicodeString";
const char NtStatusToDosErrorString[22] = "RtlNtStatusToDosError";
const wchar_t TEMP_CLASS_NAME[14]  = L"ResCheckClass";
//A pathname MUST be no more than 32,760 characters in length. (ULONG) Each pathname component MUST be no more than 255 characters in length (USHORT)
//wchar_t longPathName=(char)0;  //same as '\0'


class APP_CLASS
{
public:
APP_CLASS();

// This is the static callback that we register
static INT_PTR CALLBACK s_DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// The static callback recovers the "this" pointer and then calls this member function.
INT_PTR DlgProc(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

};


APP_CLASS::APP_CLASS(void)
{

		switch (resResult)
	{
		case 1:
			if (wideScr)
			{
			DialogBoxParamW(appHinstance, MAKEINTRESOURCEW(IDD_4320PW), nullptr, APP_CLASS::s_DlgProc, reinterpret_cast<LPARAM>(this));
			}
			else
			{
			DialogBoxParamW(appHinstance, MAKEINTRESOURCEW(IDD_4320P), nullptr, APP_CLASS::s_DlgProc, reinterpret_cast<LPARAM>(this));
			}
		break;
		case 2:
			if (wideScr)
			{
			DialogBoxParamW(appHinstance, MAKEINTRESOURCEW(IDD_2160PW), nullptr, APP_CLASS::s_DlgProc, reinterpret_cast<LPARAM>(this));
			}
			else
			{
			DialogBoxParamW(appHinstance, MAKEINTRESOURCEW(IDD_2160P), nullptr, APP_CLASS::s_DlgProc, reinterpret_cast<LPARAM>(this));
			}
		break;
		case 3:
			if (wideScr)
			{
			DialogBoxParamW(appHinstance, MAKEINTRESOURCEW(IDD_1080PW), nullptr, APP_CLASS::s_DlgProc, reinterpret_cast<LPARAM>(this));
			}
			else
			{
			DialogBoxParamW(appHinstance, MAKEINTRESOURCEW(IDD_1080P), nullptr, APP_CLASS::s_DlgProc, reinterpret_cast<LPARAM>(this));
			}
		break;
		case 4:
			if (wideScr)
			{
			DialogBoxParamW(appHinstance, MAKEINTRESOURCEW(IDD_768PW), nullptr, APP_CLASS::s_DlgProc, reinterpret_cast<LPARAM>(this));
			}
			else
			{
			DialogBoxParamW(appHinstance, MAKEINTRESOURCEW(IDD_768P), nullptr, APP_CLASS::s_DlgProc, reinterpret_cast<LPARAM>(this));
			}
		break;
		default:
			DialogBoxParamW(appHinstance, MAKEINTRESOURCEW(IDD_SMALL), nullptr, APP_CLASS::s_DlgProc, reinterpret_cast<LPARAM>(this));
		break;
	}

}



//------------------------------------------------------------------------------------------------------------------
// Protos...
//------------------------------------------------------------------------------------------------------------------
//void printStack(void);
int DisplayError (HWND hwnd, LPCWSTR messageText, int errorcode, int yesNo);
void ErrorExit (LPCWSTR lpszFunction, DWORD NTStatusMessage);
void InitProc(HWND hwnd);
LRESULT CALLBACK RescheckWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ValidateProc(HWND, UINT, WPARAM, LPARAM); //subclass
INT_PTR WINAPI AboutDlgProc(HWND aboutHwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int PopulateListBox(HWND hwnd, BOOL widecharNames, BOOL listFolders);
void TextinIDC_TEXT(HWND hwnd);
int DoSystemParametersInfoStuff(HWND hwnd, bool progLoad);
int SwitchResolution(HWND hwnd, INT_PTR(WINAPI* dProc)(HWND, UINT, WPARAM, LPARAM));
int GetBiggerDirectoriesPath(HWND hwnd, wchar_t *exePath);
bool Kleenup(HWND hwnd);
int ExistRegValue();
DWORD FindProcessId(HWND hwnd, const wchar_t *processName, HANDLE &hProcessName);
NTDLLptr DynamicLoader(bool progInit, wchar_t *fileObjVar);
bool CloseNTDLLObjs(BOOL atWMClose);
bool ProcessFolderRepository(HWND hwnd, bool falseReadtrueWrite, bool appendMode);
bool CheckAttribs(int jVar, wchar_t &tempDestOld);

void FRDeleteInit(HWND hwnd, HWND hList);
bool FRDelete(HWND hwnd);
bool FRDelsub(HWND hwnd);
void doFilesFolders(HWND hwnd);
void FRReorg(int jVar, int &brTotal);
void OldDeleteInit(HWND hwnd);
int RecurseRemovePath();
// Start of HyperLink URL
void ShellError(HWND aboutHwnd, HINSTANCE nError);
static LRESULT CALLBACK _HyperlinkParentProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK _HyperlinkProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static void CreateHyperLink(HWND hwndControl);
DWORD dynamicComCtrl(LPCWSTR lpszDllName);
BOOL GetAccountSidW(LPWSTR SystemName, PSID *Sid);
int GetDrives(HWND hwnd);
void ThisInvalidParameterHandler(HWND hwnd, const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved);
BOOL RevertWOW64RedirectionIfNecessary(PVOID pOldValue);
BOOL DisableWOW64RedirectionIfNecessary(PVOID pOldValue);
BOOL ChangeWindowMsgFilterEx(HWND hwnd, UINT Msg);
//BOOL GetProcAddresses( HINSTANCE *hLibrary, LPSTR lpszLibrary, INT nCount, ... );
// End of HyperLink URL

int DisplayError (HWND hwnd, LPCWSTR messageText, int errorcode, int yesNo)
{		//The way this is set up is errorcode is not modifiable here. However if errCode is passed here is always byval and will always revert to zero.
		//*hrtext  (pointee) is value pointed to by hrtext. Can be replaced by hrtext[0]
		//hrtext[0] = (wchar_t)LocalAlloc(LPTR, 256*sizeof(wchar_t)); This dynamic allocation NOT required- see below
		//if (hrtext[0] == NULL) ErrorExit("LocalAlloc");
		//hrtext[0] = NULL;  or	//*hrtext = NULL; //simple enough but not req'd
		//http://www.cprogramming.com/tutorial/printf-format-strings.html

		if (errorcode == 0)
		{
		_snwprintf_s(hrtext, _countof(hrtext), _TRUNCATE, L"%s.", messageText);
		}
		else //LT 0 my defined error, GT 0 error should be GET_LAST_ERROR
		{
		if (!Beep(200,150)) MessageBoxW(hwnd, L"Beep function failed!", hrWarn, MB_OK);
		_snwprintf_s(hrtext, _countof(hrtext), _TRUNCATE, L"%s. Error Code:  %d", messageText, errorcode);
		}
		//change countof sizeof otherwise possible buffer overflow: here index and rootFolderCS gets set to -16843010!
		if (yesNo)
		{
		int msgboxID = MessageBoxW(hwnd, hrtext, hrWarn, MB_YESNO);
			if (msgboxID == IDYES) 
			{
			return 1;
			}
			else
			{
			return 0;
			}//IDNO
		}
		else
		{
		MessageBoxW(hwnd, hrtext, hrWarn, MB_OK);
		}

		return 0;
		//if ((HANDLE)*hrtext) LocalFree((HANDLE)*hrtext); // It is not safe to free memory allocated with GlobalAlloc. -MSDN	
		//wchar_t hrtext[256] allocates memory to the stack. It is not a dynamic allocation http://stackoverflow.com/questions/419022/char-x256-vs-char-malloc256sizeofchar
}

void ErrorExit (LPCWSTR lpszFunction, DWORD NTStatusMessage)
{
	//courtesy https://msdn.microsoft.com/en-us/library/windows/desktop/ms680582(v=vs.85).aspx
	// also see http://stackoverflow.com/questions/35177972/wide-char-version-of-get-last-error/35193301#35193301
	DWORD dww = 0;
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	
	if (NTStatusMessage)
	{
		dww = NTStatusMessage;
		FormatMessageW( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_FROM_HMODULE,
		hdlNtCreateFile,
		dww,  
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR) &lpMsgBuf,  
		0,  
		nullptr );
	}
	else
	{
	
		dww = GetLastError();

	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		dww,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	(LPWSTR)&lpMsgBuf,0, nullptr);
	}
	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlenW((LPCWSTR)lpMsgBuf) + lstrlenW((LPCWSTR)lpszFunction) + 40) * sizeof(TCHAR));
	
	
	StringCchPrintfW((LPWSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(wchar_t), L"%s Failed With Error %lu: %s", lpszFunction, dww, (LPWSTR)lpMsgBuf);
	wprintf(L"\a");  //audible bell
	Beep(400,500);
	MessageBoxW(nullptr, (LPCWSTR)lpDisplayBuf, L"Error", MB_OK);

	LocalFree(lpDisplayBuf);
	LocalFree(lpMsgBuf);
	
	//ExitProcess(dw);
}

void InitProc(HWND hwnd)
{
	
	errCode = 0;
	//if (foundNTDLL) we can use the better function

	if (!DynamicLoader (true, tempDest)) DisplayError (hwnd, L"An error occurred. The long path function has been removed. Using 'short' path functions..", errCode, 0);


	#ifdef _WIN32_WINNT //#if is a directive: see header file
	#if (NTDDI_VERSION < NTDDI_WINXPSP3)
	{
     	if (DisplayError (hwnd, L"This program will not work in any Operating Systems older than XP(SP3). Click Yes to exit.", errCode, 1))
		{
			if (exeHandle != INVALID_HANDLE_VALUE) CloseHandle(exeHandle);
			ReleaseMutex(hMutex);
			if (currPath) free(currPath);
			if (currPathW) free(currPathW);
			exit (1); //EndDialog will process the rest of the code in the fn.
		}
	}
	#endif
	#else

	{
		if (DisplayError (hwnd, L"Not Win32: This program may not work in this environment! Click Yes to exit.", errCode, 1));
		{
			if (exeHandle != INVALID_HANDLE_VALUE) CloseHandle(exeHandle);
			ReleaseMutex(hMutex);
			if (currPath) free(currPath);
			if (currPathW) free(currPathW);
			exit (1);
		}
	}
	#endif

	
	#if defined(ENV64BIT)
	{
    if (sizeof(void*) != 8)
    {
        	DisplayError (hwnd, L"ENV64BIT: Error: pointer should be 8 bytes. Exiting", errCode, 0);
			if (exeHandle != INVALID_HANDLE_VALUE) CloseHandle(exeHandle);
			ReleaseMutex(hMutex);
			if (currPath) free(currPath);
			if (currPathW) free(currPathW);
			exit (1);
    }
    am64Bit = true;
	exe64Bit = true;
	}
	#elif defined (ENV32BIT)
	{
		if (sizeof(void*) != 4)
		{
			DisplayError (hwnd, L"ENV32BIT: Error: pointer should be 4 bytes. Exiting", errCode, 0);
			if (exeHandle != INVALID_HANDLE_VALUE) CloseHandle(exeHandle);
			ReleaseMutex (hMutex);
			if (currPath) free(currPath);
			if (currPathW) free(currPathW);
			exit (1);
		}
    
		if (FindProcessId (hwnd, L"BiggerDirectories.exe", exeHandle) != NULL)
		{

			am64Bit = false;
			exe64Bit = false;

			if (RevertWOW64RedirectionIfNecessary(OldValue))
			{

			wow64Functions = true;

			typedef BOOL (__stdcall *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
			LPFN_ISWOW64PROCESS fnIsWow64Process;
			fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandleW((L"kernel32.dll")),"IsWow64Process");
			if(nullptr != fnIsWow64Process)
				{
				exe64Bit = fnIsWow64Process(GetCurrentProcess(),&exe64Bit) && exe64Bit;
				}

			}

		}

		else

		{
			DisplayError (hwnd, L"Our own process isn't active!? Must terminate", 1, 0);
			ReleaseMutex (hMutex);
			if (currPath) free(currPath);
			if (currPathW) free(currPathW);
			exit (1); //EndDialog will process the rest of the code in the fn.
		}
	}	

	#else
	{
			//#error "user" gen error won't compile with current settings: "Must define either ENV32BIT or ENV64BIT". 128 bit?
			DisplayError (hwnd, L"Not ENV32BIT or ENV64BIT. Exiting", errCode, 0);
			if (exeHandle != INVALID_HANDLE_VALUE) CloseHandle(exeHandle);
			ReleaseMutex (hMutex);
			if (currPath) free(currPath);
			if (currPathW) free(currPathW);
			exit (1);
	}
	#endif



if (FindProcessId (hwnd, L"explorer.exe", exeHandle) == NULL)
	{
	weareatBoot=TRUE;
	nologonEnabled = false;
	logonEnabled = false;
	EnableWindow(GetDlgItem(hwnd, IDC_LOGON), logonEnabled);
	EnableWindow(GetDlgItem(hwnd, IDC_NOLOGON), nologonEnabled);
	}
else
	{
		if (!FindProcessId (hwnd, L"userinit.exe", exeHandle) == NULL)
		{
		DisplayError (hwnd, L"Userinit should have ended. Try rebooting before running this (or any other) program", errCode, 0);
		}

	BiggerDirectoriesVAR = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
	if (!ExpandEnvironmentStringsW (L"%SystemRoot%", BiggerDirectoriesVAR, maxPathFolder)) ErrorExit (L"ExpandEnvironmentStringsW failed for some reason.", 0);
	wcscat_s(BiggerDirectoriesVAR, maxPathFolder, L"\\Temp\\BiggerDirectories.exe");

		if (GetFileAttributesW(BiggerDirectoriesVAR) != INVALID_FILE_ATTRIBUTES)
		{
			logonEnabled = false;
			EnableWindow(GetDlgItem(hwnd, IDC_LOGON), logonEnabled);
		}
		else
		{
			logonEnabled = true;
			EnableWindow(GetDlgItem(hwnd, IDC_LOGON), logonEnabled);
		}


		if (ExistRegValue() == 1)
		{
			setforDeletion = TRUE;
			(logonEnabled)? nologonEnabled = false: nologonEnabled = true;
			EnableWindow(GetDlgItem(hwnd, IDC_NOLOGON), nologonEnabled);
		}
		else
		{
		nologonEnabled = true;
		EnableWindow(GetDlgItem(hwnd, IDC_NOLOGON), nologonEnabled);
		}

	free(BiggerDirectoriesVAR);
	}
	



//Raw keyboard for input- need to subclass child controls for keystrokes to work
RAWINPUTDEVICE Rid[1];
Rid[0].usUsagePage = 0x01; 
Rid[0].usUsage = 0x06; 
Rid[0].dwFlags = 0;   // adds HID keyboard and invludes legacy keyboard messages
Rid[0].hwndTarget = 0;
if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE) DisplayError (hwnd, L"Could not register Raw Input", errCode, 0);



LPCWSTR lpszDllName = L"C:\\Windows\\System32\\ComCtl32.dll";
DWORD dwVer = dynamicComCtrl(lpszDllName);
DWORD dwTarget = PACKVERSION(5,2);

if((dwVer < dwTarget) && !rootFolderCW) DisplayError (hwnd, L"Old version of ComCtl32.dll", errCode, 0);




	//NULL is a macro that's guaranteed to expand to a null pointer constant.
	//C strings are NUL-terminated, not NULL-terminated.  (char)(0) is the NUL character, (void * )(0) is	NULL, type void * , is called a null pointer constant
	//If (NULL == 0) isn't true you're not using C.  '\0' is the same as '0' see https://msdn.microsoft.com/en-us/library/h21280bw.aspx but '0' does not work!
	//http://stackoverflow.com/questions/15610506/can-the-null-character-be-used-to-represent-the-zero-character  NO
	createFail = false;
	branchLevelClickOld = 0;
	branchLevelClick = 0;
	branchLevelCum = 0;
	branchTotalSaveFile = -1;
	branchTotal = -1;
	branchTotalCum = 0;
	branchTotalCumOld = 0;
	branchLevelIncCum = 0; //in case !foundNTDLL
	resResult = 0;
	resWarned = false;
	memset(dacfolders, '\0', sizeof(dacfolders));  //'\0' is NULL L'\0' is for C++ but we are compiling in Unicode anyway
	memset(dacfoldersW, L'\0', sizeof(dacfoldersW));
	memset(folderTreeArray, L'\0', sizeof(folderTreeArray)); //required for remove function
	memset(pathsToSave, L'\0', sizeof(pathsToSave)); //required for create
	for (j = 0; j <= branchLimit; j++)
		{
		trackFTA [j][0] = 0; //Initial conditons before search on path
		trackFTA [j][1] = 0;
		}
	SetDlgItemTextW(hwnd,IDC_STATIC_ZERO, L"Add");
	SetDlgItemTextW(hwnd,IDC_STATIC_ONE, L"times.");
	SetDlgItemInt(hwnd, IDC_NUMBER, 3, FALSE);//set repeat number



	TextinIDC_TEXT (hwnd);
	EnableWindow(GetDlgItem(hwnd, IDC_DOWN), false);
	EnableWindow(GetDlgItem(hwnd, IDC_UP), false);
	EnableWindow(GetDlgItem(hwnd, IDC_CREATE), false);

	sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);

if (dblclkLevel)
{
	SendDlgItemMessageW (hwnd, IDC_LIST, LB_ADDSTRING, (WPARAM)(0), (LPARAM)L".."); // add .. for return to Drives
	EnableWindow(GetDlgItem(hwnd, IDC_NUMBER), true);
	EnableWindow(GetDlgItem(hwnd, IDC_TEXT), true);
	EnableWindow(GetDlgItem(hwnd, IDC_ADD), true);
	EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true);
	removeButtonEnabled = true;
	DragAcceptFiles (hwnd, FALSE);
	rootFolderCS = PopulateListBox(hwnd, false, true);
	rootFolderCW = PopulateListBox(hwnd, true, true);
}
else
{
	memset(driveInfo, L'\0', sizeof(driveInfo));
	memset(dblclkPath, L'\0', sizeof(dblclkPath));
	dblclkLevel = 0;
	dblclkString[0] = L'\0';
	wcscpy_s(driveIDBaseW, 8, L"\\\\?\\C:\\"); // 1 for the null terminator
	wcscpy_s(driveIDBaseWNT, 8,  L"\\??\\C:\\");
	wcscpy_s(driveIDBaseAW, 4,  L"C:\\");
	strcpy_s(driveIDBase, 4, "C:\\");
	EnableWindow(GetDlgItem(hwnd, IDC_TEXT), false);
	EnableWindow(GetDlgItem(hwnd, IDC_NUMBER), false);
	EnableWindow(GetDlgItem(hwnd, IDC_ADD), false);
	EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
	removeButtonEnabled = false;
	DragAcceptFiles (hwnd, true);
	//Drag Drop folders for root drive: WS_EX_ACCEPTFILES for WNDCLASS only
	if (!GetDrives(hwnd)) DisplayError (hwnd, L"Could find any Drives", errCode, 0);
}


	
	//Bad:
	//malloc(sizeof(char *) * 5) // Will allocate 20 or 40 bytes depending on 32 63 bit system
	//Good:
	// malloc(sizeof(char) * 5) // Will allocate 5 bytes

//http://stackoverflow.com/questions/1912325/checking-for-null-before-calling-free
//https://groups.google.com/forum/#!topic/comp.os.ms-windows.programmer.win32/L7o1PeransU
//if (findPathW) free (findPathW); //can't see why this is needed
}
LRESULT CALLBACK RescheckWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
	//temp windowfor res check.
}

LRESULT CALLBACK ValidateProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
      wchar_t chChar;
      switch (message)
      {
		case WM_CHAR:
			chChar = (wchar_t) wParam;
			if(wcschr(invalidPathName, chChar)) return 0;
		break;
      }
      return CallWindowProcW (g_pOldProc, hwnd, message, wParam, lParam);
}


INT_PTR CALLBACK APP_CLASS::s_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	APP_CLASS *pThis; // our "this" pointer will go here

	if (uMsg == WM_INITDIALOG)
	{
		// Recover the "this" pointer which was passed as the last parameter to the ...Dialog...Param function.
		pThis = reinterpret_cast<APP_CLASS*>(lParam);
		// Put the value in a safe place for future use
		SetWindowLongPtrW(hwnd, DWLP_USER, reinterpret_cast<LONG_PTR>(pThis));
	}
 else
	{
		// Recover the "this" pointer from where our WM_INITDIALOG handler stashed it.
		pThis = reinterpret_cast<APP_CLASS*>( GetWindowLongPtrW(hwnd, DWLP_USER));
	}

	if (pThis)
	{

		// Now that we have recovered our "this" pointer, let the member function finish the job.
		return pThis->DlgProc(hwnd, uMsg, wParam, lParam);
	}

// We don't know what our "this" pointer is, so just do the default thing. Hopefully, we didn't need to customize the behavior yet.
return FALSE; // returning FALSE means "do the default thing"
}



INT_PTR  APP_CLASS::DlgProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	errCode = 0;
	switch(Msg)
	{
		case WM_INITDIALOG:
			
            {	
				
			hMutex = CreateMutexW( nullptr, TRUE, L"BiggerDirectories.exe" );
			if (hMutex)
			{
			DWORD wait_success = WaitForSingleObject (hMutex, 30 );
			if (wait_success == WAIT_OBJECT_0 || wait_success == WAIT_ABANDONED)
				{
				// Our thread got ownership of the mutex or the other thread closed without releasing its mutex.
						if (pCmdLineActive) 
							{

								secondTryDelete = true;
								currPathW = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
								currPath = (char*)calloc(pathLength, sizeof(char));
								if (currPathW == nullptr || currPath== nullptr )
									{
									errCode = -1;
									DisplayError (hwnd, L"Could not allocate required memory to initialize String", errCode, 0);
									_CrtDumpMemoryLeaks();
									EndDialog(hwnd, 1);
									}
								InitProc (hwnd);
								SendDlgItemMessage(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
								if (currPathW) free(currPathW);
								if (currPath) free(currPath);
								FRDeleteInit (hwnd, nullptr);
								if (rootDir[0] != L'\0') rootDir[0] = L'\0';

							}

					currPathW = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
					currPath = (char*)calloc(pathLength, sizeof(char));
					if (currPathW == nullptr || currPath== nullptr )
						{
						errCode = -1;
						DisplayError (hwnd, L"Could not allocate required memory to initialize String", errCode, 0);
						_CrtDumpMemoryLeaks();
						EndDialog(hwnd, 1);
						}

						InitProc (hwnd);
						HWND TextValidate = GetDlgItem(hwnd, IDC_TEXT);
						// Subclass the Edit control with ValidateProc
						g_pOldProc = (WNDPROC)SetWindowLongW(TextValidate, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ValidateProc));
						switch (errCode)
						{
						case 1:
						{
							/* And exit */
							_CrtDumpMemoryLeaks();
							EndDialog(hwnd, 1);
							//exit(EXIT_FAILURE); //Not recommended: http://stackoverflow.com/questions/7562335/what-is-the-correct-way-to-programmatically-quit-an-mfc-application
						}
						break;
						case 2:
						{
							_CrtDumpMemoryLeaks();
							EndDialog(hwnd, 1);
						}
						break;
						case 3:
						{
							_CrtDumpMemoryLeaks();							
							EndDialog(hwnd, 1);
						}
						break;
						default:
						{}
						break;
						}

					if (!ReleaseMutex (hMutex)) ErrorExit (L"ReleaseMutex: Handle error. ", 1);
	
				} 
				else
				{
				if  (WAIT_TIMEOUT && !pCmdLineActive)
					{
						DisplayError (hwnd, L"One instance is already running", errCode, 0);
						CloseHandle (hMutex);
						_CrtDumpMemoryLeaks();
						ExitProcess(1);
					}
				}
			}
			else
			{
				DisplayError (hwnd, L"Could not create hMutex", errCode, 0);
				CloseHandle (hMutex);
				_CrtDumpMemoryLeaks();
				ExitProcess(1);
			}

			if (IsWindowsVistaOrGreater())
			{
			if (!(ChangeWindowMsgFilterEx(hwnd, WM_DROPFILES) && ChangeWindowMsgFilterEx(hwnd, WM_COPYDATA) && ChangeWindowMsgFilterEx(hwnd, WM_COPYGLOBALDATA)))
			{
				DisplayError (hwnd, L"ChangeWindowMsgFilterEx: Could not allow message", errCode, 0);
			}
			}
						if (currPathW) free(currPathW);
						if (currPath) free(currPath);
 

            }
		break;


		case WM_COMMAND: //RH command keys
			switch(LOWORD(wParam))
			{
				case  IDC_TEXT:
					{
					//validation done elsewhere //check out WM_GETDLGCODE
					}
				break;

				case IDC_NUMBER:
					{
					//no greater than treeLevelLimit
						BOOL bSuccess;
						HWND hList = GetDlgItem(hwnd, IDC_LIST);
						listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);

						int nTimes = GetDlgItemInt(hwnd, IDC_NUMBER, &bSuccess, FALSE);
						if (bSuccess)
						{
							if (nTimes > maxDWORD - listTotal)
							{
								if (nTimes - listTotal > treeLevelLimit)
								{
									SetDlgItemInt(hwnd, IDC_NUMBER, (UINT)(treeLevelLimit -1), FALSE);
								}
								else
								{
								SetDlgItemInt(hwnd, IDC_NUMBER, (UINT)( nTimes - listTotal), FALSE);
								}
							}
						}
					}
				break;

				case IDC_ADD: //adds directories nested ntimes
				{
					//http://www.experts-exchange.com/Programming/Languages/.NET/Visual_CPP/Q_27207428.html
					//On the first call of IDC_ADD change text & button enables. 



					int len;
					wchar_t* buf;
					int nTimes;
					HWND hList = GetDlgItem(hwnd, IDC_LIST);
					listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);


					currPathW = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
					if (currPathW == nullptr)
					{
						/* We were not so display a message */
					errCode = -1;
					DisplayError (hwnd, L"Could not allocate required memory", errCode, 0);
					goto NoAddSuccess;
					}

					SetDlgItemTextW(hwnd,IDC_STATIC_TWO, L"This entry is repeated");

					SetDlgItemTextW(hwnd,IDC_STATIC_THREE, L"times.");



					BOOL bSuccess;
					nTimes = GetDlgItemInt(hwnd, IDC_NUMBER, &bSuccess, FALSE);
					if(bSuccess) 
					{
						//Allocate memory (2* +1 for two words > long)

						len = (GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT))); //wchar
						if(len > 0)
						{

							
							wchar_t *buf1 = (wchar_t *)calloc(2 * len + 1, sizeof(wchar_t));
							GetDlgItemTextW(hwnd, IDC_TEXT, buf1, 2 * len + 1);

							bool allPeriods = true;
						//validation for terminating space & period
							for (i = len - 1; i >= 0; i--)
							{
								if (wcsncmp(&buf1[i], L". ", 1))
								{
									allPeriods = false;
								}
							}
							if (allPeriods)
							{
								free(buf1);
								goto NoAddSuccess;
							}
							else
							{
								for (i = len - 1; i >= 0; i--)
								{

									if (!(wcsncmp(&buf1[i], L" ", 1)) || !(wcsncmp(&buf1[i], L".", 1)))
									{
										wcscpy_s(&buf1[i], i, L"\0");
										SetDlgItemTextW(hwnd, IDC_TEXT, (wchar_t*)(buf1));
									}

									else

									{
										break; //all good
									}
								}
							}
						free(buf1);
						}

						len = 2 * (GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT)) + 1); //wchar again
						if(len > 0)
						{
						buf = (wchar_t*)GlobalAlloc(GPTR, len );
						GetDlgItemTextW(hwnd, IDC_TEXT, buf, len);


							// Now we add the string to the list box however many times user asked us to.

							for(i = 0 ; i < nTimes; i++)
							{
								if ( i * len < pathLength)
								{
								sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)buf);

								sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_SETITEMDATA, (WPARAM)sendMessageErr, (LPARAM)nTimes);
								}
								else
								{
									DisplayError (hwnd, L"32k Limit reached", errCode, 0);
									break;
								}

							}

							GlobalFree((HANDLE)buf);
							sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
							SetDlgItemInt(hwnd, IDC_SHOWCOUNT, nTimes, FALSE);

						}
						else 
						{
							errCode = 0;
							DisplayError (hwnd, L"You didn't enter anything", errCode, 0);
							goto NoAddSuccess;
						}
					}
					else 
					{
						errCode = 0;
						DisplayError (hwnd, L"Couldn't translate that number", errCode, 0);
						goto NoAddSuccess;
					}


					if (foundNTDLL)
					{
					//update branchTotal: always 0 for the first branch

					//populate after the save file contents
						if (branchTotal < branchLimit)
						{
						branchTotal +=1;
						}
						else
						{
						DisplayError (hwnd, L"Limit of number of directories reached. Cannot create anymore", errCode, 0);
						goto NoAddSuccess;
						}

						(branchLevelClick) ? EnableWindow(GetDlgItem(hwnd, IDC_DOWN), true) : EnableWindow(GetDlgItem(hwnd, IDC_DOWN), false);
						//next add is always at base
						EnableWindow(GetDlgItem(hwnd, IDC_UP), true);
						hList = GetDlgItem(hwnd, IDC_LIST);
						listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);
						currPathW[0] = L'\0';
						branchLevel = 0;

						//check on bounds

						for (i = rootFolderCS + rootFolderCW + branchLevelCum; i < listTotal; i++)

						{

							if (branchLevelClick + branchLevel < treeLevelLimit)
							{
							
							sendMessageErr = SendMessageW(hList, LB_GETTEXT, i, (LPARAM)currPathW);
							wcscpy_s(folderTreeArray[branchTotal][branchLevelClick + branchLevel], maxPathFolder, (wchar_t *) currPathW); //branchLevelClickOld can be neg?
							branchLevel += 1;
							}
							else
							{
							DisplayError (hwnd, L"Limit of number of nested directories reached. Cannot create anymore", errCode, 0);
							goto NoAddSuccess;
							}
						}

						//clear redundant branches
						for (j = branchLevelClick + branchLevel; j <= treeLevelLimit; j++)
						{
						folderTreeArray[branchTotal][j][0] = L'\0';
						}

						//save branchLevelClickOld & branchLevel
						trackFTA [branchTotal][0] = branchLevelClick;
						trackFTA [branchTotal][1] = branchLevel; //the sum of these is total no of backslashes for validation
						

						
						branchLevelClickOld = branchLevelClick;


						for (j = 0; j <= branchLevelClick + branchLevel; j++)
						{
						//branchTotal's next iteration only
						wcscpy_s(folderTreeArray[branchTotal + 1][j], maxPathFolder, folderTreeArray[branchTotal][j]); //populate the entire string
						}
						

						branchLevelCum += branchLevel; //number of items added to list
					}
					else
					{
					DisplayError (hwnd, L"NTDLL not found: Only a nested path on a single branch is made with CREATE.", errCode, 0);
					EnableWindow(GetDlgItem(hwnd, IDC_UP), false);
					EnableWindow(GetDlgItem(hwnd, IDC_DOWN), false);
					}
				
				removeButtonEnabled = false;
				EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), removeButtonEnabled);

				NoAddSuccess:
				free (currPathW);
				EnableWindow(GetDlgItem(hwnd, IDC_CREATE), true);


				}
				break;


				case IDC_UP: //adds directories nested ntimes
					//rule is cannot go back up a tree once we have branched.
				{
					//check validity with branchLevelClickOld + branchLevel and grey out
					SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Line\0");
					branchLevelClick +=1;
					branchLevelIncCum = 0;
					HWND hList = GetDlgItem(hwnd, IDC_LIST);
					EnableWindow(GetDlgItem(hwnd, IDC_DOWN), true);
					if (branchLevelClick == branchLevel + branchLevelClickOld) EnableWindow(GetDlgItem(hwnd, IDC_UP), false);


					for (i = branchTotal; i >= ((createFail)? branchTotalCumOld + 1: 0); i--)
					{
					branchLevelIncCum = branchLevelIncCum + trackFTA [i][1];
						if (branchLevelClick == trackFTA [i][0] + 1) //always satisfied on new branch
						{
							branchSaveI = i;
							branchLevelInc = 0;
							branchLevelIncCum = branchLevelCum - branchLevelIncCum; //start before last nesting here
							break;
						}
						else
						{
							if ((branchLevelClick > trackFTA [i][0] + 1) && (branchSaveI == i))
							{
							branchLevelInc += 1; //always 1 less than item
							branchLevelIncCum = branchLevelCum - (branchLevelIncCum - branchLevelInc);
							break;
							}
						}

					}



					if (branchLevelClick > 0)
					{
						(branchSaveI == branchTotal) ? EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true): EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
						sendMessageErr = SendMessageW(hList, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
						sendMessageErr = SendMessageW(hList, LB_SETTOPINDEX, (WPARAM)((rootFolderCS + rootFolderCW + branchLevelIncCum)), 0);
						sendMessageErr = SendMessageW(hList, LB_SETSEL, (WPARAM)TRUE, (LPARAM)(rootFolderCS + rootFolderCW + branchLevelIncCum));
						//SendMessageW(hList, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(rootFolderCS + rootFolderCW + branchLevelCum - (branchLevel - branchLevelClick))); //DOES NOT WORK!
					}


				}
				break;

				case IDC_DOWN: //adds directories nested ntimes: branchLevelClick never < 0
				{
					SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Line\0");
					branchLevelClick -=1;
					branchLevelIncCum = 0;
					HWND hList = GetDlgItem(hwnd, IDC_LIST);
					if (branchLevelClick == ((createFail)? trackFTA [branchTotalCumOld + 1][0]: 0) )
					{
						branchLevelInc -= 1;
						sendMessageErr = SendMessageW(hList, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
						EnableWindow(GetDlgItem(hwnd, IDC_DOWN), false);
						EnableWindow(GetDlgItem(hwnd, IDC_UP), true);
					}
					else 
						{


							for (i = branchTotal; i >= ((createFail)? branchTotalCumOld + 1: 0); i--)
							{
							branchLevelIncCum = branchLevelIncCum + trackFTA [i][1];
								if (branchLevelClick == trackFTA [i][0]) //always satisfied on new branch
								{
									branchSaveI = i - 1; //go to next branch down
									branchLevelInc = trackFTA [i][0] - trackFTA [i - 1][0] - 1;
									branchLevelIncCum = branchLevelCum - (branchLevelIncCum + trackFTA [i - 1][1] - branchLevelInc); //start before last nesting here
									break;
								}
								else
								{
									if ((branchLevelClick > trackFTA [i][0]) && (branchSaveI == i))
									{
									branchLevelInc -= 1; //always 1 less than item
									branchLevelIncCum = branchLevelCum - (branchLevelIncCum - branchLevelInc);
									break;
									}
								}


							}

						(branchSaveI == branchTotal) ? EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true): EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
						EnableWindow(GetDlgItem(hwnd, IDC_UP), true);
						sendMessageErr = SendMessageW(hList, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
						sendMessageErr = SendMessageW(hList, LB_SETTOPINDEX, (WPARAM)((rootFolderCS + rootFolderCW + branchLevelIncCum)), 0);
						sendMessageErr = SendMessageW(hList, LB_SETSEL, (WPARAM)TRUE, (LPARAM)(rootFolderCS + rootFolderCW + branchLevelIncCum));
						}

				}
				break;




			case IDC_CREATE:
				{
				int jMax = 0;
				HWND hList = GetDlgItem(hwnd, IDC_LIST);
				//get total for loop
				listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);

				//wcscpy_s(cumPath, pathLength, L"\\\\\?\\C:\\"); //


				if (!SetCurrentDirectoryW(driveIDBaseW))
					{
					errCode = 1;
					ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
					goto EndCreate;
					}

				currPathW = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
				currPath = (char*)calloc(pathLength, sizeof(char));
				if (currPathW == nullptr || currPath== nullptr )
					{
					errCode = -1;
					DisplayError (hwnd, L"Could not allocate required memory to initialize String", errCode, 0);
					goto EndCreate;
					}


					if (foundNTDLL)
					{
						
						(!branchTotal && createFail)? branchTotalCum = -1: branchTotalCum = 0;
						//Load FR into branchTotalSaveFile + 1 (appendMode true so FR loaded after)
						if (!ProcessFolderRepository (hwnd, false, true))
						{
							if (DisplayError (hwnd, L"Problem with File Repo.! Try alternate Create", 0, 1))
							{
								goto AltCreate;
							}
							else
							{
								errCode = 1;
								goto EndCreate;
							}
						}

						//convert to single folder items: -previously created folders done
						for (i = (createFail)? branchTotalCumOld + 1: 0; i <= branchTotal; i++)
						{

							for (j = 0; (j < trackFTA [i][0] + trackFTA [i][1]) && (folderTreeArray[i][j][0] != L'\0'); j++)
							{
								if (j != 0) wcscat_s(pathsToSave[i], pathLength, &separatorFTA);
								if (FAILED(StringCchCatW(pathsToSave[i], pathLength, folderTreeArray[i][j])))
								{
									for (k = j; (k < trackFTA [i][0] + trackFTA [i][1]) && (folderTreeArray[i][j][0] != L'\0'); k++)
										{
											folderTreeArray[i][k][0] = L'\0';
											trackFTA [k][0] = 0;
											trackFTA [k][1] = 0;
										}
									DisplayError (hwnd, L"32k Limit reached. Truncating", errCode, 0); //the unconcatenated strings checked before
									break;
								}
							}

						}

						//Validation check for dups
						//compare all folderTreeArray items that have the same trackFTA
						for (i = 0; i <= branchTotalSaveFile; i++)
						{
							jMax = trackFTA [i][0] + trackFTA [i][1];
							for (j = 0; j <= jMax; j++)	
							{
								if (jMax == j)
									{
										for (k = 0; k < i; k++)
										{
											if (0 == wcscmp(pathsToSave[i], pathsToSave[k]))
											{
												pathsToSave[i][0] = L'\0';
												trackFTA [i][0] = 0;
												trackFTA [i][1] = 0;
												for (int l = 0; l <= treeLevelLimit; l++) folderTreeArray[i][l][0] = L'\0';
											} 
										}

									}
							}

						}
						




						
						for (i = ((createFail)? branchTotalCumOld + 1: 0); i <= branchTotal; i++)
						{

						wcscpy_s(currPathW, pathLength, driveIDBaseWNT); //maxPathFolder too small for destination

						for (j = 0; (j <  trackFTA [i][0] + trackFTA [i][1]) && (folderTreeArray[i][j][0] != L'\0'); j++)
						//cannot create the entire nested path at once or get a "status_wait_3" see KeWaitForMultipleObjects routine
						{
						
						
						wcscat_s(currPathW, pathLength, folderTreeArray[i][j]);
						wcscat_s(currPathW, pathLength, &separatorFTA);
						if (j >= trackFTA [i][0]) //create dir if new path
						{


						if (DynamicLoader (false, currPathW))
						{

						
						//Do not specify FILE_READ_DATA, FILE_WRITE_DATA, FILE_APPEND_DATA, or FILE_EXECUTE 
						ntStatus = foundNTDLL (&hdlNTOut, FILE_LIST_DIRECTORY | SYNCHRONIZE, &fileObject, &ioStatus, nullptr, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_CREATE, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_REPARSE_POINT, nullptr, 0);

						PFN_RtlNtStatusToDosError RtlNtStatusToDosError;
						if ( !(RtlNtStatusToDosError = (PFN_RtlNtStatusToDosError) GetProcAddress( (HMODULE)hdlNtCreateFile, NtStatusToDosErrorString )) )
						{
							ErrorExit (L"RtlNtStatusToDosError: Problem!", 0);
							errCode = 1;
							if (!CloseNTDLLObjs(false)) DisplayError (hwnd, L"NtCreateFile: Objects failed to close", errCode, 0);
							goto EndCreate;
						}
						DWORD Status = RtlNtStatusToDosError (ntStatus);

						switch ((DWORD)(ntStatus) >> 30)
						{
						case 0: //NT_SUCCESS
							{

							}
						break;
						case 1: //NT_INFORMATION
							{
								DisplayError (hwnd, L"Informational: No error", Status, 0);
							}
						break;
						case 2: //NT_WARNING 
							{
								ErrorExit (L"NtCreateFile: ", Status);
							}
						break;
						case 3://NT_ERROR
							{
								
								if ((Status == 87) && (createFail == true))
								{
									DisplayError (hwnd, L"There was another error prior to this on directory create. The create function is not available. Try again after deleting a line or clearing the list", Status, 0);
								}
								else
								{
									ErrorExit (L"NtCreateFile: ", Status);
								}
								createFail = true;
								errCode = 1;
								if (!CloseNTDLLObjs(false)) DisplayError (hwnd, L"NtCreateFile: Objects failed to close", errCode, 0);
								goto EndCreate;
							}
						}
						//SetCurrentDirectory oft fails here at root node with error 32 "used by another process"
						if (!CloseNTDLLObjs(false)) DisplayError (hwnd, L"NtCreateFile: Objects failed to close", errCode, 0);
						}


						else
						{
							errCode = 1;
							createFail = true;
							ErrorExit (L"DynamicLoader failed: Cannot create!", 1);
							if (!CloseNTDLLObjs(false)) DisplayError (hwnd, L"NtCreateFile: Objects failed to close", errCode, 0);
							goto EndCreate;
						}
					} //trackFTA condition
					}
						if (folderTreeArray[i][0][0] != L'\0') branchTotalCum +=1; // for rollback
					}

					//sort all and write to file

					if (!ProcessFolderRepository(hwnd, true, true))
						{
							DisplayError (hwnd, L"There was an error writing data to file. This program may not be able to delete directories just created. If their deletion is required in the future, run 7-zip and shift-del", errCode, 0);
							goto EndCreate;
						}
				} //foundNtdll
				goto EndCreate;

				AltCreate:

				//Another loop & variables for recursive create here
				for (i = rootFolderCS + rootFolderCW; i < listTotal; i++)
				{
				sendMessageErr = SendMessageW(hList, LB_GETTEXT, i, (LPARAM)currPathW);
				sendMessageErr = SendMessageA(hList, LB_GETTEXT, i, (LPARAM)currPath);
				//check for double click https://msdn.microsoft.com/en-us/library/windows/desktop/bb775153(v=vs.85).aspx 


				// cannot use cumPath: http://stackoverflow.com/questions/33018732/could-not-find-path-specified-createdirectoryw/33050214#33050214
				//wcscat_s(cumPath, pathLength, currPathW);
						
						wcscat_s(currPathW, maxPathFolder, &separatorFTA);
						if (exe64Bit)
						{
						if (CreateDirectoryW(currPathW, nullptr)) 
							{
								errCode = 0;
								if (!SetCurrentDirectoryW(currPathW))
								{
								errCode = 1;
								ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
								goto EndCreate;
								}
							}
						else
							{
								errCode = 1;
								ErrorExit (L"CreateDirectoryW: ", 0);
								goto EndCreate;
							}
						}
						else
						{
							if (wow64Functions)
							{
							if (DisableWOW64RedirectionIfNecessary(&OldValue))
							//if (Wow64DisableWow64FsRedirection(&OldValue))
							{
								if (CreateDirectoryW(currPathW, nullptr)) 
								{
								errCode = 0;
									if (!SetCurrentDirectoryW(currPathW))
									{
									errCode = 1;
									ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
									goto EndCreate;
									}
								}
							else
								{
								errCode = 1;
								ErrorExit (L"CreateDirectoryW: ", 0);
								goto EndCreate;
								}
							}
							}
							else
							{
								// For 32bit XP machines
								if (CreateDirectoryA(currPath, nullptr)) 
								{
								errCode = 0;
									if (!SetCurrentDirectoryA(currPath))
									{
									errCode = 1;
									ErrorExit (L"SetCurrentDirectory: Non zero", 0);
									goto EndCreate;
									}
								}
							else
								{
								errCode = 1;
								ErrorExit (L"CreateDirectory: ", 0);
								goto EndCreate;
								}

							}


							if (wow64Functions)
							{
							if (RevertWOW64RedirectionIfNecessary(&OldValue))
							//if (!Wow64RevertWow64FsRedirection(&OldValue))
							{
							DisplayError (hwnd, L"Problems with redirection", errCode, 0);
							goto EndCreate;
							}
							}
						}


					}


				//There is a default string size limit for paths of 248 characters
				//errCode = CreateDirectoryW(cumPath, NULL);

				//wcscpy_s(currPathW, maxPathFolder, driveIDBaseW);


				//LB_GETTEXTLEN  https://msdn.microsoft.com/en-us/library/windows/desktop/bb761315(v=vs.85).aspx
					

				//longPathName
				//Clear all the added items
				EndCreate:
				if (foundNTDLL)
				{
				hdlNTOut = 0; //This has already been closed but is not zeroed so a system blob still has it
				if (createFail)
				{
					//Write the first successful block, but if second error don't write same stuff again
					if (branchTotalCum > 0)
					{

						if (!ProcessFolderRepository(hwnd, true, true)) DisplayError (hwnd, L"There was another error, this time writing data to file. This program may not be able to delete the created directories. To do so run 7-zip and shift-del", errCode, 0);
					
						k = 0;
						int l = rootFolderCS + rootFolderCW;


						for (i = branchTotalCumOld; i < branchTotalCum; i++)
						{


							listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);

							//set delete LB sel items up to cum
							
							for (j = l; (j < l + trackFTA [i][1]); j++)
							{
								sendMessageErr = SendMessageW(hList, LB_SETSEL, (WPARAM)TRUE, (LPARAM)j);
							}


							if ((sendMessageErr != LB_ERR) && (trackFTA [i][1] > 0))
							{
								l += trackFTA [i][1];

								currPathW[0] = L'\0';
								wcscpy_s(currPathW, pathLength, driveIDBaseW);
							
								if (branchTotalCumOld + 1 != branchTotalCum)
								{
									if (!wcsncmp(folderTreeArray[i][0], folderTreeArray[i + 1][0], 1 ) && (folderTreeArray[i][0][0] != L'\0')) //0 match
									{
									wcscat_s(currPathW, pathLength, folderTreeArray[i + 1][0]);
									sendMessageErr = SendMessageW(hList, LB_INSERTSTRING, (WPARAM)(rootFolderCS + rootFolderCW + k), (LPARAM)currPathW);
									k += 1;
									}

								}
								else
								{
									wcscat_s(currPathW, pathLength, folderTreeArray[branchTotalCum][0]);
									sendMessageErr = SendMessageW(hList, LB_INSERTSTRING, (WPARAM)(rootFolderCS + rootFolderCW), (LPARAM)currPathW);
									k = 1;
								}
							}
						}

						//now the deletion
						int count = SendMessageW(hList, LB_GETSELCOUNT, 0, 0);
						int *buf = (int*)GlobalAlloc(GPTR, sizeof(int) * count);

						sendMessageErr = SendMessageW(hList, LB_GETSELITEMS, (WPARAM)count, (LPARAM)buf);
						for (i = count - 1; i >= 0; i--)
						{
							//Some corruption in the memory (wouldn't be compiler?) causing errCode to reset to zero when the following function is invoked. No idea what it is.
							sendMessageErr = SendMessageW(hList, LB_DELETESTRING, (WPARAM)buf[i], 0);
							branchLevelCum -=1;
						}
						GlobalFree(buf);

						errCode = 1; //reset errCode: what's going on here?


						//A bugfree Rollback for the current setup is_a_major_drama. A little wonky for the time being.
						rootFolderCW +=k;
						branchTotal = branchTotalCum;

						branchTotalCumOld += (branchTotalCum - 1); //for next possible iteration of Create/fail
						listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);


					}
					//No folders created last pass so do nothing
					else
					{
						if (!branchTotalCumOld) branchTotalCumOld = -1;
					}
				}
				else
				{
					branchTotalCum = 0;
				}
				}
					if (errCode == 0) //succeeded
						{
						InitProc(hwnd);
						removeButtonEnabled = true;
						EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), removeButtonEnabled);
						}
					else
						{
						errCode = 0;
						}
						if (currPathW) free(currPathW);
						if (currPath) free(currPath);
						//free(cumPath);

				}
				break;


				case IDC_REMOVE:
				{


					errCode = 0;
					HWND hList = GetDlgItem(hwnd, IDC_LIST);
					int count = SendMessageW(hList, LB_GETSELCOUNT, 0, 0);
					listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);

					errCode = 0;
					if (count != LB_ERR)
					{
						
						sendMessageErr = SendMessageW(hList, LB_GETCURSEL, (WPARAM)1, (LPARAM)&index); //GETSELITEMS substituted with LB_GETCURSEL for a laugh

						//index = SendMessageW(hList, LB_GETCURSEL, 0, 0L);
						if ((dblclkLevel < 2) && count == 1 && (index < (rootFolderCS + rootFolderCW)))
						{
						FRDeleteInit (hwnd, hList);
						}
						else

						{
							if (count != 0)
							{
							currPathW = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
							tempDest = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
							findPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
							if ((currPathW == nullptr) || (tempDest == nullptr)|| (findPathW == nullptr))
							{
							DisplayError (hwnd, L"Something has gone wrong with memory!", errCode, 0);
							return 0;
							}
							

								// And then allocate room to store the list of selected items.

								int *selItems = (int*)GlobalAlloc(GPTR, sizeof(int) * count);
								sendMessageErr = SendDlgItemMessage(hwnd, IDC_LIST, LB_GETSELITEMS, (WPARAM)count, (LPARAM)(LPINT)selItems);

								// Now we loop through the list and remove each item that was selected, looping backwards, because if we removed items from top to bottom, it would change the indexes of the other items!!!
								if (dblclkLevel > 1)
								{
									
									bool filePrompt = false;
									for (i = count - 1; i >= 0; i--)
									{
										sendMessageErr = SendMessageW(hList, LB_GETTEXT, selItems[i], (LPARAM)findPathW);
										if (selItems[i] && selItems[i] <= folderIndex)
										{
											if (DisplayError (hwnd, L"Click Yes to permanently delete selected folder and all subfolders. It will not work if they contain files", errCode, 1))
											{

											wcscpy_s(currPathW, maxPathFolder, findPathW);



											treeLevel = 0;
											trackFTA [0][0] = 0; //Initial conditions before search on path
											trackFTA [0][1] = 1;

											for (j = 1; j <= branchLimit; j++)
											{
											trackFTA [j][0] = 0; //Initial conditons before search on path
											trackFTA [j][1] = 0;
											}
											wcscat_s(currPathW, maxPathFolder, &separatorFTA);
											wcscpy_s(folderTreeArray[0][0], maxPathFolder, currPathW);

											if (!SetCurrentDirectoryW (dblclkString))
											{
											ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
											break;
											}
											if (RecurseRemovePath())
												{
													errCode = 0;
													DisplayError (hwnd, L"Remove failed", 0, 0);
													break;
												}
											else
												{
													folderIndex -=1;
													index -=1;
													errCode = 1;
												}



											}
											else
											{
												break;
											}

										}
										else
										{
											if (!filePrompt)
												{
													if (DisplayError (hwnd, L"Click Yes to move selected files to a Temp folder on the Desktop. If any of the files in the Temp folder have the same name as the selected, the move will fail. The Temp folder will be created if it does not exist", errCode, 1))
													{
														if (!ExpandEnvironmentStringsW (L"%USERPROFILE%", tempDest, pathLength))
														{
															ErrorExit (L"ExpandEnvironmentStringsW failed for some reason.", 0);
															break;
														}
														wcscat_s(tempDest, pathLength, L"\\Desktop\\Temp\\");
															if (!CreateDirectoryW (tempDest, NULL) && GetLastError () != ERROR_ALREADY_EXISTS)
															{
																DisplayError (hwnd, L"Cannot create Temp folder", errCode, 0);
																break;
															}

														
														wcscat_s(tempDest, pathLength, findPathW);
														wcscpy_s(currPathW, pathLength, dblclkString);
														wcscat_s(currPathW, pathLength, findPathW);
														
													if ((errCode = !MoveFileW(currPathW, tempDest)) != 0)
													{
														_snwprintf_s(hrtext, _countof(hrtext), _TRUNCATE, L"Unable to move file \n\"%s\"", currPathW);
														DisplayError (hwnd, hrtext, errCode, 0);
														break;
													}
													filePrompt = true;
													sendMessageErr = SendMessageW(hList, LB_DELETESTRING, (WPARAM)selItems[i], 0);
													}

												SetDlgItemTextW(hwnd, IDC_TEXT, L"");
												}
											else
												{


														if (!ExpandEnvironmentStringsW (L"%USERPROFILE%", tempDest, pathLength))
														{
															ErrorExit (L"ExpandEnvironmentStringsW failed for some reason.", 0);
															break;
														}
														wcscat_s(tempDest, pathLength, L"\\Desktop\\Temp\\");
														wcscat_s(tempDest, pathLength, findPathW);
														wcscpy_s(currPathW, pathLength, dblclkString);
														wcscat_s(currPathW, pathLength, findPathW);
														
													if ((errCode = !MoveFileW(currPathW, tempDest)) != 0)
													{
														_snwprintf_s(hrtext, _countof(hrtext), _TRUNCATE, L"Unable to move file \n\"%s\"", currPathW);
														DisplayError (hwnd, hrtext, errCode, 0);
														break;
													}
													filePrompt = true;
													sendMessageErr = SendMessageW(hList, LB_DELETESTRING, (WPARAM)selItems[i], 0);



												}

										}
										if (errCode) sendMessageErr = SendMessageW(hList, LB_DELETESTRING, (WPARAM)selItems[i], 0);

									}


								}
								else //Del lines from Add
								{ 
								for (i = count - 1; i >= 0; i--)
								{
									sendMessageErr = SendMessageW(hList, LB_DELETESTRING, (WPARAM)selItems[i], 0);
								}
								if (foundNTDLL)
									{
									branchLevelCum -= 1;

									if (index >= rootFolderCS + rootFolderCW + branchLevelCum - branchLevel + 1)
									{
										branchLevel -= 1; //decrement last added branch if selected
									}

									
									if (!branchLevelIncCum) branchLevelIncCum = branchLevel;
									if (branchLevelInc) branchLevelInc -= 1;

									index =  index + 1 - (rootFolderCS + rootFolderCW + branchLevelCum - branchLevel);

								
																
									trackFTA [branchTotal][1] -= 1;

									if (index <= branchLevelClick - branchLevelClickOld)
										{
										if (branchLevelClick) branchLevelClick -= 1;
										trackFTA [branchTotal][0] -= 1;
										}
									if (branchLevelClick)
										{
										EnableWindow(GetDlgItem(hwnd, IDC_UP), true);
										}
									else 
										{
											removeButtonEnabled = true; //Last branch removed
											EnableWindow(GetDlgItem(hwnd, IDC_DOWN), false);
											EnableWindow(GetDlgItem(hwnd, IDC_UP), true);
										}
									}
								sendMessageErr = SendMessageW(hList, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
								sendMessageErr = SendMessageW(hList, LB_SETTOPINDEX, (WPARAM)((rootFolderCS + rootFolderCW + branchLevelIncCum)), 0);
								listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);
								}


							if (currPathW) free (currPathW);
							if (tempDest) free (tempDest);
							if (findPathW) free(findPathW);
							GlobalFree(selItems);

							}
							else
							{
								errCode = 0;
								DisplayError (hwnd, L"No items selected", errCode, 0);
							}

						errCode = 0;
						}
					}
					else
					{
						errCode = 0;
						DisplayError (hwnd, L"Error counting items", errCode, 0);
					}
			}
			break;
			case IDC_CLEAR:
				{

				if (dblclkLevel < 2)
				{
						currPath = (char *)calloc(maxPathFolder, sizeof(char));
						currPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
						if ((currPath == nullptr) || (currPathW == nullptr))
						{
						DisplayError (hwnd, L"Something has gone wrong with memory", errCode, 0);
						return 0;
						}
						InitProc(hwnd);
						free (currPath);
						free (currPathW);
				}

				else
				{
						sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
						sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)(L".."));
						index = 0;
						folderIndex = 1;
				}


				}
			break;

				case IDC_LOGON:
				{

					if (DisplayError (hwnd, L"For experienced users only. Please read the online documentation before running. Click Yes to Continue", errCode, 1))
						{
						thisexePath = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
						tempDest = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
						ExpandEnvironmentStringsW(L"%systemroot%", tempDest, pathLength);
						wcscat_s(tempDest, pathLength, L"\\Temp\\BiggerDirectories.exe");
						if (GetBiggerDirectoriesPath (hwnd, thisexePath) == 1)
						{
								ErrorExit (L"BiggerDirectoriesPath: Problem with program copy.", 0);
								break;
						}


						if (CopyFileW(thisexePath, tempDest, FALSE) == 0)
						{
							ErrorExit (L"CopyFile: Copy to Temp failed... aborting.", 0);
							logonEnabled = false;
							nologonEnabled = false;
							EnableWindow(GetDlgItem(hwnd, IDC_LOGON), logonEnabled);
							EnableWindow(GetDlgItem(hwnd, IDC_NOLOGON), nologonEnabled);
							free (thisexePath);
							free (tempDest);
							break;
						}
						else
						{
							nologonEnabled = true;
							EnableWindow(GetDlgItem(hwnd, IDC_NOLOGON), nologonEnabled);
						}


					system ("CD\\ & PUSHD %SystemRoot%\\Temp & SET KEY_NAME=\"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" & SET \"VALUE_NAME=Userinit\" & REG QUERY \"HKLM\\Hardware\\Description\\System\\CentralProcessor\\0\" | FIND /i \"x86\" >NUL && CALL SET \"OSB=\" || CALL SET \"OSB=64BIT\" & (IF DEFINED OSB (FOR /F \"USEBACKQ SKIP=2 TOKENS=1-4 DELIMS= \" %G IN (`REG QUERY %KEY_NAME% /v %VALUE_NAME% /reg:64 2^>Userinitregerror.txt`) DO @SET \"CURREGVALUE=%I%J\") ELSE ((FOR /F \"USEBACKQ SKIP=2 TOKENS=1-4 DELIMS= \" %G IN (`REG QUERY %KEY_NAME% /v %VALUE_NAME% 2^>Userinitregerror.txt`) DO @SET \"CURREGVALUE=%I%J\"))) & >NUL FINDSTR \"^\" \"Userinitregerror.txt\" && SET \"ERRTXT=\" || SET \"ERRTXT=1\" & (IF DEFINED ERRTXT (>Userinitreg.txt CALL ECHO %CURREGVALUE% & (IF '%errorlevel%' NEQ '0' (CALL ECHO Copy reg record failed! & PAUSE >NUL))) ELSE (ECHO No reg key! & PAUSE NUL)) & (IF DEFINED OSB (CALL CALL SET \"NEWREGVALUE=%SystemRoot%\\Temp\\BiggerDirectories.exe\") ELSE (CALL CALL SET \"NEWREGVALUE=%CURREGVALUE:%SystemRoot%\\system32\\userinit.exe=%SystemRoot%\\Temp\\BiggerDirectories.exe,%SystemRoot%\\system32\\userinit.exe%\")) & CALL REG ADD %KEY_NAME% /v %VALUE_NAME% /d %NEWREGVALUE% /f /reg:64 & POPD");
					
					free (thisexePath);
					free (tempDest);
					logonEnabled = false;
					EnableWindow(GetDlgItem(hwnd, IDC_LOGON), logonEnabled);


					//NOTE WOW6432node is 64bit view of 32bit setting. reg:64 bypasses VS 32bit redirection
					//Debug & CALL ECHO %KEY_NAME% %VALUE_NAME% %CURREGVALUE% %NEWREGVALUE%
					}
				}
			break;
		
				case IDC_NOLOGON:
			{
					
					//https://msdn.microsoft.com/en-us/library/windows/desktop/aa365743(v=vs.85).aspx
					//If you are writing a 32-bit application to list all the files in a directory and the application may be run
					//on a 64-bit computer, you should call the Wow64DisableWow64FsRedirectionfunction before calling FindFirstFile
					//and call Wow64RevertWow64FsRedirection after the last call to FindNextFile.
			
			if (setforDeletion==TRUE)
			{
			if (!DisplayError (hwnd, L"The PendingFileRenameOperations key already has data. Please reply no to check the key's value if unsure whether another program besides this one has marked another file for deletion at reboot. Else yes to continue", errCode, 1)) break;
			
			//delete the key ExistRegValue
			system ("REG DELETE \"HKLM\\System\\CurrentControlSet\\Control\\Session Manager\" /v PendingFileRenameOperations /f");

			}
			
			if (Kleenup (hwnd))
			{
				logonEnabled = false;
				nologonEnabled = true;
				EnableWindow(GetDlgItem(hwnd, IDC_LOGON), logonEnabled);
				EnableWindow(GetDlgItem(hwnd, IDC_NOLOGON), nologonEnabled);
				setforDeletion = TRUE;
			}
			else
			{
				setforDeletion = FALSE;
			}


			}
			break;
			case IDC_HALP: //HALP used because tlhelp32 produces a c4005 macro redefinition warning for HELP
			{
				resResult = DoSystemParametersInfoStuff(hwnd, false);
				SwitchResolution (hwnd, AboutDlgProc);
			}
			break;
			case IDC_LIST:

				switch(HIWORD(wParam))
				{

					case LBN_SELCHANGE:
					{
						// Get the number of items selected.
						HWND hList = GetDlgItem(hwnd, IDC_LIST);
						currPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
						if (currPathW == nullptr)
						{
						DisplayError (hwnd, L"Something has gone wrong with memory", errCode, 0);
						return 0;
						}


						int count = SendMessageW(hList, LB_GETSELCOUNT, 0, 0);
						
						if(count != LB_ERR)
						{
							bool removeTrig = false;
							// We only want to continue if one and only one item is
							// selected.

							if(count == 1)
							{

								//place string into text box
								index = SendMessageW(hList, LB_GETCURSEL, 0, 0L);
								if (index == LB_ERR)
								{
									DisplayError (hwnd, L"Something has gone wrong with the Listbox", errCode, 0);
									if (currPathW) free (currPathW);
									return 0;
								}

								

								sendMessageErr = SendMessageW(hList, LB_GETTEXT, index, (LPARAM)currPathW);
								SetDlgItemTextW(hwnd, IDC_TEXT, currPathW);


								// Since we know ahead of time we're only getting one index, there's no need to allocate an array.
								switch (dblclkLevel)
								{
								case 0:
									break;
								case 1:
									{

									if (index >= rootFolderCS + rootFolderCW)
									{
										SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Line\0");
										if (foundNTDLL)
											{
											if (index >= listTotal - branchLevel)
											{
											EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true);
											}
											else
											{
												if (branchLevelIncCum)
												{
												EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
												}
												else //This is broken- fair bit of coding to delete older branches
												{
												EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true);
												}
											
											}
											}
										else
										{
										EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true);
										}
									}
									else
									{

										SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Dir\0");

										if (wcsstr(dacfoldersW[index], lPref) == nullptr)
											//Check for wide string folder here
										{
											EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), removeButtonEnabled);
										}
										else
										{
												//Cannot remove short folders
											EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
										}
									}


									if (sendMessageErr != LB_ERR)
									{
										// Get the data we associated with the item above (the number of times it was added)
										idata = SendMessageW(hList, LB_GETITEMDATA, (WPARAM)index, 0); //lparam not used, but return value IS value of lparam in setitemdata
										//SO idata becomes ntimes when items are added, but not reliable! http://stackoverflow.com/questions/25337801/why-is-lb-getitemdata-returning-0

										if (idata)
										{
											SetDlgItemTextW(hwnd,IDC_STATIC_TWO, L"This entry is repeated");
											SetDlgItemTextW(hwnd,IDC_STATIC_THREE, L"times.");
											SetDlgItemInt(hwnd, IDC_SHOWCOUNT, idata, FALSE);
										}
									
										else
										{
											SetDlgItemInt(hwnd, IDC_SHOWCOUNT, index, FALSE);
											SetDlgItemTextW(hwnd,IDC_STATIC_TWO, L"This entry is ranked");
											SetDlgItemTextW(hwnd,IDC_STATIC_THREE, L"on the list.");
										}
										//This function performs like:
										//TCHAR buf[16];
										//wnsprintf(buf, 16, bSigned ? TEXT("%i") : TEXT("%u"), uValue);
										//SetDlgItemText(hwnd, nIDDlgItem, buf);



									}
									else 
									{
										errCode = 0;
										DisplayError (hwnd, L"Error getting selected item", errCode, 0);
									}


									}
									break;
								default:
									{
										if (index <= folderIndex)
										{

											SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Dir\0");
											if (index == 0 || index == folderIndex)
											{
											SetDlgItemTextW(hwnd,IDC_STATIC_ZERO, L"");
											EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
											}
											else
											{
											SetDlgItemTextW(hwnd,IDC_STATIC_ZERO, L"Dir:");
											EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true);
											}
										}
										else
										{
											SetDlgItemTextW(hwnd,IDC_STATIC_ZERO, L"File:");
											SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Move file\0");
											EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true);

										}
									}
								}



							}
							else 
							{
								// No items selected, or more than one.Either way, we aren't going to process this.
									
								sendMessageErr = SendMessageW(hList, LB_GETANCHORINDEX, 0, 0L);
								
								int * selItems = (int *)GlobalAlloc(GPTR, sizeof(int) * count);
								sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_GETSELITEMS, count, (LPARAM)(LPINT)selItems);
								if (dblclkLevel) 
								{
										for (i = 0; i < count; i++)
										{
											if (selItems[i] <= folderIndex)
											{
												EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
												SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del\0");
												removeTrig = true;
												break;
											}
										}
											if (removeTrig)
											{



											for (i = 0; i < count; i++)
											{
												if (selItems[i] > folderIndex)
												{
													EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
													SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del\0");
													break;
												}
											}

											}
											else
											{
											SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Move Files\0");
											EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true);
											}



								}
								else
								{

									if (listTotal > rootFolderCS + rootFolderCW)
									{
										for (i = 0; i < count; i++)
										{
											if (selItems[i] < listTotal - branchLevel)
											{
												EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
												SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del\0");
												removeTrig = true;
											}

										}
										if (removeTrig)
										{
										for (i = 0; i < count; i++)
										{
											if (selItems[i] >= listTotal - branchLevel)
											{
												EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
												SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del\0");
												break;
											}

										}
										}
										else
										{
										SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Line\0");
										EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true);

										}



									}
									else
									{
										for (i = 0; i < count; i++)
										{
											if (selItems[i] < rootFolderCS)
											{
											SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Dir\0");
											EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
											removeTrig = true;
											}

										}
											if (!removeTrig)
											{
											SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Dir\0");
											EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true);
											}
										
									}

								}

								GlobalFree((HANDLE)selItems);
								SetDlgItemTextW(hwnd, IDC_SHOWCOUNT, L"-");





							}
						}
						else
						{
							errCode = 0;
							DisplayError (hwnd, L"Error counting items", errCode, 0);
						}
						if (currPathW) free (currPathW);
					}
					break;
					case LBN_DBLCLK:
					{

					HWND hList = GetDlgItem(hwnd, IDC_LIST);
					index = SendMessageW(hList, LB_GETCURSEL, 0, 0L);
					//do nothing if clicking a file

					currPath = (char *)calloc(maxPathFolder, sizeof(char));
					currPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
					findPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
					if ((currPath == nullptr) || currPathW == nullptr || (findPathW == nullptr))
					{
					DisplayError (hwnd, L"Something has gone wrong with memory!", errCode, 0);
					return 0;
					}

					sendMessageErr = SendMessageW(hList, LB_GETTEXT, index, (LPARAM)findPathW);
					if (wcsstr(findPathW, L"not ready") || wcsstr(findPathW, L"Unknown")) goto DblclkEnd;;

					switch (dblclkLevel)
					{
					case 0:
						dblclkLevel = 1;
						driveIndex[0] = driveInfo[index][0];
						driveIndex[1] = driveInfo[index][1];
						driveIDBaseW[4] = driveIndex[0];
						driveIDBaseWNT[4] = driveIndex[0];
						driveIDBase[0] = (char)driveIndex[0];
						driveIDBaseAW[0] = driveIndex[0];
						goto DblclkEnd;
						break;
					case 1:
						if (index >= rootFolderCS + rootFolderCW)
						{	
							goto DblclkEnd;
						}
						else
							if (index < rootFolderCS)
							{
								wcscpy_s(dblclkPath[1], maxPathFolder, driveIDBaseAW);
							}
							else
							{
								wcscpy_s(dblclkPath[1], maxPathFolder, driveIDBaseW);
							}	
						break;
					default:
						if (index >= folderIndex) return 0;
					}

					wcscpy_s(dblclkString, pathLength, dblclkPath[1]);

					if (0 == wcscmp(findPathW, L".."))
					{
						dblclkLevel -=1;
						if (dblclkLevel < 2) goto DblclkEnd;
							for (i = 2; i <= dblclkLevel; i++)
							{
							wcscat_s(dblclkString, pathLength, dblclkPath[i]);
							wcscat_s(dblclkString, pathLength, L"\\");
							}
							wcscpy_s(findPathW, maxPathFolder, dblclkString);
							if (!SetCurrentDirectoryW (findPathW))
							{
								ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
								dblclkLevel = 0;
							goto DblclkEnd;
						}
					}
					else
					{
						if (!SetCurrentDirectoryW (findPathW))
						{
							ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
							dblclkLevel = 0;
							goto DblclkEnd;
						}
						if (dblclkLevel == treeLevelLimit)
						{
							ErrorExit (L"Double Click No more!", 0);
							dblclkLevel = 0;
							goto DblclkEnd;
						}

						dblclkLevel +=1;
						if (dblclkLevel == 2)
						{
							wchar_t * dblclkPathtmp;
							if (index < rootFolderCS)
								{
									dblclkPathtmp = findPathW + 3;
									wcscpy_s(dblclkPath[2], maxPathFolder, dblclkPathtmp);
								}
								else
								{
									dblclkPathtmp = findPathW + 7;
									wcscpy_s(dblclkPath[2], maxPathFolder, dblclkPathtmp);
								}	
							wcscpy_s(dblclkString, pathLength, findPathW);
							wcscat_s(dblclkString, pathLength, L"\\");
						}
						else
						{
							wcscpy_s(dblclkPath[dblclkLevel], maxPathFolder, findPathW);
							wcscpy_s(dblclkString, pathLength, dblclkPath[1]);
							for (i = 2; i <= dblclkLevel; i++)
							{
							wcscat_s(dblclkString, pathLength, dblclkPath[i]);
							wcscat_s(dblclkString, pathLength, L"\\");
							}

						}
						
					}

					DblclkEnd:


					switch (dblclkLevel)
					{
					case 0:
						{
							//DragAcceptFiles (hwnd, FALSE);
							InitProc(hwnd);
							EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
							EnableWindow(GetDlgItem(hwnd, IDC_CREATE), false);
							EnableWindow(GetDlgItem(hwnd, IDC_ADD), false);
							//enable buttons
						}
						break;
					case 1:
						{

							if ((driveInfo[index][0] == L'X')) //X: Disk not ready
							{
								dblclkLevel = 0;
								InitProc(hwnd);
								EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
								EnableWindow(GetDlgItem(hwnd, IDC_CREATE), false);
								EnableWindow(GetDlgItem(hwnd, IDC_ADD), false);
							}
							else
							{
								InitProc(hwnd);

								//U: Unknown, X: Disk not ready, M: Removable, F: Fixed disk, B: Network, C: CD/DVD, R Ramdisk
								if ((driveIndex[1] == L'C') || (driveIndex[1] == L'X') || (driveIndex[1] == L'U'))
									{
									EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
									EnableWindow(GetDlgItem(hwnd, IDC_ADD), false);
									}
							}
						}
						break;
					default:
						{
							doFilesFolders(hwnd);
						}
					}
							if (findPathW) free (findPathW);
							if (currPathW) free (currPathW);
							if (currPath) free (currPath);
					
					}
					break; //This break for consistency
				}
			break;
			} //end WM_COMMAND
		break;

			case WM_DROPFILES:
				{
				HWND hList = GetDlgItem(hwnd, IDC_LIST);
				wchar_t *dropBuf;
				int pdest;
				bool fileExisting;
				dropBuf = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
				currPathW = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
				tempDest = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
				if ((currPathW == nullptr) || (dropBuf == nullptr) || (tempDest == nullptr))
				{
				DisplayError (hwnd, L"Something has gone wrong with memory!", errCode, 0);
				return 0;
				}
				

				if ((dblclkLevel))
				{

				_snwprintf_s(hrtext, _countof(hrtext), _TRUNCATE, L"Files, not folders are processed in this window. Click Yes to continue moving the selection to the target directory:\n\n %s", dblclkString);
				if (DisplayError (hwnd, hrtext, errCode, 1))
					{


					int n = 0;
					int count = DragQueryFileW((HDROP) wParam, 0xFFFFFFFF, 0, 0 );
					while ( n < count )
					{
					if (!DragQueryFileW ((HDROP) wParam, n, dropBuf, pathLength)) DisplayError (hwnd, L"DragQuery: Failed", 0, 0);
					fileExisting = false;
					pdest = (int)(wcsrchr( dropBuf, separatorFTA ) - dropBuf + 1);
					if (GetFileAttributesW(dropBuf) == FILE_ATTRIBUTE_DIRECTORY) break;
					wcscpy_s(currPathW, pathLength, dblclkString);
					wcscat_s(currPathW, pathLength, &dropBuf[pdest]);
					// prepend "\\?\" to the path.
					wcscpy_s(tempDest, pathLength, lPref);
					wcscat_s(tempDest, pathLength, dropBuf);
					if (CopyFileW(tempDest, currPathW, FALSE))
					{
						//check if entry already exists
						for (i = folderIndex; i < (j = SendMessageW(hList,LB_GETCOUNT,NULL,NULL)); i++)
						{
							sendMessageErr = SendMessageW(hList, LB_GETTEXT, i, (LPARAM)currPathW);
							if (0 == wcscmp(currPathW, &dropBuf[pdest])) fileExisting = true;
						}
						
						if (!fileExisting) sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)&dropBuf[pdest]);
					} 
					else ErrorExit (L"CopyFile: Copy of dragged file error: ", 0);
					n++;
					}
					currPathW[0] = L'\0';
					doFilesFolders(hwnd);

					}

				}

				else //only drag in folders to set path
				{


					if ( DragQueryFileW((HDROP) wParam, 0xFFFFFFFF, 0, 0 ) == 1)
					{
					if (!DragQueryFileW ((HDROP) wParam, 0, dropBuf, pathLength)) DisplayError (hwnd, L"DragQuery: Failed", 0, 0);
					pdest = (int)(wcsrchr( dropBuf, separatorFTA ) - dropBuf + 1);
					if (GetFileAttributesW(dropBuf) == FILE_ATTRIBUTE_DIRECTORY)
					{
					if (!SetCurrentDirectoryW (dropBuf)) //objects to L".."
						{
							ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
						}
					else
						{
							dblclkLevel = 1;
							wcscpy_s(dblclkPath[dblclkLevel], maxPathFolder, lPref);
							wchar_t *tokStr;
							wchar_t *tokDest = wcstok_s( dropBuf, &separatorFTA, &tokStr);
							//tokDest = wcstok_s(NULL, &separatorFTA, &tokStr);
							wcscat_s(dblclkPath[dblclkLevel], maxPathFolder, tokDest);
							for (dblclkLevel = 2; dblclkLevel <= treeLevelLimit; dblclkLevel++)
							{

								if ((tokDest = wcstok_s(NULL, &separatorFTA, &tokStr))== nullptr) break;
								wcscpy_s(dblclkPath[dblclkLevel], maxPathFolder, tokDest);
							}
							dblclkLevel -= 1; //Ouch
						wcscat_s(dblclkPath[1], maxPathFolder, L"\\");
						wcscpy_s(dblclkString, pathLength, dblclkPath[1]);
						wcscat_s(dblclkString, pathLength, L"\\");
						for (i = 2; i <= dblclkLevel; i++)
						{
						wcscat_s(dblclkString, pathLength, dblclkPath[i]);
						wcscat_s(dblclkString, pathLength, L"\\");
						}
						}

					}
					if (!SetCurrentDirectoryW (dblclkString))
					{
						ErrorExit (L"SetCurrentDirectoryW: Non zero. Must be a Folder, a shortcut or symlink", 0);
						dblclkLevel = 0;
					}
					else
					{
					doFilesFolders(hwnd);
					}
					EnableWindow(GetDlgItem(hwnd, IDC_NUMBER), true);
					EnableWindow(GetDlgItem(hwnd, IDC_TEXT), true);
					}

				}

				DragFinish(hDropInfo);
				free (dropBuf);
				free (currPathW);
				free (tempDest);
				}
		break;
			/*case WM_KEYDOWN: 
            switch (wParam) 
            {

			}*/
		case WM_CLOSE:
			{
			//Cleanup
			if (weareatBoot) Kleenup (hwnd);
			 
				if (foundNTDLL)
				{
					if (!CloseNTDLLObjs(true)) DisplayError (hwnd, L"NtCreateFile: Objects failed to close", errCode, 0);
				}
				#if defined(_WIN64) || defined(_WIN32) ||  defined(__TOS_WIN__)
				__try
				{
					_invalid_parameter_handler oldHandler, newHandler;
					newHandler = (_invalid_parameter_handler)ThisInvalidParameterHandler;
					oldHandler = _set_invalid_parameter_handler(newHandler);
					_CrtSetReportMode(_CRT_ASSERT, 0);
				}
				__except (GetExceptionCode() == EXCEPTION_INVALID_HANDLE ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
				{
					// No debugger is attached, so return FALSE and continue.
					ErrorExit(L"Created Directory Handle not an OS Handle: This occurs in debug only. See Github Issues.", errCode);
				}
				#endif			
				_CrtDumpMemoryLeaks();
				EndDialog(hwnd, 0);
				if (IsWindowsVistaOrGreater()) if (exeHandle != INVALID_HANDLE_VALUE) CloseHandle(exeHandle);

			}
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
			if (!(UnregisterClassW(APP_CLASS_NAME, appHinstance))) DisplayError (hwnd, L"App class failed to unregister", 0, 0);
		break;
		default: return FALSE;
		break;	
	}
	return TRUE;
}

INT_PTR WINAPI AboutDlgProc(HWND aboutHwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PSID Sid = nullptr;
	LPWSTR StringSid;
	
	switch(uMsg)
	{
	case WM_INITDIALOG:
	{
	CreateHyperLink(GetDlgItem(aboutHwnd, IDC_STATIC_FOUR));
	CreateHyperLink(GetDlgItem(aboutHwnd, IDC_STATIC_FIVE));
	if (GetAccountSidW(NULL, &Sid)) DisplayError (aboutHwnd, L"Unable to retrieve account ID", errCode, 0);
	//When the user clicks the Remove button, we first get the number of selected items
	ConvertSidToStringSidW(Sid, &StringSid);
	currPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
	wcscpy_s(currPathW, maxPathFolder, L"B.D. Help for {");
	wcscat_s(currPathW, maxPathFolder, StringSid);
	wcscat_s(currPathW, maxPathFolder, L"}");
	SendMessageW (aboutHwnd, WM_SETTEXT, 0, (LPARAM)currPathW);
	free (currPathW);
	//wcscat_s(tempDest, pathLength, L"$Recycle.Bin\\"); //"Recycler" on XP //originally for recycle fn here //SetWindowTextW (aboutHwnd, (wchar_t *)Sid);
	}
	break;
	//case WM_KEYDOWN needs a child control
	case WM_COMMAND:
    switch (LOWORD(wParam))
            {
				case IDC_STATIC_FOUR:
				{
				if(HIWORD(wParam) == BN_CLICKED)
					{
						ShellError(aboutHwnd, ShellExecuteW(NULL, L"open", L"http://www.codeproject.com/Tips/1089681/Bigger-Directories", NULL, NULL, SW_SHOWNORMAL));
					}
				}           
				break;
				case IDC_STATIC_FIVE:
				{
					{
						ShellError(aboutHwnd, ShellExecuteW(NULL, L"open", L"https://github.com/lmstearn/Bigger-Directories/wiki", NULL, NULL, SW_SHOWNORMAL));
					}

				}           
				break;

				case IDC_OK:
				EndDialog(aboutHwnd, IDC_OK);
				break;

				case IDC_RES:

				resResult +=1;
				if (resResult == 5) resResult = 1;
				if (resResult < 3 && !resWarned)
					{
						if (DisplayError (aboutHwnd, L"Under Construction: Form testing only: No controls visible? Spacebar toggles forms. Click Yes to continue", 0, 1))
						{
							PlaySoundW(MAKEINTRESOURCEW(IDW_CLICK), (HMODULE)GetWindowLongW(aboutHwnd, GWLP_HINSTANCE), SND_RESOURCE | SND_ASYNC);
							resWarned = true;
						}
						else
						{
							return 0;
						}
					}


				SwitchResolution (aboutHwnd, AboutDlgProc);
				EndDialog(aboutHwnd, IDC_OK);
				break;

			}

			break;
	case WM_CLOSE:
			{
				EndDialog(aboutHwnd, IDC_OK);
			}
			default: return FALSE;

	//default: return DefWindowProc(aboutHwnd, uMsg, wParam, lParam); //this really breaks stuff even with the WM_SETTEXT
	//probably message deadlock? https://msdn.microsoft.com/en-us/library/ms644927(v=VS.85).aspx#deadlocks
	}
return FALSE;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	
	if (pCmdLine[0] != L'\0')
	{
	//also https://msdn.microsoft.com/en-us/library/windows/desktop/bb776391%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
	memset(rootDir, L'\0', sizeof(rootDir));
	wcscpy_s (rootDir, pathLength, (wchar_t *) pCmdLine);
	pCmdLineActive = true;
	}
	//else
	
	_invalid_parameter_handler oldHandler, newHandler;
	newHandler = (_invalid_parameter_handler)ThisInvalidParameterHandler;
	oldHandler = _set_invalid_parameter_handler(newHandler);
	_CrtSetReportMode(_CRT_ASSERT, 0); // Disable the message box for assertions.  


	// Create a new window see https://msdn.microsoft.com/en-us/library/windows/desktop/ff381397(v=vs.85).aspx
	WNDCLASSW ReschkC = { };
	ReschkC.lpfnWndProc   = RescheckWindowProc;
	ReschkC.hInstance     = hInstance;
	ReschkC.lpszClassName = TEMP_CLASS_NAME;
	if (!RegisterClassW(&ReschkC)) ErrorExit (L"Cannot register Rescheck window!!!?", 0);

	HWND hwnd = CreateWindowExW(
		0,								// Optional window styles.
		TEMP_CLASS_NAME,						// Window class
		L"Nada",	// Window text
		WS_OVERLAPPEDWINDOW,			// Window style
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,	//Size and position
		NULL,							// Parent window
		NULL,							// Menu
		hInstance,						// Instance handle of owning application (wWinMain)
		NULL							// Additional application data
		);

	if (hwnd == NULL)
	{
		ErrorExit (L"Cannot create the Rescheck window!!!?", 0);
		return 0;
	}
	else
	{

		resResult = DoSystemParametersInfoStuff(hwnd, true);
		SwitchResolution (nullptr, APP_CLASS::s_DlgProc);
	}
return 0; //never gets here, but suppress C4715 warning
}

int PopulateListBox (HWND hwnd, BOOL widecharNames, BOOL listFolders)
{
BOOL findhandle = TRUE;
int listNum = 0;

if (listFolders)
{

	if (widecharNames == 0)
	{
		memset(&da, 0, sizeof(WIN32_FIND_DATAA));
		strcpy_s(currPath, maxPathFolder, driveIDBase);

		strcat_s(currPath, maxPathFolder, "*");
		ds = FindFirstFileA(currPath, &da);
		listNum = 1;

	}
	else
	{
		memset(&dw, 0, sizeof(WIN32_FIND_DATAW));
		//http://stackoverflow.com/questions/32540779/wcscpy-does-not-accept-tchar-in-destination-variable
		(dblclkLevel > 1)? wcscat_s(currPathW, maxPathFolder, dblclkString): wcscpy_s(currPathW, maxPathFolder, driveIDBaseW);
		wcscat_s(currPathW, maxPathFolder, L"*");
		//findHandle = FindFirstFile(@"\\?\UNC\" + folder_path, out findData
		ds = FindFirstFileW(currPathW, &dw); //dw points to found folders

	}


	if (ds == INVALID_HANDLE_VALUE && (widecharNames == 0))
	{
		errCode = -3;
		DisplayError (hwnd, L"No directories found", errCode, 0);
		return false;
	}

	//There's an internal index that is reset to 0 each time you call FindFirstFile() and it's incremented each time you call FindNextFile() so unless you do it in a loop, you'll only get the first filename ( a dot ) each time.
	while (ds != INVALID_HANDLE_VALUE && findhandle)
	{

		if (widecharNames == 0)
		{
			if ((da.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(da.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM
				|| da.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT || da.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE || da.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {

				//define FILE_ATTRIBUTE_READONLY              0x00000001
				//#define FILE_ATTRIBUTE_HIDDEN               0x00000002
				//#define FILE_ATTRIBUTE_SYSTEM               0x00000004
				//#define FILE_ATTRIBUTE_DIRECTORY            0x00000010
				//#define FILE_ATTRIBUTE_ARCHIVE              0x00000020
				//#define FILE_ATTRIBUTE_DEVICE               0x00000040
				//#define FILE_ATTRIBUTE_NORMAL               0x00000080
				//#define FILE_ATTRIBUTE_TEMPORARY            0x00000100
				//#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200
				//#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400
				//#define FILE_ATTRIBUTE_COMPRESSED           0x00000800
				//#define FILE_ATTRIBUTE_OFFLINE              0x00001000
				//#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000
				//#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000
				//#define FILE_ATTRIBUTE_VIRTUAL              0x00010000

				strcpy_s(currPath, maxPathFolder, (char *)driveIDBase);
				strcat_s(currPath, maxPathFolder, da.cFileName);


				strcat_s(dacfolders[listNum], maxPathFolder, currPath);
				if (listNum < branchLimit) 
				{
					listNum += 1;
				}
				else
				{
					DisplayError (hwnd, L"Limit of folders reached!", errCode, 0);
					break;
				}

				sendMessageErr = SendDlgItemMessageA(hwnd, IDC_LIST, LB_ADDSTRING, (WPARAM)(listNum), (LPARAM)currPath); // wparam cannot exceed 32,767 
					
			}
			findhandle = FindNextFileA(ds, &da);
		}
		else
		{
			if ((dw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(dw.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM
				|| dw.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT || dw.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE || dw.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
			{


			if (dblclkLevel > 1)
			{
				wcscpy_s(currPathW, maxPathFolder, dw.cFileName);
			}
			else
			{
			wcscpy_s(currPathW, maxPathFolder, driveIDBaseW);
			wcscat_s(currPathW, maxPathFolder, dw.cFileName);
			//compare with dacfolders[rootFolderC] to check for dups
			wcscat_s(dacfoldersW[listNum], maxPathFolder, currPathW);
			}
			if (listNum < branchLimit) 
				{
					listNum += 1;
				}
			else
				{
					DisplayError (hwnd, L"Limit of folders reached", errCode, 0);
					break;
				}

			sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_ADDSTRING, (WPARAM)(rootFolderCS + listNum), (LPARAM)currPathW); // wparam cannot exceed 32,767 
				
			}
		findhandle = FindNextFileW(ds, &dw);

		}
		sendMessageErr = SendDlgItemMessage(hwnd, IDC_LIST, LB_SETITEMDATA, (WPARAM)(rootFolderCS + listNum), (LPARAM)(rootFolderCS + listNum));
	//(lparam)rootFolderC required for getitemdata
	//The Notification Code is passed as the HIWORD of wParam, the other half of the parameter that gave us the index of the control identifier in the first place. 
	//HIWORD is the Upper 16 bits of UINT and LOWORD is the Lower 16 bits of UINT
	//DWORD is just a typedef for 32-bit integer, whereas WORD is a typedef for a 16-bit integer
	}

	FindClose(ds);

}
else
{




	if (dblclkLevel > 1)

	{
		listNum = 0;
		findhandle = TRUE;
		memset(&dw, 0, sizeof(WIN32_FIND_DATAW));
		//http://stackoverflow.com/questions/32540779/wcscpy-does-not-accept-tchar-in-destination-variable
		wcscpy_s(currPathW, maxPathFolder, dblclkString);
		wcscat_s(currPathW, maxPathFolder, L"*");

		ds = FindFirstFileW(currPathW, &dw);


	if (ds == INVALID_HANDLE_VALUE)
	{
		errCode = -3;
		DisplayError (hwnd, L"No files found", errCode, 0);
		return false;
	}


	while (ds != INVALID_HANDLE_VALUE && findhandle)
	{

			//Symbolic links, junction points and mount points are all reparse points
			if (!(dw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ||	dw.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT || dw.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM || dw.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
			{
				//|| dw.dwFileAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED || dw.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE || dw.dwFileAttributes & FILE_ATTRIBUTE_VIRTUAL || dw.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY || dw.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE || dw.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED || dw.dwFileAttributes & FILE_ATTRIBUTE_DEVICE

			wcscpy_s(currPathW, maxPathFolder, dw.cFileName);
			if (listNum < maxDWORD - folderIndex) 
				{
					listNum += 1;
				}
			else
				{
					DisplayError (hwnd, L"Limit of files in listbox reached", errCode, 0);
					break;
				}				
			//compare with dacfolders[rootFolderC] to check for dups
			sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_ADDSTRING, (WPARAM)(rootFolderCS + listNum), (LPARAM)currPathW); // wparam cannot exceed 32,767 
				
			}
		findhandle = FindNextFileW(ds, &dw);

	}
		sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_SETITEMDATA, (WPARAM)(rootFolderCS + listNum), (LPARAM)(rootFolderCS + listNum));

	FindClose(ds);

	}
}

return listNum;

}

void TextinIDC_TEXT (HWND hwnd)
{
	wchar_t *pszTxt0 = L"Dblclick or drag in folder to start"; //fixed allocation
	wchar_t *pszTxt1 = L"My Bigger Directory Path";
	wchar_t pszDest[arraysize];
	HRESULT hr;
	size_t cbDest = arraysize * sizeof(wchar_t); //the use of size_t implies C++ compile.
	LPCWSTR pszFormat = L"%s";


	(dblclkLevel)? hr = StringCbPrintfW(pszDest, cbDest, pszFormat, pszTxt1): hr = StringCbPrintfW(pszDest, cbDest, pszFormat, pszTxt0); //swprintf_s
	if (hr == S_OK)
	{
		SetDlgItemTextW(hwnd, IDC_TEXT, pszDest);
	}
	else
	{

		DisplayError (hwnd, L"StringCbPrintfW didn't work, quitting: code %#08X", (int)hr, 0);
	}

}

int DoSystemParametersInfoStuff(HWND hwnd, bool progLoad)
{
	HMONITOR hMon = MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);

	if (progLoad) //destroy ephemeral window 
	{
	if (!DestroyWindow(hwnd)) DisplayError (hwnd, L"Rescheck window cannot be destroyed", 0, 0);
	if (!(UnregisterClassW(TEMP_CLASS_NAME, appHinstance))) DisplayError (hwnd, L"Rescheck class failed to unregister", 0, 0);
	}

if (GetMonitorInfo (hMon, &monInfo))
{
	
	if ((float)abs(monInfo.rcMonitor.right - monInfo.rcMonitor.left) / (float)abs(monInfo.rcMonitor.top - monInfo.rcMonitor.bottom) < 1.5) wideScr = false;


	if ((monInfo.rcMonitor.right - monInfo.rcMonitor.left) > 5000) return 1;
	if ((monInfo.rcMonitor.right - monInfo.rcMonitor.left) > 3000) return 2;
	if ((monInfo.rcMonitor.right - monInfo.rcMonitor.left) > 2000) return 3;
	if ((monInfo.rcMonitor.right - monInfo.rcMonitor.left) > 850) return 4;
	else return 5;
}
else
{
	DisplayError (hwnd, L"GetMonitorInfo: Cannot get info", 0, 0);
}
return 0;

}
int SwitchResolution (HWND hwndParent, INT_PTR (WINAPI * dProc)(HWND, UINT, WPARAM, LPARAM))
{

if (hwndParent) //About dialogue
{
	
	switch (resResult)
	{
		//(HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE) alternative to appHinstance. GetWindowLongPtr (64 bit) probably should be used
		case 1:
			return DialogBoxW(appHinstance, MAKEINTRESOURCEW(IDD_HELP4320P), hwndParent, (DLGPROC)dProc);
		break;
		case 2:
			return DialogBoxW(appHinstance, MAKEINTRESOURCEW(IDD_HELP2160P), hwndParent, (DLGPROC)dProc);
		break;
		case 3:
			return DialogBoxW(appHinstance, MAKEINTRESOURCEW(IDD_HELP1080P), hwndParent, (DLGPROC)dProc);
		break;
		default:
			return DialogBoxW(appHinstance, MAKEINTRESOURCEW(IDD_HELP768P), hwndParent, (DLGPROC)dProc);
		break;
	}

}



else 
{

	WNDCLASSEXW RSC_CLASS = { };
	appHinstance = GetModuleHandleW(NULL); //same as hInstance: use for application hInstance: (okay for exe not for DLL)
	
	if( !GetClassInfoExW( NULL, L"#32770", &RSC_CLASS )) ErrorExit (L"Cannot get App Class!?", 0);
	//The Dialog class is the default Window Class #32770, already registered,

	//These most likely not required
	//HRSRC hRsc = FindResource(NULL, MAKEINTRESOURCE(IDD_768P), RT_DIALOG );
	//RSC_CLASS.hInstance = (HINSTANCE)LoadResource(NULL, hRsc);

	RSC_CLASS.cbSize        = sizeof(RSC_CLASS);
	RSC_CLASS.hInstance = appHinstance;
	RSC_CLASS.lpszClassName = APP_CLASS_NAME;
	if (!RegisterClassExW(&RSC_CLASS)) ErrorExit (L"Cannot register Application Window!!!?", 0);

	HWND hwnd = CreateWindowExW(
		0,												// Optional window styles.
		APP_CLASS_NAME,									// Window class
		L"BiggerDirectories",							// Window text
		WS_OVERLAPPEDWINDOW | WS_EX_ACCEPTFILES,		// Window style
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,	//Size and position: X, Y, Width, Height
		HWND_DESKTOP,									// Parent window
		NULL,											// Menu
		appHinstance,									// Instance handle of owning application (wWinMain)
		NULL											// Additional application data
		);

APP_CLASS wnd;
//APP_CLASS* wnd =(APP_CLASS*)malloc(sizeof(APP_CLASS));
//wnd = new APP_CLASS();


MSG msg;
while (GetMessageW(&msg, NULL, 0, 0))
{
TranslateMessage(&msg);
DispatchMessage(&msg);
}
return 0;
}

}


int GetBiggerDirectoriesPath (HWND hwnd, wchar_t *exePath)
{
DWORD result;


    //  Disable redirection immediately prior to the native API
    //  function call.
    
if (exe64Bit)
{
		result  = GetModuleFileNameW(nullptr, exePath, MAX_PATH-4);
	    errCode = (int)GetLastError();
}
else
{
	 
	if (wow64Functions)
	{
		if (DisableWOW64RedirectionIfNecessary(&OldValue))
		{
		result  = GetModuleFileNameW(nullptr, exePath, MAX_PATH-4);
		errCode = (int)GetLastError();
		}
		else
		{
			DisplayError (hwnd, L"Problems with redirection", errCode, 0);
			return 1;
		}
	}
	else // For 32bit XP machines
	{
		char *exePathA = (char *)malloc( sizeof (char));
		size_t i; 
		wcstombs_s(&i, exePathA, sizeof (char), exePath, sizeof (wchar_t));
		result = GetModuleFileNameA(nullptr, exePathA, MAX_PATH-4);
		errCode = (int)GetLastError();
		free (exePathA);
	}
	if (wow64Functions)
	{
    if (RevertWOW64RedirectionIfNecessary(&OldValue))
	{
		DisplayError (hwnd, L"Problems with redirection", errCode, 0);
		return 1;
	}
	}
}

    if (0 == result)
    {
        exePath = 0;
        return 1;
    }
    else if (result == (int)MAX_PATH-4)
    {
        /* May need to also check for ERROR_SUCCESS here if XP/2K */
        if (ERROR_INSUFFICIENT_BUFFER != errCode)
        {
            return 1;
        }
    }


if (!exePath)
{
    switch (errCode)
	{
		case 2:
			DisplayError (hwnd, L"Problems: FILE_NOT_FOUND", errCode, 0);
		break;
		case 50:
			DisplayError (hwnd, L"Problems: NOT_SUPPORTED", errCode, 0);
		break;
		case 110:
			DisplayError (hwnd, L"Problems: OPEN_FAILED", errCode, 0);
		break;
		case 113:
			DisplayError (hwnd, L"Problems: NO_MORE_SEARCH_HANDLES", errCode, 0);
		break;
		case 114:
			DisplayError (hwnd, L"Problems: INVALID_TARGET_HANDLE", errCode, 0);
		break;
		case 123:
			DisplayError (hwnd, L"Problems: INVALID_NAME", errCode, 0);
		break;
		case 126:
			DisplayError (hwnd, L"Problems: MODULE_NOT_FOUND", errCode, 0);
		break;
		case 259:
			DisplayError (hwnd, L"Problems: NO_MORE_ITEMS", errCode, 0);
		break;
		case 303:
			DisplayError (hwnd, L"Problems: DELETE_PENDING", errCode, 0);
		break;
		case 310:
			DisplayError (hwnd, L"Problems: INVALID_EXCEPTION_HANDLER", errCode, 0);
		break;
		case 335:
			DisplayError (hwnd, L"Problems: Cannot run this out of an archive: COMPRESSED_FILE_NOT_SUPPORTED", errCode, 0);
		break;
		default:
			{
			DisplayError (hwnd, L"Unknown error has occurred", errCode, 0);
			}
	}
	return 1;
}
else
{
	return 0;
}
}
bool Kleenup (HWND hwnd)
{
	STARTUPINFOW lpStartupInfo;
	PROCESS_INFORMATION lpProcessInfo; // = {0} not recommended -suppresses warning when closed

	ZeroMemory(&lpStartupInfo, sizeof(lpStartupInfo)); //zeros all members
	lpStartupInfo.cb = sizeof(lpStartupInfo);
	ZeroMemory(&lpProcessInfo, sizeof(lpProcessInfo));

	thisexePath = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
	tempDest = (wchar_t *)calloc(pathLength, sizeof(wchar_t));

	if (pCmdLineActive) //on program restart on remove root directory bug
	{
		if (!GetModuleFileNameW(nullptr, thisexePath, pathLength) || (wcslen(thisexePath) > pathLength))
		{
			DisplayError (hwnd, L"Oops, process path too long!? or non-existent?! Quitting", 0, 0);
			free (tempDest);
			free (thisexePath);
			CloseHandle(lpProcessInfo.hProcess);
			CloseHandle(lpProcessInfo.hThread);
			return false;
		}
		else
		{
		wcscpy_s (tempDest, pathLength, L"\"");
		wcscat_s (tempDest, pathLength, thisexePath);
		wcscat_s (tempDest, pathLength, L"\" ");
		wcscat_s (tempDest, pathLength, pathToDeleteW);
		if (!CreateProcessW (thisexePath, tempDest, nullptr, nullptr, FALSE, NULL, nullptr, nullptr, &lpStartupInfo, &lpProcessInfo)) ErrorExit (L"Oops: Something went wrong. Please restart the program...", 0);
		}
	}
	else
	{

			system ("CD\\ & PUSHD %SystemRoot%\\Temp & REG QUERY \"HKLM\\Hardware\\Description\\System\\CentralProcessor\\0\" | FIND /i \"x86\" >NUL && CALL SET \"OSB=\" || CALL SET \"OSB=64BIT\" & SET KEY_NAME=\"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" & SET \"VALUE_NAME=Userinit\" & SET \"Userinitreg=\" & (IF EXIST Userinitreg.txt CALL SET \"Userinitreg=TRUE\") & (IF DEFINED Userinitreg (FOR /F \"usebackq delims=\" %G IN (\"Userinitreg.txt\") DO SET \"REGVALUE=%G\" & (IF DEFINED OSB (CALL REG ADD %KEY_NAME% /v %VALUE_NAME% /d %REGVALUE% /f /reg:64) ELSE (CALL REG ADD %KEY_NAME% /v %VALUE_NAME% /d %REGVALUE% /f))) ELSE (CALL ECHO Backup reg record does not exist. Was NoLogon already called? & PAUSE >NUL)) & (IF EXIST Userinitregerror.txt (DEL \"Userinitregerror.txt\")) & DEL \"Userinitreg.txt\" & POPD");
			//CALL SET \"REGVALUE=%REGVALUE%,\" not necessary befroe REG ADD as data is saved.
			if (!ExpandEnvironmentStringsW(L"%systemroot%", tempDest,  pathLength)) ErrorExit (L"ExpandEnvironmentStringsW failed.", 0);
			wcscpy_s(thisexePath, pathLength, tempDest); //Small hole in logic here
			wcscat_s(tempDest, pathLength, L"\\Temp\\BiggerDirectories.exe");

			if (weareatBoot)
				{
				//reset to vanilla
				wcscat_s(thisexePath, pathLength, L"\\system32\\userinit.exe");
				//Create process userinit.exe


				SetLastError(ERROR_INVALID_PARAMETER); //https://msdn.microsoft.com/en-us/library/ms682425(VS.85).aspx
				if (!CreateProcessW(thisexePath, nullptr, nullptr, nullptr, FALSE, NULL, nullptr, nullptr, &lpStartupInfo, &lpProcessInfo)) ErrorExit (L"userinit could not be started through this program. Please reboot after closing this program.", 0);
				//The reg value is restored to userinit before this point
				WaitForSingleObject(lpProcessInfo.hProcess, INFINITE);
				}

			if(!MoveFileExW(tempDest,nullptr,MOVEFILE_DELAY_UNTIL_REBOOT))
			{
			DisplayError (hwnd, L"Problems with file deletion. Solved with next Disk Cleanup", 0, 0);
			free (tempDest);
			free (thisexePath);
			CloseHandle(lpProcessInfo.hProcess);
			CloseHandle(lpProcessInfo.hThread);
			return false;
			}
	}
	free (tempDest);
	free (thisexePath);
	CloseHandle(lpProcessInfo.hProcess);
	CloseHandle(lpProcessInfo.hThread);
	return true;
}
int ExistRegValue ()
{
	bool retVal = false;
	bool openVal = false;
	wchar_t *valueName, *subKeyName;
	HKEY hKey;
	valueName = (wchar_t *)calloc(260, sizeof(wchar_t));
	subKeyName = (wchar_t *)calloc(260, sizeof(wchar_t));
	wcscpy_s(subKeyName, 260, L"System\\CurrentControlSet\\Control\\Session Manager");
	wcscpy_s(valueName, 260, L"PendingFileRenameOperations");
	if (RegOpenKeyExW (HKEY_LOCAL_MACHINE, subKeyName, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
	{
		ErrorExit (L"Could not open Session Manager key!",0);
	}
	else
	{
		openVal = true;
	}
		

	if (RegQueryValueExW(hKey, valueName, nullptr, nullptr, nullptr, nullptr) != ERROR_FILE_NOT_FOUND) retVal = true;

	free (valueName);
	free (subKeyName);

	if (openVal && RegCloseKey (hKey) != ERROR_SUCCESS)
		{
			ErrorExit (L"Could not close Session Manager key!",0);
		}
	return retVal;
		// \??\C:\Windows\Temp\BiggerDirectories.exe: nasty multo_sz

}
DWORD FindProcessId(HWND hwnd, const wchar_t *processName, HANDLE &hProcessName)
{
    HANDLE hProcessSnap;
    PROCESSENTRY32W pe32;
    DWORD result = NULL;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hProcessSnap) return(FALSE);

    pe32.dwSize = sizeof(PROCESSENTRY32W); // <----- IMPORTANT

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if (!Process32FirstW(hProcessSnap, &pe32))
    {
		CloseHandle(hProcessSnap);          // clean the snapshot object
        DisplayError (hwnd, L"Failed to gather information on system processes", 1, 0);
        return(NULL);
    }

    do
    {
		

		
        if (0 == wcscmp(processName, pe32.szExeFile))
        {
            result = pe32.th32ProcessID;
			hProcessName = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
			if(!hProcessName)
				{
					ErrorExit (L"Cannot open this process", 0);
					CloseHandle(hProcessSnap);
					return(NULL);
				}
            break;
        }

    } while (Process32NextW(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

    return result;
}

NTDLLptr DynamicLoader (bool progInit, wchar_t * fileObjVar)
{
	

	hdlNtCreateFile = LoadLibraryW(L"NtDll.dll");
	if (hdlNtCreateFile) 
	{
	foundNTDLL = (NTDLLptr) GetProcAddress ((HMODULE) hdlNtCreateFile, createFnString);
	if (foundNTDLL)
		{
			if (progInit)
			{
			memset(&ioStatus, 0, sizeof(ioStatus));
			memset(&fileObject, 0, sizeof(fileObject));
			fileObject.Length = sizeof(fileObject);
			fileObject.Attributes = OBJ_CASE_INSENSITIVE;
			//RtlNtStatusToDosError = (PFN_RtlNtStatusToDosError) GetProcAddress((HMODULE)hdlNtCreateFile, NtStatusToDosErrorString);
			PFN_RtlNtStatusToDosError RtlNtStatusToDosError;
			if( !(RtlNtStatusToDosError = (PFN_RtlNtStatusToDosError) GetProcAddress( (HMODULE)hdlNtCreateFile, NtStatusToDosErrorString )) ) foundNTDLL = NULL;
			}
			else
			{
			wchar_t rtlSourceStr[maxPathFolder];
			int retVal = false;
			retVal = wcsncpy_s (rtlSourceStr, _countof(rtlSourceStr),fileObjVar, _TRUNCATE);
			if (retVal == STRUNCATE || retVal == 0)
			{
			fn.Buffer = rtlSourceStr;
			fn.Length = _countof (rtlSourceStr);
			fn.MaximumLength = sizeof (rtlSourceStr);

			//init Unicode string
			PFN_RtlInitUnicodeString RtlInitUnicodeString;
			if (RtlInitUnicodeString = (PFN_RtlInitUnicodeString) GetProcAddress( (HMODULE)hdlNtCreateFile, initUnicodeFnString ))
			{
			RtlInitUnicodeString(&fn, fileObjVar);
			InitializeObjectAttributes (&fileObject, &fn, OBJ_CASE_INSENSITIVE | OBJ_OPENIF, NULL, NULL);
			}
			else
			{
			FreeLibrary ((HMODULE) hdlNtCreateFile);
			foundNTDLL = NULL;
			fileObject.ObjectName = NULL;
			}
			}
			else
			{
			FreeLibrary ((HMODULE) hdlNtCreateFile);
			foundNTDLL = NULL;
			fileObject.ObjectName = NULL;
			}

			}
		}
	else
		{
		FreeLibrary ((HMODULE) hdlNtCreateFile);
		}
	}
return foundNTDLL;
}

bool CloseNTDLLObjs (BOOL atWMClose)
{
	bool returnClose = true;
if (atWMClose)
{
	memset(&ioStatus, 0, sizeof(ioStatus));
	memset(&fileObject, 0, sizeof(fileObject));
}
else
{
	if (hdlNTOut) 
	{
		if (CloseHandle (hdlNTOut) == (ERROR_INVALID_HANDLE)) returnClose = false;
		fileObject.Length = sizeof(fileObject);
		fileObject.Attributes = OBJ_CASE_INSENSITIVE;
		fileObject.ObjectName = NULL;
	}
}
	if (hdlNtCreateFile)
	{
		if (!FreeLibrary ((HMODULE)hdlNtCreateFile)) returnClose = false; 
	}
	ntStatus = NULL;

	return returnClose;
}

bool ProcessFolderRepository(HWND hwnd, bool falseReadtrueWrite, bool appendMode)
{
	DWORD Status;
	int result, jLim = 0;
	wint_t ch = 0, chOld = 0;
	FILE *stream = nullptr;
	bool frReturn = true;
	wchar_t *tempDestOld = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
	wchar_t *frName= (wchar_t *)calloc(pathLength, sizeof(wchar_t));

	if (!ExpandEnvironmentStringsW (L"%SystemRoot%", frName, pathLength)) ErrorExit (L"ExpandEnvironmentStringsW failed for some reason.",0);
	wcscat_s(frName, pathLength, L"\\Temp\\BiggerDirectories.txt");
	stream = _wfopen(frName, L"r+b");
	//If the file already exists and is opened for reading or appending, the Byte Order Mark (BOM), if it present in the file, determines the encoding.
	if (!stream) //returns NULL Pointer
	{


	if (DisplayError (hwnd, L"_wfopen returns NULL: possible first time run? Click yes to create new file, no to abort", 0, 1))
		{

			stream = _wfopen(frName, L"w+b");
			if (stream == nullptr) 
			{
				ErrorExit (L"Problems with opening input File.", 0);
				frReturn = false;
				goto WEOFFOUND;

			}
			_setmode(_fileno(stdout), _O_U16TEXT);
			//write BOM for byte-order endianness (storage of most/least significant bytes) and denote Unicode steream
				if(fputwc(BOM, stream) == EOF)
				//if (fwrite("\xFEFF", 2, 2, stream) < 0)
		
				{
					ErrorExit (L"fwprintf: Problems with writing to input File.", 0);
					frReturn = false;
					goto WEOFFOUND;
				}
		
		}
		else
		{
		free (frName);
		return false; //user won't read/write on empty file
		}


	}
	else //file exists
	{
		if (fclose(stream)) //cppcheck recommends this
		{
			ErrorExit(L"Stream was not closed properly: Recommend restart.", 0);
			frReturn = false;
		}

		if (appendMode)
		{
		stream = _wfopen(frName, L"a+b");
		//When you switch from writing to reading, you must use an intervening call to either fflush or a file positioning function.
		if (!stream) //returns NULL Pointer
		{
		ErrorExit (L"Problems with input File: Cannot append.", 0);
		frReturn = false;
		goto WEOFFOUND;
		}

		}
		else //load F when deleting
		{
		//BOM must be rewritten as it is wiped
		(falseReadtrueWrite)? stream = _wfopen(frName, L"w+b"): stream = _wfopen(frName, L"rb");
			_setmode(_fileno(stdout), _O_U16TEXT);
		//write BOM for byte-order endianness (storage of most/least significant bytes) and denote Unicode steream
			if(fputwc(BOM, stream) == EOF)
			//if (fwrite("\xFEFF", 2, 2, stream) < 0)
		
			{
				ErrorExit (L"fwprintf: Problems with writing to input File.", 0);
				frReturn = false;
				goto WEOFFOUND;
			}
		}
	

	}


	if (falseReadtrueWrite) //write or append to file
	{


		
		//copy to whole string first: no sorting for write: create: append, Remove: write


		for (i = (createFail)? branchTotalCumOld: 0; (i <= ((createFail)? branchTotalCum - 1: branchTotal)); i++) //For deletion write the strings NOT deleted, creation the strings that succeeded
		{
			
			(appendMode)? jLim = trackFTA[i][0] + trackFTA[i][1] - 1: jLim = trackFTA[i][0] - 1;
			
			
			for (j = 0; (folderTreeArray[i][0][0] != L'\0') && (j <= jLim) ; j++)
			{
				k = 0;

				if (folderTreeArray[i][j][0] != L'\0')
					{
						do
						{	
    						ch = folderTreeArray[i][j][k];
							if (ch == L'\0')
							{	
								fputwc (separatorFTA, stream);
							}

							fputwc (ch, stream);
							
							k += 1;
						}
						while (ch != L'\0');

						if (j == jLim) fputwc (eolFTA, stream);
					}


 				}

		}

	}



	else //read from file
  	{
	verifyFail = 0;
	ch = 1;

	result = fseek(stream, 0L, SEEK_SET);  /* moves the pointer to the beginning of the file */
	//rewind(stream); //does the same?
	if (result)
	{
	ErrorExit (L"fseek: Could not rewind!", 0);
	frReturn = false;
	goto WEOFFOUND;
	}

	//Read BOM
	ch = fgetwc(stream);
	
	if(ch != BOM)
		
	{
		DisplayError(hwnd, L"fgetwc: input file does not have BOM", 0, 0);
		frReturn = false;
		goto WEOFFOUND;
	}

(!folderTreeArray[0][0][0])? i = 0 : (appendMode)? i = branchTotal + 1: i = 0;



		do
		{
		//we are reading so last null condition mandatory
		
			if (ch == eolFTA) ch = 1;	//ugly
			for (j = 0; (j <= treeLevelLimit) && (ch != eolFTA); j++)
			{
			if (chOld == separatorFTA) chOld = 1;
			//populate folderTreeArray- using getline method (or "template function") might be more efficent
			for (k = 0; ((k  < (maxPathFolder - 1)) && (chOld != separatorFTA) && (ch != WEOF) ); k++) //screwy logic?

			{


			if ((ch = fgetwc(stream)) == separatorFTA) 
			{
				chOld = ch;
				ch = fgetwc(stream);
				
			}
			else
			{
				chOld = ch;
				if (ch == WEOF) goto WEOFFOUND;
			}
						
			if (ch == eolFTA)
			{
				//must sanitise
				folderTreeArray[i][j][k] = L'\0';
				break;
			}
			else
			{
				folderTreeArray[i][j][k] = (wchar_t)ch;
			}
			}

			trackFTA [i][0] = j; //track the nesting level for validation
			if (ch == eolFTA) break;
			if (j == 0)
				{
					wcscpy_s(tempDestOld, pathLength, driveIDBaseWNT);
				}
			else
				{
					wcscpy_s(tempDestOld, pathLength, pathsToSave[i]);
					wcscat_s(pathsToSave[i], pathLength, &separatorFTA); //tacking them back on: what a waste doing it this way
				}
			wcscat_s(pathsToSave[i], pathLength, folderTreeArray[i][j]);



			if (foundNTDLL && !appendMode)   //verify entire FR before create, delete
				{
					wcscpy_s(tempDest, pathLength, driveIDBaseWNT);
					wcscat_s(tempDest, pathLength, pathsToSave[i]);
					if (DynamicLoader (false, tempDest))
					{
						//Do NOT query file with FILE_OPEN_REPARSE_POINT!
						ntStatus = foundNTDLL (&hdlNTOut, FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES, &fileObject, &ioStatus, 0, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN, FILE_DIRECTORY_FILE, nullptr, 0);

						PFN_RtlNtStatusToDosError RtlNtStatusToDosError;
						if( !(RtlNtStatusToDosError = (PFN_RtlNtStatusToDosError) GetProcAddress( (HMODULE)hdlNtCreateFile, NtStatusToDosErrorString )) ) 
						{
							ErrorExit (L"RtlNtStatusToDosError: Problem!", 0);
							frReturn = false;
							goto WEOFFOUND;
						}
						Status = RtlNtStatusToDosError (ntStatus);

						switch ((DWORD)(ntStatus) >> 30)
							{
							case 0: //NT_SUCCESS
								{

								}
							break;
							case 1: //NT_INFORMATION
								{
								}
							break;
							case 2: //NT_WARNING 
								{
								}
							break;
							case 3://NT_ERROR
								{
									verifyFail = static_cast <int> (Status);
								}
							break;
							default:
							break;
							}
						}
						else
						{
							ErrorExit (L"DynamicLoader failed: Cannot verify. ", 1);
							frReturn = false;
							goto WEOFFOUND;

						}
				if (!CloseNTDLLObjs(false)) DisplayError (hwnd, L"NtCreateFile: Objects failed to close", errCode, 0);

				}//FoundNTdll


			jLim = j; //for verify

 			}

		if (verifyFail)
		{

		//Status can be invalid parm 0X00000057 ERROR_INVALID_PARAMETER from previous call
		// Also can be 0x00000087 ERROR_IS_SUBSTED: 

		if (verifyFail == 57 || verifyFail == 87)
		{
		if (GetFileAttributesW(tempDest) == INVALID_FILE_ATTRIBUTES) //The status_wait_one is always reached after creation
			{
				_snwprintf_s(hrtext, _countof(hrtext), _TRUNCATE, L"Cannot verify a directory entry in the FR (Are you running XP?): If the following directory is to be deleted, a reboot may be required to clear: \n\n %s", tempDest);
			}
			else
			{
				_snwprintf_s(hrtext, _countof(hrtext), _TRUNCATE, L"Cannot verify the directory entry in the FR: Try restarting Bigger Directories to delete the following directory: \n\n %s", tempDest);
			}
			for (k = 0; ((k  < (pathLength)) && (ch!= eolFTA)); k++)
			{
				ch = fgetwc(stream);
				chOld = ch;
			}
			ErrorExit (hrtext, 1);
		}
		else
		{
			//GetFileAttributesW is good to go else it's FltCancellableWaitForSingleObject: The status_wait_one on Get_Last_Error is the error before this (dir creation?)- nothing to do with this- it's related to the mutex
		if (GetFileAttributesW(tempDest) == INVALID_FILE_ATTRIBUTES)
		{
			_snwprintf_s(hrtext, _countof(hrtext), _TRUNCATE, L"Cannot verify a directory entry in the FR: If it exists, its attributes cannot be read by BD on this thread- else the FR entry is corrupt & continuing will ensure its removal: \n\n %s", tempDest);
			if (DisplayError (hwnd, hrtext, errCode, 1))		
			{

				if (jLim > 0) //trackFTA [i][0] is the same as jLim
				{
				folderTreeArray[i][jLim][0] = L'\0';
				//Check if lower branches of the tree are valid, but NOT anything lower than trackFTA [i][1] + 1
				for (j = 0; (j <= treeLevelLimit); j++) pathsToSave[i][j] = L'\0';

				wcscpy_s(tempDestOld, pathLength, driveIDBaseWNT);
				
					if (CheckAttribs (0, *tempDestOld)) 
					{
						for (j = 1; (j < jLim); j++)
						{
							if (!CheckAttribs (j, *tempDestOld)) break;
						}
					}

					else
					{
						//Zero entire branch & reset current i
						for (j = 1; (j <= treeLevelLimit); j++)
						{
						folderTreeArray[i][j][0] = L'\0';
						pathsToSave[i][j] = L'\0';
						}
						i -= 1;
					}


				}
				else
				{
				//reset current i
				for (j = 1; (j <= treeLevelLimit); j++)
				{
				folderTreeArray[i][j][0] = L'\0';
				pathsToSave[i][j] = L'\0';
				}

				i -= 1;
				}

				verifyFail = 0;
				

			}
			else
			{
				for (k = 0; ((k  < (pathLength)) && (ch!= eolFTA)); k++)
				{
					ch = fgetwc(stream);
					chOld = ch;
				}
				break;
			}
		}
		else
		{
		//wind forward to EOL
			for (k = 0; ((k  < (pathLength)) && (ch!= eolFTA)); k++)
			{
				ch = fgetwc(stream);
				chOld = ch;
			}
			_snwprintf_s(hrtext, _countof(hrtext), _TRUNCATE, L"Cannot verify the directory entry in the FR: Try restarting Bigger Directories: \n\n %s ", tempDest);
			ErrorExit (hrtext, 1);

		break;

		}
		}

		}
		branchTotalSaveFile = i;
		i += 1;

		} while ((i < branchLimit) && (ch != WEOF));

	} 


	WEOFFOUND:
	free (tempDestOld);

	if (!falseReadtrueWrite && !appendMode) //cleanup
	{
	hdlNTOut = 0;
		if (foundNTDLL)
		{
			if (!CloseNTDLLObjs(false)) DisplayError (hwnd, L"NtCreateFile: Objects failed to close", errCode, 0);
		}
	}
	// Close stream if it is not NULL 

	if (fclose (stream))
	{
	ErrorExit (L"Stream was not closed properly: Recommend restart.", 0);
	frReturn = false;
	}
	free (frName);
	return frReturn;

	}
bool CheckAttribs(int jVar, wchar_t &testPath)
{
	wcscpy_s(tempDest, pathLength, pathsToSave[i]);
	wcscat_s(&testPath, pathLength, folderTreeArray[i][jVar]);

	if (GetFileAttributesW(&testPath) == INVALID_FILE_ATTRIBUTES)
	{
		folderTreeArray[i][jVar][0] = L'\0';
		wcscpy_s(pathsToSave[i], pathLength, tempDest);
		trackFTA[i][0] = jVar - 1;
		return false;
	}
	if (jVar > 0) wcscat_s(pathsToSave[i], pathLength, &separatorFTA);
	wcscat_s(pathsToSave[i], pathLength, folderTreeArray[i][jVar]);
	wcscat_s(&testPath, pathLength, &separatorFTA);
	return true;
}

void FRDeleteInit (HWND hwnd, HWND hList)
{
bool cmdlineParmtooLong = false;
tempDest = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
findPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t)); // only required for the old RecurseRemovePath
currPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
currPath = (char *)calloc(maxPathFolder, sizeof(char));
pathToDeleteW = (wchar_t *)calloc(pathLength, sizeof(wchar_t));


if ((tempDest == nullptr) || (findPathW == nullptr) || (currPathW == nullptr) || (pathToDeleteW == nullptr))
	{
		DisplayError (hwnd, L"Could not allocate required memory", errCode, 0);
		goto RemoveKleenup;
	}


if (errCode != -100) errCode = -4;

if (pCmdLineActive)
{
	wchar_t * rootDirPtr = wcschr (rootDir, '\\'); //strip leading // if exists
	if (rootDirPtr)
	{
		
		for (i = 0; i < (int)(rootDirPtr - rootDir); i++) rootDirPtr[i] = rootDir[i];
		rootDirPtr[i] = L'\0';
		wcscpy_s(rootDir, pathLength, rootDirPtr);
	}
	pCmdLineActive = false;

	currPathW[0] = L'\0';
	wcscpy_s(pathToDeleteW, pathLength, rootDir);
	wcscpy_s(currPathW, maxPathFolder, rootDir);
	if (sizeof(pathToDeleteW) > maxPathFolder) cmdlineParmtooLong = true;
	findPathW[0] = L'\0';
}

else
{
	memset(dacfoldersWtmp, L'\0', sizeof(dacfoldersWtmp)); //sizeof(dacfoldersW) ok
	//SHFileOperationW(_Inout_ LPSHFILEOPSTRUCT lpFileOp);

		for (i = 1; i < rootFolderCS; i++)
		{
			mbstowcs(dacfoldersWtmp[i], dacfolders[i], maxPathFolder);
			//cumPath = dacfoldersW[i] + 4; // ////?// prefix away: 4 cuts out the //?/
			if (wcscmp(dacfoldersW[index - 1 - rootFolderCW] + 4, dacfoldersWtmp[i]) == 0)
			{
			if (!DisplayError (hwnd, L"This Directory has an \"ANSII\" equivalent. Remove won't work if it contains files. Click Yes to continue", errCode, 1))
				{
					free(pathToDeleteW);
					goto RemoveKleenup;
				}
			}
		}

	wcscpy_s(currPathW, maxPathFolder, dacfoldersW[index - 1 - rootFolderCW]);
	wchar_t * currPathWtmp;
	currPathWtmp = currPathW + 7;
	wcscpy_s(rootDir, pathLength, currPathWtmp);
	findPathW[0] = L'\0';
}


	if (foundNTDLL)
	{
		if (errCode > -100) 
		{
			if (ProcessFolderRepository(hwnd, false, false)) //Reads and verifies entire FR
			{
				if (verifyFail)
				{
					free (tempDest);
					free(pathToDeleteW);
					goto RemoveKleenup;
				}
			}
			else
			{
			if (!DisplayError (hwnd, L"No FR file! Cannot tell whether directory was created by this program. Click Yes for alternate delete", 0, 1))
				{
					free (tempDest);
					free(pathToDeleteW);
					goto RemoveKleenup;
				}
			else
				{
					free (tempDest);
					free(pathToDeleteW);
					goto OldDelete;
				}
			}
		}
		else
		{
		errCode = -4;
		}
		//zero all trackFTA for anything that isn't rootDir
		//reorg DB so rootdir is first.
		free (tempDest);
		j = branchTotalSaveFile;
		branchTotal = j;
		int tmp;
		memset(reorgTmpWFS, L'\0', sizeof(reorgTmpWFS));
		memset(reorgTmpW, L'\0', sizeof(reorgTmpW));


		for (i = branchTotal; (i >= 0); i--) //place paths to delete at end of FR
		{

			if (!wcscmp (rootDir, folderTreeArray[i][0])) //0 if perfect match which applies multiple branches from the same rootDir
			{
				if (i < j)
				{
				wcscpy_s(reorgTmpW, pathLength, pathsToSave[j]);
				wcscpy_s(pathsToSave[j], pathLength, pathsToSave[i]);
				wcscpy_s(pathsToSave[i], pathLength, reorgTmpW);

				for (k = 0; (k < trackFTA [j][0]); k++)
				{
				wcscpy_s(reorgTmpWFS[k], maxPathFolder, folderTreeArray[j][k]);
				}
				for (k = 0; (k < trackFTA [i][0]); k++)
				{
				wcscpy_s(folderTreeArray[j][k], maxPathFolder, folderTreeArray[i][k]);
				}
				folderTreeArray[j][trackFTA [i][0]][0] = L'\0';

				for (k = 0; (k < trackFTA [j][0]); k++)
				{
				wcscpy_s(folderTreeArray[i][k], maxPathFolder, reorgTmpWFS[k]);
				}
				folderTreeArray[i][trackFTA [j][0]][0] = L'\0';

				tmp = trackFTA [j][0];
				trackFTA [j][0] = trackFTA [i][0];
				trackFTA [i][0] = tmp;
				}
				j -= 1;
			}

		}

		if (branchTotal)
		{
			if (branchTotal == j)
			{
				if (DisplayError (hwnd, L"The selected folder is not found in the File Repo. Click Yes for alternate delete", 0, 1))
				{
					free(pathToDeleteW);
					goto OldDelete;
				}
				else
				{
					free(pathToDeleteW);
					goto RemoveKleenup;

				}
			}
			branchTotalCum = j + 1; //j is branchTotalSaveFile
		}
		else
		{
			branchTotalCum = 0;
		}


			do
			{
				if (verifyFail == 145) // When electing to continue on non-empty directory. This will leave an invalid entry in the FR.
				{
					OldDeleteInit (hwnd);
					verifyFail = 0;
				}
			} while (FRDelete (hwnd));
			//Write remaining FR
			free(pathToDeleteW);
			if (!errCode) //errCode is still -4 if no match!
			{
			if (!pCmdLineActive) ((ProcessFolderRepository(hwnd, true, false))? errCode = 1: errCode = 0);
			//this can bug out if the user edits or deletes the FR in the intervening milliseconds when called from InitProc.				
			}
			goto RemoveKleenup;

	}
	else
	{
		if (!DisplayError (hwnd, L"NTDLL not found on this machine. Click Yes for alternate delete", 0, 1))
			{
				free(pathToDeleteW);
				goto RemoveKleenup;
			}
	}


OldDelete:


if (cmdlineParmtooLong)
	{
		errCode = 0;
		DisplayError (hwnd, L"Oops, command line too long! Delete won't work. Quit and rerun", 0, 0);
		goto RemoveKleenup;
	}
else
	OldDeleteInit (hwnd);



					
	RemoveKleenup:
	if (findPathW) free (findPathW);
	rootDir[0] = L'\0';
		if (pCmdLineActive)
		{
			//ReleaseMutex (hMutex); //problematic??
			if (currPathW) free(currPathW);
			if (currPath) free(currPath);
			_CrtDumpMemoryLeaks();
			EndDialog(hwnd, 1);
		}

		else 
		{
			if (errCode != 0) //"succeeded"
			{
			errCode = 0; //flag okay now
			listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);
			InitProc(hwnd);
			}
			else 
			{
			errCode = -100;
			}
			if (currPathW) free(currPathW);
			if (currPath) free(currPath);
		}
return;
}

bool FRDelete (HWND hwnd)
{	

if (branchTotal == branchTotalCum - 1) //branchTotal is decremented here, not branchTotalCum
	{
		branchTotal = branchTotalCum; // required for FR Write
		return false;
	}



	for (i = branchTotalCum; (i <= branchTotal); i++)
	{
		trackFTA [i][1] = trackFTA [i][0];
	}


	for (i = branchTotalCum + 1; (i <= branchTotal); i++)
		{ 
		int tmp = trackFTA [i][1]; //insertion sort in descending order: doesn't apply if single stem (rootDir is unique)
			for (j = i; ((j > branchTotalCum) && (tmp < trackFTA [j-1][1])); j--)
			{
				trackFTA [j][1] = trackFTA [j-1][1];
			
			}
		trackFTA [j][1] = tmp;
	}



	for (i = branchTotal; (i >= branchTotalCum); i--)
	{

		//delete the bottom folder of pathsToSave[i] whose first strings correspond to rootDir in the order of trackFTA [i][1] 
			{
				for (j = branchTotal; (j >= branchTotalCum); j--)
				{
					if (trackFTA [i][1] == trackFTA [j][0]) return (FRDelsub (hwnd));
				}
			}
	}

return true; //shouldn't get here though
}
bool FRDelsub (HWND hwnd)
{

	for (k = (trackFTA [i][1]); (k >= ((i > branchTotalCum)? trackFTA [i - 1][1]: 0)); k--)
	{
	//do not iterate below trackFTA [i + 1][1]


	wcscpy_s(pathToDeleteW, pathLength, IsWindowsVistaOrGreater()? driveIDBaseWNT: driveIDBaseWNT);
	wcscat_s(pathToDeleteW, pathLength, pathsToSave[j]);


	if (RemoveDirectoryW (pathToDeleteW))
			{
				//clear everything -we have only removed the last dir from pathsToSave so remove last dir from folderTreeArray

				folderTreeArray[j][k-1][0] = L'\0';

				trackFTA [i][1] -=1;
				trackFTA [j][0] -=1;


				if (trackFTA [i][1] == 0)
				{
				errCode = 0;
				FRReorg (j, branchTotal);
				return true;
				}
				else
				{
					if (trackFTA [i][1] < 0)
					{
					DisplayError (hwnd, L"Something went wrong with the deletion.", 0, 0);
					return false;
					}
					//rebuild pathsToSave
					wcscpy_s(pathsToSave[j], pathLength, folderTreeArray[j][0]);
					for (int l = 1; (l < k - 1); l++) //extra loop adds the terminator
					{
						wcscat_s(pathsToSave[j], pathLength, &separatorFTA);
						wcscat_s(pathsToSave[j], pathLength, folderTreeArray[j][l]);
					}

				}
			
			}
		else
			{
				
				if (((int)GetLastError() == 32) ) //"used by another process" error
				{
					if (secondTryDelete)
					{
					DisplayError (hwnd, L"Delete failed the second time. Try running 7-zip and shift-del", 0, 0);
					secondTryDelete = false;
					}
					else
					{
					DisplayError (hwnd, L"Oops, 'Used by another process': Something went wrong. Restarting to attempt deletion", 0, 0);
					rootDir[0] = L'\0';
					pCmdLineActive = true;
					wcscpy_s(pathToDeleteW, pathLength, L" "); //http://forums.codeguru.com/showthread.php?213443-How-to-pass-command-line-arguments-when-using-CreateProcess
					wcscat_s(pathToDeleteW, pathLength, pathsToSave[j]);

					((ProcessFolderRepository(hwnd, true, false))? errCode = 0: errCode = 1);
					Kleenup (hwnd);
					}
					return false;

				}
				else
				{
					errCode = (int)GetLastError();
					if ((errCode == 2) || (errCode == 3)) //cannot find file or path specified
						{
						//The entry in pathsToSave must have a duplicate elsewhere which has already been removed: nuke the current one.
						pathsToSave[j][0] = L'\0';
						folderTreeArray[j][0][0] = L'\0';
						trackFTA [i][1] = 0;
						trackFTA [j][0] = 0;
						errCode = 0;
						FRReorg (j, branchTotal);
						return true;

						}
					else
					{
						if (errCode == 145)
						{
							if (DisplayError (hwnd, L"Delete error: folder is not empty. This can occur when an FR entry is modified ny another program or the folder contains new folders or files. Click Yes to try Alternate deletion: ", 0, 1))
							{
								verifyFail = 145; //The messagebox call is actually overwriting the errCode variable so try verifyFail?
								return true;
							}
							else
							{
								errCode = 0;
								return false;
							}
						}
						else
						{
						ErrorExit (L"RemoveDirectoryW: Cannot remove Folder. ", 0);
						errCode = 0;
						return false;
						}
					}

				}

			}

	}
//k loop

return true;
}
void FRReorg (int jVar, int &brTotal)
{
if (jVar != brTotal)
	{
//Move everything down to fill slot
	for (k = jVar + 1; (k <= brTotal); k++)
	{
		for (int l = 0; (l < trackFTA [k][0]); l++)
			{
			wcscpy_s(folderTreeArray[k-1][l], maxPathFolder, folderTreeArray[k][l]); 
			}	

		trackFTA [k-1][0] = trackFTA [k][0];
		wcscpy_s(pathsToSave[k-1], pathLength, pathsToSave[k]); 
	}
	}
//folderTreeArray[branchTotal][0] taken care of
pathsToSave [brTotal][0] = L'\0';
brTotal -=1;
}
void doFilesFolders(HWND hwnd)
{
		DragAcceptFiles(hwnd,TRUE);
		SendDlgItemMessageW(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
		folderIndex = PopulateListBox (hwnd, true, true);
		SendDlgItemMessageW(hwnd, IDC_LIST, LB_DELETESTRING, 0, 0); //remove the '.'
		folderIndex -=1;
		//Account for folder/file separator
		if (PopulateListBox (hwnd, true, false)) sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_INSERTSTRING, WPARAM(folderIndex), (LPARAM)(L"\0"));
		//disable buttons
		SetDlgItemTextW(hwnd,IDC_STATIC_ZERO, L"Dir:");
		SetDlgItemTextW(hwnd,IDC_STATIC_ONE, L"\0");
		SetDlgItemInt(hwnd, IDC_NUMBER, 0, FALSE);
		SetDlgItemTextW(hwnd, IDC_TEXT, dblclkPath[dblclkLevel]);
		SetDlgItemInt(hwnd, IDC_SHOWCOUNT, 0, FALSE);
		EnableWindow(GetDlgItem(hwnd, IDC_ADD), false);
		EnableWindow(GetDlgItem(hwnd, IDC_UP), false);
		EnableWindow(GetDlgItem(hwnd, IDC_DOWN), false);
		EnableWindow(GetDlgItem(hwnd, IDC_CREATE), false);
		EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
}
void OldDeleteInit(HWND hwnd)
{

	treeLevel = 0;
	trackFTA [0][0] = 0; //Initial conditions before search on path
	trackFTA [0][1] = 1;

	for (i = 1; i <= branchLimit; i++)
	{
	trackFTA [i][0] = 0;
	trackFTA [i][1] = 0;
	}
	
	wcscat_s(currPathW, maxPathFolder, &separatorFTA);
	wcscpy_s(folderTreeArray[0][0], maxPathFolder, currPathW);

	if (!SetCurrentDirectoryW (driveIDBaseW))
	{
	ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
	return;
	}
	if (RecurseRemovePath())
		{
			errCode = 0;
			DisplayError (hwnd, L"Remove failed: this has been verified to occur if the folder is openb in Explorer.", 0, 0);
		}
	else
		{
			errCode = 1;
		}
}
int RecurseRemovePath()

	 //first element of trackFTA is LAST_VISIT, second is number of folders found
{
	

	if (trackFTA [treeLevel][1] > 0) //Have we done a search on this level yet? if yes then here
	{

			//move along the branch

			//wcscpy_s(currPathW, maxPathFolder, folderTreeArray[treeLevel][trackFTA[treeLevel][0]-1]);

			//FINISH coding this
			if (trackFTA [treeLevel][0] == trackFTA [treeLevel][1]) //end of the branch?
			{
			//Must go down but we have already found the files of directory below
			trackFTA [treeLevel][1] = 0;  //important
			treeLevel -=1;
			wcscpy_s(currPathW, maxPathFolder, folderTreeArray[trackFTA [treeLevel][0]-1][treeLevel]);


					if (treeLevel == 0) //Last folder to do!! 
					{
						if (dblclkLevel > 1)

						{
							if (!SetCurrentDirectoryW (dblclkString)) //objects to L".."
							{
								ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
								return 1;
							}
							if (RemoveDirectoryW (currPathW))
							{
								return 0;
							}
							else
							{
								ErrorExit (L"RemoveDirectoryW: Cannot remove Folder. It may contain files.", 0);

								return 1; //Need more than this
							}


						}
						
						else
						{

							if (!SetCurrentDirectoryW (driveIDBaseW)) //objects to L".."
							{
							ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
							return 1;
							}
							wchar_t * currPathWtmp;
							currPathWtmp = currPathW + 4;

							if (RemoveDirectoryW (currPathWtmp))
							{
								return 0;
							}
							else
							{
								ErrorExit (L"RemoveDirectoryW: Cannot remove Folder. It may contain files.", 0);

								return 1; //Need more than this
							}
						}

					}

					else
					{
						if (!SetCurrentDirectoryW (L".."))
						{
							ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
							return 1;
						}

						if (RemoveDirectoryW (currPathW))
						{

						if (RecurseRemovePath())
							{
							return 1;
							}
							else
							{
							return 0;
							}
						}
						else
							{
								ErrorExit (L"RemoveDirectoryW: Cannot remove Folder. It may contain files.", 0);
								return 1; //Need more than this
							}


					}




			}

			else
			{
				//folderTreeArray[treeLevel][j+1]
				if (trackFTA[treeLevel][0] <= 999)
				{
				trackFTA[treeLevel][0] +=1;

				// set inits for this branch
				wcscpy_s(findPathW, maxPathFolder, folderTreeArray[trackFTA[treeLevel][0]-1][treeLevel]);
				//
				if (!SetCurrentDirectoryW (findPathW))
				{
					ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
					return 1;
				}

				treeLevel +=1; // up next tree
					if (RecurseRemovePath())
						{
						return 1;
						}
						else
						{
						return 0;
						}

				}
				else
				{
				trackFTA[treeLevel][0] = 0;
				trackFTA[treeLevel][1] = 0;
				treeLevel -=1;
				ErrorExit (L"Too many folders in the tree: If folder was created by this program, a warning should have been issued on folder creation.", 0);
				return 1; 
				}
			}

	}

	else //search yet to be done on branch
	{



	//Do find folders in new branch, findPathW already set
			
				
		memset(&dw, 0, sizeof(WIN32_FIND_DATAW));
		//Find first file
		//Get fulqualpath
		if (!GetCurrentDirectoryW (maxPathFolder, findPathW))
			{
				ErrorExit (L"GetCurrentDirectoryW: Zero", 0);
				return 1;
			}
		wcscat_s(findPathW, maxPathFolder, L"\\*");
		ds = FindFirstFileW(findPathW, &dw);
			if (ds == INVALID_HANDLE_VALUE) //redundant as first 2 pickups are "." and ".."
			{
				// No Folders so this must be top level
				FindClose(ds);
				ErrorExit (L"FindFirstFileW: Should never get here. No can do!", 0);
				return 1; //Need more than this
							
			}


						
			BOOL findhandle = TRUE;
			j = 0;
							
			while (ds != INVALID_HANDLE_VALUE && findhandle)
			{

			if ((dw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(dw.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM || dw.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ||
				dw.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE || dw.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN || !wcscmp(dw.cFileName, L".") || !wcscmp(dw.cFileName, L"..")))
										
				//"." is just an alias for "this" directory
				//".." is just an alias for the parent directory.

			{

				wcscpy_s(currPathW, maxPathFolder, dw.cFileName);
				wcscat_s(currPathW, maxPathFolder, &separatorFTA);


				wcscpy_s(folderTreeArray[j][treeLevel], maxPathFolder, (wchar_t *)currPathW);
				j +=1;

			}
				findhandle = FindNextFileW(ds, &dw);
			}

			if (!FindClose(ds)) ErrorExit (L"FindClose: Non zero", 0);
			//wcscpy_s(currPathW, maxPathFolder, folderTreeArray[treeLevel][j-1]);
			trackFTA [treeLevel][0] = 0; //check reset counter if necessary here

			if (j == 0)
				{
				// No Folders so this must be top level

					if (treeLevel == 1) //Last folder to do!! 
					{
						if (dblclkLevel > 1)
						{

							if (!SetCurrentDirectoryW (dblclkString)) //objects to L".."
							{
							ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
							return 1;
							}
							if (RemoveDirectoryW (currPathW))
							{
								return 0;
							}
							else
							{
								ErrorExit (L"RemoveDirectoryW: Cannot remove Folder. It may contain files.", 0);

								return 1; //Need more than this
							}


						}
						
						else
						{
						
							if (!SetCurrentDirectoryW (driveIDBaseW)) //objects to L".."
							{
							ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
							return 1;
							}
							wchar_t * currPathWtmp;

							(wcsstr(currPathW, L"\\\\?\\C:"))? currPathWtmp = currPathW + 4: currPathWtmp = currPathW;
							//GetCurrentDirectoryW(maxPathFolder, findPathW);
							if (RemoveDirectoryW (currPathWtmp))
							{
								return 0;
							}
							else
							{
								ErrorExit (L"RemoveDirectoryW: Cannot remove Folder. It may contain files.", 0);

								return 1; //Need more than this
							}

						}
					}
					else
					{

						if (!SetCurrentDirectoryW (L".."))
						{
						ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
						return 1;
						}
					}

					//GetCurrentDirectoryW(maxPathFolder, findPathW);
					if (RemoveDirectoryW (folderTreeArray[trackFTA[treeLevel-1][0]-1][treeLevel-1]))
					{
						trackFTA [treeLevel][1] = 0;  //important
						treeLevel -=1;
						if (RecurseRemovePath())
							{
							return 1;
							}
							else
							{
							return 0;
							}
					}
					else
					{
						ErrorExit (L"RemoveDirectoryW: Cannot remove Folder. It may contain files.", 0);
						return 1; //Need more than this
					}

				}

		else //Do an iteration on this new branch
				{
					//if (!GetCurrentDirectoryW(maxPathFolder, findPathW)) ErrorExit("SetCurrentDirectoryW: Non zero", 0);
					trackFTA [treeLevel][1] = j;

					if (RecurseRemovePath())
					{
					return 1;
					}
					else
					{
					return 0;
					}
				}
	} //trackFTA[treeLevel][0] = 0
}
void ShellError (HWND aboutHwnd, HINSTANCE nError)
{
	if ((LONG_PTR)nError > 32) return; //no problem
	wchar_t* str = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
	switch ((LONG_PTR)nError)  //reinterpret_cast <int> (
	{
	case 0:							wcscpy_s(str, maxPathFolder, L"The operating system is out\nof memory or resources"); break;
	case ERROR_FILE_NOT_FOUND:		wcscpy_s(str, maxPathFolder, L"The specified path was not found"); break;
	case ERROR_PATH_NOT_FOUND:		wcscpy_s(str, maxPathFolder, L"The specified file was not found"); break;
	case ERROR_BAD_FORMAT:			wcscpy_s(str, maxPathFolder, L"The .EXE file is invalid\n(non-Win32 .EXE or error in .EXE image)"); break;
	case SE_ERR_ACCESSDENIED:		wcscpy_s(str, maxPathFolder, L"The operating system denied\naccess to the specified file"); break;
	case SE_ERR_ASSOCINCOMPLETE:	wcscpy_s(str, maxPathFolder, L"The filename association is\nincomplete or invalid"); break;
	case SE_ERR_DDEBUSY:			wcscpy_s(str, maxPathFolder, L"The DDE transaction could not\nbe completed because other DDE transactions\nwere being processed"); break;
	case SE_ERR_DDEFAIL:			wcscpy_s(str, maxPathFolder, L"The DDE transaction failed"); break;
	case SE_ERR_DDETIMEOUT:			wcscpy_s(str, maxPathFolder, L"The DDE transaction could not\nbe completed because the request timed out"); break;
	case SE_ERR_DLLNOTFOUND:		wcscpy_s(str, maxPathFolder, L"The specified dynamic-link library was not found"); break;
	case SE_ERR_NOASSOC:			wcscpy_s(str, maxPathFolder, L"There is no application associated\nwith the given filename extension"); break;
	case SE_ERR_OOM:				wcscpy_s(str, maxPathFolder, L"There was not enough memory to complete the operation"); break;
	case SE_ERR_SHARE:				wcscpy_s(str, maxPathFolder, L"A sharing violation occurred"); break;
	default:						wcscpy_s(str, maxPathFolder, L"Unknown Error occurred"); break;
	}
	swprintf_s(hrtext, _countof(hrtext), L"Unable to open hyperlink:\n\n %s", str);
	free (str);
	DisplayError (aboutHwnd, hrtext, 0, 0);
 }
static void CreateHyperLink(HWND hwndControl)
{
    // Subclass the parent so we can color the controls as we desire.
    HWND hwndParent = GetParent(hwndControl);
    if (NULL != hwndParent)
    {
        WNDPROC pfnOrigProc = (WNDPROC)GetWindowLongW(hwndParent, GWLP_WNDPROC);
        if (pfnOrigProc != _HyperlinkParentProc)
        {
            SetPropW(hwndParent, PROP_ORIGINAL_PROC, reinterpret_cast<HANDLE>((LONG_PTR)(pfnOrigProc)));
            SetWindowLongW(hwndParent, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>((WNDPROC)_HyperlinkParentProc));
        }
    }

    // Make sure the control will send notifications.
    DWORD dwStyle = GetWindowLongW(hwndControl, GWL_STYLE);
    SetWindowLongW(hwndControl, GWL_STYLE, dwStyle | SS_NOTIFY);

    // Subclass the existing control.
    WNDPROC pfnOrigProc = (WNDPROC)GetWindowLongW(hwndControl, GWLP_WNDPROC);
    SetPropW(hwndControl, PROP_ORIGINAL_PROC, reinterpret_cast<HANDLE>((LONG_PTR)(pfnOrigProc)));
    SetWindowLongW(hwndControl, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>((WNDPROC)_HyperlinkProc));

    // Create an updated font by adding an underline.
    HFONT hOrigFont = (HFONT)SendMessageW(hwndControl, WM_GETFONT, 0, 0);
    SetPropW(hwndControl, PROP_ORIGINAL_FONT, reinterpret_cast<HANDLE>((LONG_PTR)(hOrigFont)));

    LOGFONT lf;
    GetObject(hOrigFont, sizeof(lf), &lf);
    lf.lfUnderline = TRUE;

    HFONT hFont = CreateFontIndirect(&lf);
    SetPropW(hwndControl, PROP_UNDERLINE_FONT, reinterpret_cast<HANDLE>((LONG_PTR)(hFont)));

    // Set a flag on the control so we know what color it should be.
    SetPropW(hwndControl, PROP_STATIC_HYPERLINK, (HANDLE)(LONG_PTR)1);
}
static LRESULT CALLBACK _HyperlinkParentProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WNDPROC pfnOrigProc = (WNDPROC)GetPropW(hwnd, PROP_ORIGINAL_PROC);

    switch (message)
    {
    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        HWND hwndCtl = (HWND)lParam;

        BOOL fHyperlink = (NULL != GetPropW(hwndCtl, PROP_STATIC_HYPERLINK));
        if (fHyperlink)
        {
            LRESULT lr = CallWindowProcW(pfnOrigProc, hwnd, message, wParam, lParam);
            SetTextColor(hdc, RGB(0, 0, 192));
            return lr;
        }

        break;
    }
    case WM_DESTROY:
    {
        SetWindowLongW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(pfnOrigProc));
        RemovePropW(hwnd, PROP_ORIGINAL_PROC);
        break;
    }
    }
    return CallWindowProcW(pfnOrigProc, hwnd, message, wParam, lParam);
}

static LRESULT CALLBACK _HyperlinkProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WNDPROC pfnOrigProc = (WNDPROC)GetPropW(hwnd, PROP_ORIGINAL_PROC);

    switch (message)
    {
    case WM_DESTROY:
    {
        SetWindowLongW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(pfnOrigProc));
        RemovePropW(hwnd, PROP_ORIGINAL_PROC);

        HFONT hOrigFont = (HFONT)GetPropW(hwnd, PROP_ORIGINAL_FONT);
        SendMessageW(hwnd, WM_SETFONT, (WPARAM)hOrigFont, 0);
        RemovePropW(hwnd, PROP_ORIGINAL_FONT);

        HFONT hFont = (HFONT)GetPropW(hwnd, PROP_UNDERLINE_FONT);
        DeleteObject(hFont);
        RemovePropW(hwnd, PROP_UNDERLINE_FONT);

        RemovePropW(hwnd, PROP_STATIC_HYPERLINK);

        break;
    }
    case WM_MOUSEMOVE:
    {
        if (GetCapture() != hwnd)
        {
            HFONT hFont = (HFONT)GetPropW(hwnd, PROP_UNDERLINE_FONT);
            SendMessageW(hwnd, WM_SETFONT, (WPARAM)hFont, FALSE);
            InvalidateRect(hwnd, NULL, FALSE);
            SetCapture(hwnd);
        }
        else
        {
            RECT rect;
            GetWindowRect(hwnd, &rect);

            POINT pt = { LOWORD(lParam), HIWORD(lParam) };
            ClientToScreen(hwnd, &pt);

            if (!PtInRect(&rect, pt))
            {
                HFONT hFont = (HFONT)GetPropW(hwnd, PROP_ORIGINAL_FONT);
                SendMessageW(hwnd, WM_SETFONT, (WPARAM)hFont, FALSE);
                InvalidateRect(hwnd, NULL, FALSE);
                ReleaseCapture();
            }
        }
        break;
    }
    case WM_SETCURSOR:
    {
        // Since IDC_HAND is not available on all operating systems,
        // we will load the arrow cursor if IDC_HAND is not present.
        HCURSOR hCursor = LoadCursor(NULL, IDC_HAND);
        if (NULL == hCursor)
            hCursor = LoadCursor(NULL, IDC_ARROW);
        SetCursor(hCursor);
        return TRUE;
    }
    }

    return CallWindowProcW(pfnOrigProc, hwnd, message, wParam, lParam);
}
DWORD dynamicComCtrl(LPCWSTR lpszDllName)
{

    HINSTANCE hinstDll;
    DWORD dwVersion = 0;

    // For security purposes, LoadLibrary should be provided with a fully qualified 
    // path to the DLL. The lpszDllName variable should be tested to ensure that it 
    // is a fully qualified path before it is used. 
    hinstDll = LoadLibraryW(lpszDllName);
	
	if(hinstDll)
	{
	DLLGETVERSIONPROC pDllGetVersion;
	pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");

	// Because some DLLs might not implement this function, you must test for 
	// it explicitly. Depending on the particular DLL, the lack of a DllGetVersion 
	// function can be a useful indicator of the version. 

	if(pDllGetVersion)
	{
	DLLVERSIONINFO2 dvi;
           
	ZeroMemory(&dvi, sizeof(dvi));
	dvi.info1.cbSize = sizeof(dvi);

	HRESULT hr = (*pDllGetVersion)(&dvi.info1);

	if(SUCCEEDED(hr))
	{
	dwVersion = PACKVERSION(dvi.info1.dwMajorVersion, dvi.info1.dwMinorVersion);
	}
	}
	FreeLibrary(hinstDll);
	}



	return dwVersion;
}
BOOL GetAccountSidW(LPWSTR SystemName, PSID *Sid)
{
LPWSTR ReferencedDomain=NULL;
DWORD cbSid=128;    // initial allocation attempt
DWORD cchReferencedDomain=16; // initial allocation size
SID_NAME_USE peUse;
wchar_t infoBuf[maxDWORD];
DWORD  bufCharCount = maxDWORD;
bufCharCount = maxDWORD;


// 
// initial memory allocations
// 
if((*Sid=HeapAlloc(GetProcessHeap(),0,cbSid)) == NULL)
{
	errCode = 1;
	goto CleanHeap;
}
if(!GetUserNameW(infoBuf, &bufCharCount))
{
	errCode = 2;
	goto CleanHeap;
}


if((ReferencedDomain=(LPWSTR)HeapAlloc(GetProcessHeap(),0,cchReferencedDomain * sizeof(wchar_t))) == NULL)
{
	errCode = 3;
	goto CleanHeap;
}

// 
// Obtain the SID of the specified account on the specified system.
// 
while(!LookupAccountNameW(
SystemName,			//local SystemName is NULL
infoBuf,			//account to lookup
*Sid,				// SID of interest
&cbSid,				// size of SID
ReferencedDomain,	// domain account was found on
&cchReferencedDomain,
&peUse
))
{
	if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
	// 
	// reallocate memory
	// 
	if((*Sid=HeapReAlloc(GetProcessHeap(),0,*Sid,cbSid)) == NULL)
	{
		errCode = 4;
		goto CleanHeap;
	}

	if((ReferencedDomain=(LPWSTR)HeapReAlloc(
	GetProcessHeap(),
	0,
	ReferencedDomain,
	cchReferencedDomain * sizeof(wchar_t)
	)) == NULL)
	{
		errCode = 5;
		goto CleanHeap;
	}

	}
	else 
	{
		errCode = 6;
		goto CleanHeap;
	}

}



// 
// Cleanup and indicate failure, if appropriate.
// 
CleanHeap:
HeapFree(GetProcessHeap(), 0, ReferencedDomain);

if(errCode)
{
if(*Sid != NULL)
	{
	HeapFree(GetProcessHeap(), 0, *Sid);
	*Sid = NULL;
	}
}


return errCode;
}
int GetDrives(HWND hwnd)
{
	DWORD cchBuffer;
	wchar_t* driveStrings, *outputString;
	int driveType, driveCt = 0;
	PWSTR driveTypeString;

	// Find out how big a buffer we need
	cchBuffer = GetLogicalDriveStringsW(0, NULL);
	if (cchBuffer)
	{
		driveTypeString = (PWSTR)calloc((cchBuffer + 1), 74);
		driveStrings = (wchar_t *)calloc((cchBuffer + 1), sizeof(wchar_t));
		outputString = (wchar_t *)calloc((maxPathFolder), sizeof(wchar_t));
		if (driveStrings == nullptr || outputString == nullptr)
		{
			DisplayError(hwnd, L"Could not allocate required memory", errCode, 0);
			return 0;
		}

		// Fetch all drive strings    
		GetLogicalDriveStringsW(cchBuffer, driveStrings);

		// Loop until we find the final '\0'
		// driveStrings is a double null terminated list of null terminated strings)
		wchar_t * singleDriveString = driveStrings;
		while (*singleDriveString)
		{
			// Dump drive information
			driveType = GetDriveTypeW(singleDriveString);
			driveInfo[driveCt][0] = singleDriveString[0];
			switch (driveType)
			{
			case DRIVE_UNKNOWN: //known unknowns
				//driveTypeString = L"                         (Unknown)";
				wcscpy_s(driveTypeString, 74, L"                         (Unknown)");
				driveInfo[driveCt][1] = L'U';
				break;
			case DRIVE_NO_ROOT_DIR:
				wcscpy_s(driveTypeString, 74, L"                   (Disk not ready)");
				//driveTypeString = L"                   (Disk not ready)";
				driveInfo[driveCt][1] = L'X';
				break;
			case DRIVE_REMOVABLE:
				wcscpy_s(driveTypeString, 74, L"                        (Removable)");
				//driveTypeString = L"                        (Removable)";
				driveInfo[driveCt][1] = L'M';
				break;
			case DRIVE_FIXED:
				wcscpy_s(driveTypeString, 74, L"                       (Fixed disk)");
				//driveTypeString = L"                       (Fixed disk)";
				driveInfo[driveCt][1] = L'F';
				break;
			case DRIVE_REMOTE:
				wcscpy_s(driveTypeString, 74, L"                          (Network)");
				//driveTypeString = L"                          (Network)";
				driveInfo[driveCt][1] = L'N';
				break;
			case DRIVE_CDROM:
				wcscpy_s(driveTypeString, 74, L"                           (CD/DVD)");
				//driveTypeString = L"                           (CD/DVD)";
				driveInfo[driveCt][1] = L'C';
				break;
			case DRIVE_RAMDISK:
				wcscpy_s(driveTypeString, 74, L"                          (Ramdisk)");
				//driveTypeString = L"                          (Ramdisk)";
				driveInfo[driveCt][1] = L'R';
				break;
			default: //unknown unknowns
				wcscpy_s(driveTypeString, 74, L"                         (Unknown*)");
				//driveTypeString = L"                         (Unknown*)";
				driveInfo[driveCt][1] = L'U';
			}

			wcscpy_s(outputString, maxPathFolder, singleDriveString);
			wcscat_s(outputString, maxPathFolder, driveTypeString);
			sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_ADDSTRING, (WPARAM)(driveCt), (LPARAM)outputString);
			// +1 is to move past the null at the end of the string.
			driveCt += 1;
			singleDriveString += lstrlenW(singleDriveString) + 1;

		}
		if (driveTypeString) free(driveTypeString);
		if (outputString) free(outputString);
		if (driveStrings) free(driveStrings);

		return 1;
	}
	return 0; //No Drives
	}
void ThisInvalidParameterHandler(HWND hwnd, const wchar_t* expression, const wchar_t* function, const wchar_t* file, unsigned int line, uintptr_t pReserved)
{
	//wcscpy_s(str, maxPathFolder, L"Unknown Error occurred"); break;
	swprintf_s(hrtext, _countof(hrtext), L" Invalid parm in function %s. File: %s Line: %u\n", function, file, line);
	DisplayError(hwnd, hrtext, 0, 0);
}

BOOL RevertWOW64RedirectionIfNecessary(PVOID pOldValue)
{
//XP compatibility for these fns
//http://stackoverflow.com/questions/40944793/typedef-with-function-pointer-function-does-not-exist?

	typedef BOOL (WINAPI * fnWow64RevertWow64FsRedirection)(PVOID);

    fnWow64RevertWow64FsRedirection pfn =
        reinterpret_cast<fnWow64RevertWow64FsRedirection>(
           reinterpret_cast<void*>(
           GetProcAddress(GetModuleHandle(L"kernel32"),
                          "Wow64RevertWow64FsRedirection")));

    if (pfn)
    {
        // The function exists, so call it through the pointer we obtained.
        return pfn(pOldValue);
    }
    else
    {
         return FALSE;
	}
}
BOOL DisableWOW64RedirectionIfNecessary(PVOID pOldValue)
{
    typedef BOOL (WINAPI * fnWow64DisableWow64FsRedirection)(PVOID);

    fnWow64DisableWow64FsRedirection pfn =
        reinterpret_cast<fnWow64DisableWow64FsRedirection>(
           reinterpret_cast<void*>(
           GetProcAddress(GetModuleHandle(L"kernel32"),
                          "Wow64DisableWow64FsRedirection")));

    if (pfn)
    {
        return pfn(pOldValue);
    }
    else
    {
         return FALSE;
    }
}
BOOL ChangeWindowMsgFilterEx(HWND hWnd, UINT uMsg)
{
    
	typedef BOOL (WINAPI * fnChangeWindowMessageFilterEx)(HWND, UINT, DWORD, PCHANGEFILTERSTRUCT);
    fnChangeWindowMessageFilterEx pfn =
        reinterpret_cast<fnChangeWindowMessageFilterEx>(
           reinterpret_cast<void*>(
           GetProcAddress(GetModuleHandle(L"user32"),
                          "ChangeWindowMessageFilterEx")));

    if (!(pfn))
	//use the old function
    {

		typedef BOOL (WINAPI * fnChangeWindowMessageFilter)( UINT, DWORD);
    fnChangeWindowMessageFilter pfn =
        reinterpret_cast<fnChangeWindowMessageFilter>(
           reinterpret_cast<void*>(
           GetProcAddress(GetModuleHandle(L"user32"),
                          "ChangeWindowMessageFilter")));
		return pfn(uMsg, MSGFLT_ADD);
    }
        return pfn(hWnd, uMsg, MSGFLT_ALLOW, NULL);
}