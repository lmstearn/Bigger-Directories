#include<stdlib.h> //malloc
#include <fcntl.h>
#include <io.h> //setmode
#include<stdio.h> //sprintf
#include <windows.h>
#include <Strsafe.h> //safe string copy e.e. StringCchPrintf
#include <tlhelp32.h> //Find process stuff
#include "BiggerDirectories.h" //my file
#include <Winternl.h> //NtCreateFile
#include <Strsafe.h>
//#include <afxwin.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


//#include <ntstatus.h>
//#include <ntstrsafe.h>


//http://www.cprogramming.com/tutorial/printf-format-strings.html
//https://msdn.microsoft.com/en-us/library/windows/desktop/aa378137(v=vs.85).aspx




wchar_t hrtext[256]; //An array name is essentially a pointer to the first element in an array.
WIN32_FIND_DATAW dw; // directory data this will use stack memory as opposed to LPWIN32_FIND_DATA
WIN32_FIND_DATAA da;
int const pathLength = 32759, maxPathFolder = MAX_PATH - 3, treeLevelLimit = 2000, branchLimit = 1000;
const wchar_t BOM = L'\xFEFF'; //65279
wchar_t const *invalidPathName = L":\"/\\|?*<>";
wchar_t const eolFTA = L'\n';
wchar_t const separatorFTA = L'\\';
wchar_t const *lpref = L"\\\\?\\";
wchar_t const *driveIDBaseW = L"\\\\?\\C:\\";
wchar_t const *driveIDBaseWNT = L"\\??\\C:\\"; //NtCreateFile wants the wildcard
char const *driveIDBase = "C:\\";
wchar_t rootDir [pathLength], dblclkPath [treeLevelLimit + 1][maxPathFolder], dblclkString [pathLength], reorgTmpW[pathLength];//maxPathFolder unless delete fails
wchar_t *pathToDeleteW, *currPathW, *findPathW, *tempDest, *thisexePath, *BiggerDirectoriesVAR; // directory pointers. cannot be initialised as a pointer
char *currPath;
//http://stackoverflow.com/questions/2516096/fastest-way-to-zero-out-a-2d-array-in-c
char dacfolders[127][MAX_PATH-3]; //[32768 / 257] [ MAX_PATH- 3] double array char is triple array
wchar_t dacfoldersW[255][MAX_PATH-3], dacfoldersWtmp[127][maxPathFolder], folderTreeArray[branchLimit + 1][treeLevelLimit + 1][maxPathFolder] = { NULL };
wchar_t reorgTmpWFS[treeLevelLimit][maxPathFolder], pathsToSave [branchLimit][pathLength];


int rootFolderCS, rootFolderCW, branchLevel, branchTotal, branchLevelCum, branchLevelClickOld, branchLevelClick, branchTotalSaveFile, branchLevelInc, branchLevelIncCum, branchSaveI, branchTotalCum, branchTotalCumOld, dblclkLevel;
int i,j,k, errCode;
int idata, index, listTotal = 0, sendMessageErr = 0;
int treeLevel, trackFTA[branchLimit][2];
int resResult;
bool resWarned;
bool foundResolution = false;
bool pCmdLineActive = false;
bool secondTryDelete = false;
bool createFail = false;
bool setforDeletion = false;
bool removeButtonEnabled = true;
bool nologonEnabled = false;
bool logonEnabled = false;
BOOL folderNotEmpty = false;
BOOL weareatBoot = FALSE;
BOOL am64Bit, exe64Bit;
PVOID OldValue = nullptr; //Redirection
WNDPROC g_pOldProc;
HANDLE keyHwnd, hMutex, hdlNtCreateFile, hdlNTOut, exeHandle, ds;     // directory handle
HINSTANCE appHinstance;

//struct fileSystem
//{
//    char FT[treeLevelLimit][branchLimit][maxPathFolder];
	//char FB[1000];

//};


typedef BOOL (__stdcall *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);



//typedef int (*NTDLLptr) (int); //Function pointer example, but following is required
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


NTDLLptr foundNTDLL = nullptr; //returns variable here
UNICODE_STRING fn;
OBJECT_ATTRIBUTES fileObject;
IO_STATUS_BLOCK ioStatus;
NTSTATUS ntStatus;
const char createFnString[13] = "NtCreateFile"; //one extra for null termination
const char initUnicodeFnString[21] = "RtlInitUnicodeString";
const char NtStatusToDosErrorString[22] = "RtlNtStatusToDosError";
const wchar_t CLASS_NAME[]  = L"ResCheckClass";
//A pathname MUST be no more than 32, 760 characters in length. (ULONG) Each pathname component MUST be no more than 255 characters in length (USHORT)
//wchar_t longPathName=(char)0;  //same as '\0'


//------------------------------------------------------------------------------------------------------------------
// Protos...
//------------------------------------------------------------------------------------------------------------------
BOOL WINAPI AboutDlgProc(HWND aboutHwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK RescheckWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ValidateProc(HWND, UINT, WPARAM, LPARAM); //subclass
int PopulateListBox (HWND hwnd, BOOL widecharNames);
int DoSystemParametersInfoStuff(HWND hwnd, bool progLoad);
int SwitchResolution (HWND hwnd, DLGPROC dProc);
int GetBiggerDirectoriesPath (HWND hwnd, wchar_t *exePath);
bool Kleenup (HWND hwnd, bool weareatBoot);
int ExistRegValue ();
DWORD FindProcessId(HWND hwnd, const wchar_t *processName, HANDLE hProcessName);
NTDLLptr DynamicLoader (bool progInit, wchar_t *fileObjVar);
bool CloseNTDLLObjs (BOOL atWMClose);
bool ProcessfileSystem(HWND hwnd, bool falseReadtrueWrite, bool appendMode);
void FSDeleteInit (HWND hwnd, HWND hList);
bool FSDelete (HWND hwnd);
bool fsDelsub (int i, int j, HWND hwnd);
int RecurseRemovePath(int trackFTA[branchLimit][2], wchar_t folderTreeArray[branchLimit + 1][treeLevelLimit + 1][maxPathFolder]);
// Start of HyperLink URL
void ShellError (HWND aboutHwnd, int nError);
LRESULT CALLBACK _HyperlinkParentProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK _HyperlinkProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
static void CreateHyperLink(HWND hwndControl);
// End of HyperLink URL


int DisplayError (HWND hwnd, LPCWSTR messageText, int errorcode, int yesNo)
{		//The way this is set up is errorcode is not modifiable here. However if errCode is passed here is always byval and will always revert to zero.
		//*hrtext  (pointee) is value pointed to by hrtext. Can be replaced by hrtext[0]
		//hrtext[0] = (wchar_t)LocalAlloc(LPTR, 256*sizeof(wchar_t)); This dynamic allocation NOT required- see below
		//if (hrtext[0] == NULL) ErrorExit("LocalAlloc");
		//hrtext[0] = NULL;  or	//*hrtext = NULL; //simple enough but not req'd

		if (errorcode == 0){
		swprintf_s(hrtext, _countof(hrtext), L"%s.", messageText);
		}
		else //LT 0 my defined error, GT error should be GET_LAST_ERROR
		{
		swprintf_s(hrtext, _countof(hrtext), L"%s. Error Code:  %d", messageText, errorcode);
		}
		//change countof sizeof otherwise possible buffer overflow: here index and rootFolderCS gets set to -16843010!
		if (yesNo)
		{
		int msgboxID = MessageBoxW(hwnd, hrtext, L"Warning", MB_YESNO);
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
		MessageBoxW(hwnd, hrtext, L"Warning", MB_OK);
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

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	
	
	StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("%s failed with error %lu: %s"), lpszFunction, dww, lpMsgBuf);
	wprintf(L"\a");  //audible bell
	MessageBoxW(nullptr, (LPCTSTR)lpDisplayBuf, L"Error", MB_OK);

	LocalFree(lpDisplayBuf);
	LocalFree(lpMsgBuf);
	
	//ExitProcess(dw);
}


void InitProc(HWND hwnd)
{
	TCHAR* pszTxt = TEXT("My Bigger Directory");
	TCHAR pszDest[arraysize];
	errCode = 0;


	size_t cbDest = arraysize * sizeof(TCHAR); //the use of size_t implies C++ compile.
	LPCTSTR pszFormat = TEXT("%s");
	HRESULT hr = StringCbPrintf(pszDest, cbDest, pszFormat, pszTxt); //swprintf_s
	
	
	//if (foundNTDLL) we can use the better function

	if (!DynamicLoader (true, tempDest)) DisplayError (hwnd, L"The long path function has been removed. Using 'short' path functions...", errCode, 0);

	
	#if defined(ENV64BIT) //#if is a directive: see header file
	{
    if (sizeof(void*) != 8)
    {
        	DisplayError (hwnd, L"ENV64BIT: Error: pointer should be 8 bytes. Exiting.", errCode, 0);
			if (exeHandle != INVALID_HANDLE_VALUE) CloseHandle(exeHandle);
			exit (1); //EndDialog will process the rest of the code in the fn.
    }
    am64Bit = true;
	exe64Bit = true;
	}
	#elif defined (ENV32BIT)
	{
		if (sizeof(void*) != 4)
		{
			DisplayError (hwnd, L"ENV32BIT: Error: pointer should be 4 bytes. Exiting.", errCode, 0);
			if (exeHandle != INVALID_HANDLE_VALUE) CloseHandle(exeHandle);
			ReleaseMutex (hMutex);
			exit (1);
		}
    
		if (FindProcessId (hwnd, L"BiggerDirectories.exe", exeHandle) != NULL)
		{

			am64Bit = false;
			exe64Bit = false;
			LPFN_ISWOW64PROCESS fnIsWow64Process;

			fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandleW((L"kernel32")),"IsWow64Process");
			if(nullptr != fnIsWow64Process)
				{
				exe64Bit = fnIsWow64Process(GetCurrentProcess(),&exe64Bit) && exe64Bit;
				}

		}

		else

		{
			DisplayError (hwnd, L"Our own process isn't active!? Must terminate!", 1, 0);
			ReleaseMutex (hMutex);
			exit (1); //EndDialog will process the rest of the code in the fn.
		}
}	

	#else
    //#error "user" gen error won't compile with current settings: "Must define either ENV32BIT or ENV64BIT". 128 bit?
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
	DisplayError (hwnd, L"Userinit should have ended. Try rebooting before running this (or any other) program!", errCode, 0);
	}
	//Now to check if I am 64 bit


	BiggerDirectoriesVAR= (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
	if (!ExpandEnvironmentStringsW (L"%SystemRoot%", BiggerDirectoriesVAR, maxPathFolder)) ErrorExit (L"ExpandEnvironmentStringsW failed for some reason.", 0);
	wcscat_s(BiggerDirectoriesVAR, maxPathFolder, L"\\Temp\\BiggerDirectories.exe");

		if (GetFileAttributesW(BiggerDirectoriesVAR) != INVALID_FILE_ATTRIBUTES)
		{
			logonEnabled = false;
			EnableWindow(GetDlgItem(hwnd, IDC_LOGON), logonEnabled);
		}
		else
		{
			EnableWindow(GetDlgItem(hwnd, IDC_LOGON), true);
		}
	nologonEnabled = true;
	EnableWindow(GetDlgItem(hwnd, IDC_NOLOGON), nologonEnabled);
	free(BiggerDirectoriesVAR);
	}
	
	if (ExistRegValue() == 1)
	{
		setforDeletion = TRUE;
	}


//Raw keyboard for input- need to subclass child controls for keystrokes to work
RAWINPUTDEVICE Rid[1];
Rid[0].usUsagePage = 0x01; 
Rid[0].usUsage = 0x06; 
Rid[0].dwFlags = 0;   // adds HID keyboard and invludes legacy keyboard messages
Rid[0].hwndTarget = 0;
if (RegisterRawInputDevices(Rid, 1, sizeof(Rid[0])) == FALSE) DisplayError (hwnd, L"Could not register Raw Input!", errCode, 0);



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
	dblclkLevel = 0;
	dblclkString[0] =L'';
	resResult = 0;
	resWarned = false;
	memset(dacfolders, '\0', sizeof(dacfolders));  //'\0' is NULL L'\0' is for C++ but we are compiling in Unicode anyway
	memset(dacfoldersW, L'\0', sizeof(dacfoldersW));
	memset(folderTreeArray, L'\0', sizeof(folderTreeArray)); //required for remove function
	memset(pathsToSave, L'\0', sizeof(pathsToSave)); //required for create
	memset(dblclkPath, L'\0', sizeof(dblclkPath));
	SetDlgItemTextW(hwnd,IDC_STATIC_ZERO, L"Add");
	SetDlgItemTextW(hwnd,IDC_STATIC_ONE, L"times.");
	SetDlgItemInt(hwnd, IDC_NUMBER, 3, FALSE);//set repeat number
	EnableWindow(GetDlgItem(hwnd, IDC_NUMBER), true);
	EnableWindow(GetDlgItem(hwnd, IDC_ADD), true);
	EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true);
	EnableWindow(GetDlgItem(hwnd, IDC_DOWN), false);
	EnableWindow(GetDlgItem(hwnd, IDC_UP), false);
	EnableWindow(GetDlgItem(hwnd, IDC_CREATE), false);
	removeButtonEnabled = true;
		
	for (j = 0; j < branchLimit; j++)
		{
		trackFTA [j][0] = 0; //Initial conditons before search on path
		trackFTA [j][1] = 0;
		}
	
	//Bad:
	//malloc(sizeof(char *) * 5) // Will allocate 20 or 40 bytes depending on 32 63 bit system
	//Good:
	// malloc(sizeof(char) * 5) // Will allocate 5 bytes

	currPath = (char *)calloc(maxPathFolder, sizeof(char));
	currPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
	if ((currPath == nullptr) || (currPathW == nullptr))
	{
		/* We were not so display a message */
		errCode = -1;
		DisplayError (hwnd, L"Could not allocate required memory", errCode, 0);
		return;
	}


	if (hr == S_OK)
	{
		SetDlgItemText(hwnd, IDC_TEXT, pszDest);

	}
	else
	{

		DisplayError (hwnd, L"StringCbPrintf didn't work, quitting: code %#08X", (int)hr, 0);
		return;

	}
	
sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
rootFolderCS = PopulateListBox (hwnd, false);
rootFolderCW = PopulateListBox (hwnd, true);


//http://stackoverflow.com/questions/1912325/checking-for-null-before-calling-free
//https://groups.google.com/forum/#!topic/comp.os.ms-windows.programmer.win32/L7o1PeransU
if (currPath) free (currPath);
if (currPathW) free (currPathW);
if (findPathW) free (findPathW);
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
      return CallWindowProc (g_pOldProc, hwnd, message, wParam, lParam);
}


INT_PTR CALLBACK DlgProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	errCode = 0;
	switch(Msg)
	{
		case WM_INITDIALOG:
			
            {	
				
		
			hMutex = CreateMutex( nullptr, TRUE, L"BiggerDirectories.exe" );
			if (hMutex)
			{
			DWORD wait_success = WaitForSingleObject (hMutex, 30 );
			if (wait_success == WAIT_OBJECT_0 || wait_success == WAIT_ABANDONED)
				{
				// Our thread got ownership of the mutex or the other thread closed without releasing its mutex.
						if (pCmdLineActive) 
							{
								secondTryDelete = true;

								InitProc (hwnd);
								SendDlgItemMessage(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
								FSDeleteInit (hwnd, nullptr);
								if (rootDir[0] != L'\0') rootDir[0] = L'\0';
								
							}

						InitProc (hwnd);
						HWND TextValidate = GetDlgItem(hwnd, IDC_TEXT);
						// Subclass the Edit control with ValidateProc
						g_pOldProc = (WNDPROC)SetWindowLong(TextValidate, GWL_WNDPROC, (LONG)ValidateProc);
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
							EndDialog(hwnd, 1);
						}
						break;
						case 3:
						{
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
						DisplayError (hwnd, L"One instance is already running!", errCode, 0);
						CloseHandle (hMutex);
						_CrtDumpMemoryLeaks();
						ExitProcess(1);
					}
				}
			}
			else
			{
				DisplayError (hwnd, L"Could not create hMutex!", errCode, 0);
				CloseHandle (hMutex);
				_CrtDumpMemoryLeaks();
				ExitProcess(1);
			}
				
				
				
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
						GetDlgItemInt(hwnd, IDC_NUMBER, &bSuccess, FALSE);
						if (bSuccess)
						{
						if (GetDlgItemInt(hwnd, IDC_NUMBER, &bSuccess, FALSE) >	treeLevelLimit)
							{
								SetDlgItemInt(hwnd, IDC_NUMBER, (UINT)(treeLevelLimit -1), FALSE);
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
					int nTimes = GetDlgItemInt(hwnd, IDC_NUMBER, &bSuccess, FALSE);
					if(bSuccess) 
					{
						//Allocate memory (2* +1 for two words > long)

						len = (GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT))); //wchar
						if(len > 0)
						{

							
							wchar_t *buf1 = (wchar_t *)calloc(2 * len + 1, sizeof(wchar_t));
							GetDlgItemTextW(hwnd, IDC_TEXT, buf1, 2 * len + 1);


						//validation for terminating space & period
			
							for(i = len-1; i >= 0; i--)
							{

								if (!(wcsncmp(&buf1[i], L" ", 1 )) || !(wcsncmp(&buf1[i], L".", 1)) )
								{
									wcscpy_s(&buf1[i], i, L"\0");
									SetDlgItemTextW(hwnd, IDC_TEXT, (wchar_t*)(buf1));
								}

								else

								{
								break; //all good
								}
							}
						free(buf1);
						}

						int len = 2 * (GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT)) + 1); //wchar again
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
									DisplayError (hwnd, L"32k Limit reached!", errCode, 0);
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
							DisplayError (hwnd, L"You didn't enter anything!", errCode, 0);
							goto NoAddSuccess;
						}
					}
					else 
					{
						errCode = 0;
						DisplayError (hwnd, L"Couldn't translate that number :(", errCode, 0);
						goto NoAddSuccess;
					}


					if (foundNTDLL)
					{
					//update branchTotal: always 0 for the first branch

					//populate after the save file contents
						if (branchTotal < branchLimit - 1)
						{
						branchTotal +=1;
						}
						else
						{
						DisplayError (hwnd, L"Limit of number of directories reached. Cannot create anymore!", errCode, 0);
						goto NoAddSuccess;
						}

						(branchLevelClick) ? EnableWindow(GetDlgItem(hwnd, IDC_DOWN), true) : EnableWindow(GetDlgItem(hwnd, IDC_DOWN), false);
						//next add is always at base
						EnableWindow(GetDlgItem(hwnd, IDC_UP), true);
						HWND hList = GetDlgItem(hwnd, IDC_LIST);
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
							DisplayError (hwnd, L"Limit of number of nested directories reached. Cannot create anymore!", errCode, 0);
							goto NoAddSuccess;
							}
						}

						//clear redundant branches
						for (j = branchLevelClick + branchLevel; j < treeLevelLimit; j++)
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
					DisplayError (hwnd, L"NTDLL not found: Only one nested path is made with CREATE.", errCode, 0);
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
					if (branchLevelClick == branchLevel + branchLevelClickOld ) EnableWindow(GetDlgItem(hwnd, IDC_UP), false);


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



					if (foundNTDLL)
					{
						currPathW = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
						if (currPathW == nullptr)
							{
							/* We were not so display a message */
							errCode = -1;
							DisplayError (hwnd, L"Could not allocate required memory to initialize String.", errCode, 0);
							goto EndCreate;
							}
						
						(!branchTotal && createFail)? branchTotalCum = -1: branchTotalCum = 0;
						//Load FS into branchTotalSaveFile + 1 (appendMode true so FS loaded after)
						if (!ProcessfileSystem (hwnd, false, true))
						{
							if (DisplayError (hwnd, L"Problem with FS file! Try alternate Create?", 0, 1))
							{
								free (currPathW);
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
								if FAILED(StringCchCatW(pathsToSave[i], pathLength, folderTreeArray[i][j]))
								{
									for (k = j; (k < trackFTA [i][0] + trackFTA [i][1]) && (folderTreeArray[i][j][0] != L'\0'); k++)
										{
											folderTreeArray[i][k][0] = L'\0';
											trackFTA [k][0] = 0;
											trackFTA [k][1] = 0;
										}
									DisplayError (hwnd, L"32k Limit reached. truncating!", errCode, 0); //the unconcatenated strings checked before
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
												pathsToSave[k][0] = L'\0';
												trackFTA [k][0] = 0;
												trackFTA [k][1] = 0;
												for (int l = 0; l < treeLevelLimit; l++) folderTreeArray[k][l][0] = L'\0';
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
								DisplayError (hwnd, L"Informational: No error ", Status, 0);
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
									DisplayError (hwnd, L"There was another error prior to this on directory create. The create function is not available. Try again after deleting a line or clearing the list.", Status, 0);
								}
								else
								{
									ErrorExit (L"NtCreateFile: ", Status);
								}
								createFail = true;
								errCode = 1;
								goto EndCreate;
							}
						}
						//SetCurrentDirectory Often fails here at root node with error 32 "used by another process"
						if (!CloseNTDLLObjs(false)) DisplayError (hwnd, L"NtCreateFile: Objects failed to close ", errCode, 0);
						}


						else
						{
							errCode = 1;
							createFail = true;
							ErrorExit (L"DynamicLoader failed: Cannot create!", 1);
							goto EndCreate;
						}
					} //trackFTA condition
					}
						if (folderTreeArray[i][0][0] != L'\0') branchTotalCum +=1; // for rollback
					}

					//sort all and write to file

					if (!ProcessfileSystem(hwnd, true, true))
						{
							DisplayError (hwnd, L"There was an error writing data to file. This program may not be able to delete directories just created. If their deletion is required in the future, run 7-zip and shift-del.", errCode, 0);
							goto EndCreate;
						}
				} //foundNtdll
				goto EndCreate;

				AltCreate:

				currPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));

				if (currPathW == nullptr)
					{
					/* We were not so display a message */
					errCode = -1;
					DisplayError (hwnd, L"Could not allocate required memory to initialize String.", errCode, 0);
					goto EndCreate;
					}


				//Another loop & variables for recursive create here
				for (i = rootFolderCS + rootFolderCW; i < listTotal; i++)
				{
				sendMessageErr = SendMessageW(hList, LB_GETTEXT, i, (LPARAM)currPathW);
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
							if (Wow64DisableWow64FsRedirection(&OldValue))
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
							if (!Wow64RevertWow64FsRedirection(&OldValue))
							{
							DisplayError (hwnd, L"Problems with redirection...", errCode, 0);
							goto EndCreate;
							}

						}


					}


				//There is a default string size limit for paths of 248 characters
				//errCode = CreateDirectoryW(cumPath, NULL);

				//wcscpy_s(currPathW, maxPathFolder, driveIDBaseW);
				//\a  audible bell

				//LB_GETTEXTLEN  https://msdn.microsoft.com/en-us/library/windows/desktop/bb761315(v=vs.85).aspx
					

				//longPathName
				//Clear all the added items
				EndCreate:
				if (foundNTDLL)
				{
					if (!CloseNTDLLObjs(false)) DisplayError (hwnd, L"NtCreateFile: Objects failed to close ", errCode, 0);

				if (createFail)
				{
					//Write the first successful block, but if second error don't write same stuff again
					if (branchTotalCum > 0)
					{

						if (!ProcessfileSystem(hwnd, true, true)) DisplayError (hwnd, L"There was another error, this time writing data to file. This program may not be able to delete the created directories. To do so run 7-zip and shift-del.", errCode, 0);
					
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

				if (currPathW) free(currPathW);
				//free(cumPath);
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
				}
				break;


				case IDC_REMOVE:
				{
					//When the user clicks the Remove button, we first get the number of selected items

					HWND hList = GetDlgItem(hwnd, IDC_LIST);
					int count = SendMessageW(hList, LB_GETSELCOUNT, 0, 0);
					listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);
					if (count != LB_ERR)
					{
						
						sendMessageErr = SendMessageW(hList, LB_GETCURSEL, (WPARAM)1, (LPARAM)&index); //GETSELITEMS substituted with LB_GETCURSEL for a laugh

						//index = SendMessageW(hList, LB_GETCURSEL, 0, 0L);
						if (count == 1 && (index < (rootFolderCS + rootFolderCW)))
						{
							
						FSDeleteInit (hwnd, hList);

						}
						else

						{
							if (count != 0)
							{

								// And then allocate room to store the list of selected items.


								int *buf = (int*)GlobalAlloc(GPTR, sizeof(int) * count);
								sendMessageErr = SendMessageW(hList, LB_GETSELITEMS, (WPARAM)count, (LPARAM)buf);
								//index = SendMessageW(hList, LB_GETCURSEL, 0, 0L) + 1; //+ 1 ???
								// Now we loop through the list and remove each item that was selected, looping backwards, because if we removed items from top to bottom, it would change the indexes of the other items!!!

								for (i = count - 1; i >= 0; i--)
								{
									sendMessageErr = SendMessageW(hList, LB_DELETESTRING, (WPARAM)buf[i], 0);
								}

								GlobalFree(buf);


								
								branchLevelCum -= 1;
								branchLevel -= 1;
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
								sendMessageErr = SendMessageW(hList, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
								sendMessageErr = SendMessageW(hList, LB_SETTOPINDEX, (WPARAM)((rootFolderCS + rootFolderCW + branchLevelIncCum)), 0);
								listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);
								


							}
							else
							{
								errCode = 0;
								DisplayError (hwnd, L"No items selected...", errCode, 0);
							}
						}
					}
					else
					{
						errCode = 0;
						DisplayError (hwnd, L"Error counting items...", errCode, 0);
					}
			}
			break;
			case IDC_CLEAR:
				{

					if (dblclkLevel)
					{
						sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
						sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)(L".."));
					}
					else
					{
						InitProc(hwnd);
					}
				}
			break;

				case IDC_LOGON:
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


				system ("CD\\ & PUSHD %SystemRoot%\\Temp & SET KEY_NAME=\"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" & SET \"VALUE_NAME=Userinit\" & REG QUERY \"HKLM\\Hardware\\Description\\System\\CentralProcessor\\0\" | FIND /i \"x86\" >NUL && CALL SET \"OSB=\" || CALL SET \"OSB=64BIT\" & (IF DEFINED OSB (FOR /F \"USEBACKQ SKIP=2 TOKENS=1-4 DELIMS= \" %G IN (`REG QUERY %KEY_NAME% /v %VALUE_NAME% /reg:64 2^>Userinitregerror.txt`) DO @SET \"CURREGVALUE=%I%J\") ELSE ((FOR /F \"USEBACKQ SKIP=2 TOKENS=1-4 DELIMS= \" %G IN (`REG QUERY %KEY_NAME% /v %VALUE_NAME% 2^>Userinitregerror.txt`) DO @SET \"CURREGVALUE=%I%J\"))) & >NUL FINDSTR \"^\" \"Userinitregerror.txt\" && SET \"ERRTXT=\" || SET \"ERRTXT=1\" & (IF DEFINED ERRTXT (>Userinitreg.txt CALL ECHO %CURREGVALUE% & (IF '%errorlevel%' NEQ '0' (CALL ECHO Copy reg record failed! & PAUSE >NUL))) ELSE (ECHO No reg key! & PAUSE NUL)) & (IF DEFINED OSB (CALL CALL SET \"NEWREGVALUE=%SystemRoot%\\Temp\\BiggerDirectories.exe\") ELSE (CALL CALL SET \"NEWREGVALUE=%CURREGVALUE:%SystemRoot%\\system32\\userinit.exe=%SystemRoot%\\Temp\\BiggerDirectories.exe,%SystemRoot%\\system32\\userinit.exe%\")) & CALL REG ADD %KEY_NAME% /v %VALUE_NAME% /d %NEWREGVALUE% /f /reg:64 & POPD");
					
				free (thisexePath);
				free (tempDest);
				logonEnabled = false;
				EnableWindow(GetDlgItem(hwnd, IDC_LOGON), logonEnabled);


				//NOTE WOW6432node is 64bit view of 32bit setting. reg:64 bypasses VS 32bit redirection
				//Debug & CALL ECHO %KEY_NAME% %VALUE_NAME% %CURREGVALUE% %NEWREGVALUE%
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
			if (!DisplayError (hwnd, L"The PendingFileRenameOperations key already has data. Please reply no and check the key's value if unsure whether another program besides this one has marked another file for deletion at reboot.", errCode, 1)) break;
			
			//delete the key ExistRegValue
			system ("REG DELETE \"HKLM\\System\\CurrentControlSet\\Control\\Session Manager\" /v PendingFileRenameOperations /f");

			}
			
			if (Kleenup (hwnd, weareatBoot))
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
						DisplayError (hwnd, L"Something has gone wrong with memory!", errCode, 0);
						return 0;
						}


						int count = SendMessageW(hList, LB_GETSELCOUNT, 0, 0);
						bool removeTrig = false;
						if(count != LB_ERR)
						{
							// We only want to continue if one and only one item is
							// selected.

							if(count == 1)
							{

								//place string into text box
								index = SendMessageW(hList, LB_GETCURSEL, 0, 0L);
								if (index == LB_ERR)
								{
									DisplayError (hwnd, L"Something has gone wrong with the Listbox!", errCode, 0);
									if (currPathW) free (currPathW);
									return 0;
								}								
								sendMessageErr = SendMessageW(hList, LB_GETTEXT, index, (LPARAM)currPathW);
								SetDlgItemTextW(hwnd, IDC_TEXT, currPathW);

								// Since we know ahead of time we're only getting one index, there's no need to allocate an array.
									
								//sendMessageErr = SendMessageW(hList, LB_GETCURSEL, (WPARAM)1, (LPARAM)&index); //GETSELITEMS substituted with LB_GETCURSEL for a laugh
									

								if (index >= rootFolderCS + rootFolderCW)
								{
									SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Line\0");
									if (index >= listTotal - branchLevel)
									{
									EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true);
									}
									else
									{
									EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
									}
								}
								else
								{

									SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Dir\0");

									if (wcsstr(dacfoldersW[index], lpref) == nullptr)
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
									DisplayError (hwnd, L"Error getting selected item :(", errCode, 0);
								}
							}
							else 
							{
								// No items selected, or more than one.Either way, we aren't going to process this.
									
								sendMessageErr = SendMessageW(hList, LB_GETANCHORINDEX, 0, 0L);
								int selItems[32767];
								sendMessageErr = SendMessage(hwnd, LB_GETSELITEMS, count, (LPARAM)selItems);

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
									if (!removeTrig)
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




								SetDlgItemTextW(hwnd, IDC_SHOWCOUNT, L"-");
							}
						}
						else
						{
							errCode = 0;
							DisplayError (hwnd, L"Error counting items :(", errCode, 0);
						}
						if (currPathW) free (currPathW);
					}
					break;
					case LBN_DBLCLK:
					{
					HWND hList = GetDlgItem(hwnd, IDC_LIST);
					currPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
					findPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
					if ((currPathW == nullptr) || (findPathW == nullptr))
					{
					DisplayError (hwnd, L"Something has gone wrong with memory!", errCode, 0);
					return 0;
					}
					index = SendMessageW(hList, LB_GETCURSEL, 0, 0L);
					if ((index >= rootFolderCS + rootFolderCW) && (!dblclkLevel)) goto DblclkEnd;

					sendMessageErr = SendMessageW(hList, LB_GETTEXT, index, (LPARAM)findPathW);
					
					dblclkString[0]= L'';
					if ((0 == wcscmp(findPathW, L"..")))
					{
						dblclkLevel -=1;
						if (!dblclkLevel) goto DblclkEnd;
							for (i = 0; i < dblclkLevel; i++)
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
						wcscpy_s(dblclkPath[dblclkLevel], maxPathFolder, findPathW);

						for (i = 0; i <= dblclkLevel; i++)
						{
						wcscat_s(dblclkString, pathLength, dblclkPath[i]);
						wcscat_s(dblclkString, pathLength, L"\\");
						}
						dblclkLevel +=1;
					}
					DblclkEnd:
					
					if (dblclkLevel)
						{
							SendDlgItemMessage(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
							PopulateListBox (hwnd, true);
							sendMessageErr = SendMessageW(hList, LB_DELETESTRING, (WPARAM)0, 0); //remove the '.'
							//disable buttons
							SetDlgItemTextW(hwnd,IDC_STATIC_ZERO, L"Dir:");
							SetDlgItemTextW(hwnd,IDC_STATIC_ONE, L"");
							SetDlgItemInt(hwnd, IDC_NUMBER, 0, FALSE);
							SetDlgItemText(hwnd, IDC_TEXT, dblclkPath[dblclkLevel-1]);
							SetDlgItemInt(hwnd, IDC_SHOWCOUNT, 0, FALSE);
							EnableWindow(GetDlgItem(hwnd, IDC_NUMBER), false);
							EnableWindow(GetDlgItem(hwnd, IDC_ADD), false);
							EnableWindow(GetDlgItem(hwnd, IDC_UP), false);
							EnableWindow(GetDlgItem(hwnd, IDC_DOWN), false);
							EnableWindow(GetDlgItem(hwnd, IDC_CREATE), false);
							EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
							if (findPathW) free (findPathW);
							if (currPathW) free (currPathW);
						}
					else
						{
							if (findPathW) free (findPathW);
							if (currPathW) free (currPathW);
							InitProc(hwnd);
							//enable buttons
							return 0;
						}
					}
					break; //This break for consistency
				}
			break;
			} //end WM_COMMAND
		break;
/*		        case WM_KEYDOWN: 
            switch (wParam) 
            {

			}*/
		case WM_CLOSE:
			{
			//Cleanup
			if (weareatBoot) Kleenup (hwnd, weareatBoot);
			 
			if (exeHandle != INVALID_HANDLE_VALUE) CloseHandle(exeHandle);
				if (foundNTDLL)
				{
					if (!CloseNTDLLObjs(true)) DisplayError (hwnd, L"NtCreateFile: Objects failed to close ", errCode, 0);
				}
			EndDialog(hwnd, 0);
			_CrtDumpMemoryLeaks();
			}
		break;

		case WM_DESTROY:
			PostQuitMessage(0);
		break;
		default: return FALSE;
		break;	
	}
	return TRUE;
}
BOOL WINAPI AboutDlgProc(HWND aboutHwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	
	switch(uMsg)
	{
	case WM_INITDIALOG:
	{
	CreateHyperLink(GetDlgItem(aboutHwnd, IDC_STATIC_FOUR));
	CreateHyperLink(GetDlgItem(aboutHwnd, IDC_STATIC_FIVE));
	PlaySound(MAKEINTRESOURCE(IDW_CLICK), (HMODULE)GetWindowLong(aboutHwnd, GWL_HINSTANCE), SND_RESOURCE | SND_ASYNC);
	return true;
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
						ShellError(aboutHwnd, (int) ShellExecuteW(NULL, L"open", L"http://members.ozemail.com.au/~lmstearn/index.html", NULL, NULL, SW_SHOWNORMAL));
					}
				}           
				break;
				case IDC_STATIC_FIVE:
				{
					{
						ShellError(aboutHwnd, (int) ShellExecuteW(NULL, L"open", L"http://members.ozemail.com.au/~lmstearn/index.html", NULL, NULL, SW_SHOWNORMAL));
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
						DisplayError (aboutHwnd, L"Form testing: No controls? Spacebar loads new dialog", 0, 0);
						resWarned = true;
					}


				SwitchResolution (aboutHwnd, AboutDlgProc);
				EndDialog(aboutHwnd, IDC_OK);
				break;

				}
				break;

			break;

	case WM_CLOSE:
			{
				EndDialog(aboutHwnd, IDC_OK);
			}
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
	

	// Create a new window see https://msdn.microsoft.com/en-us/library/windows/desktop/ff381397(v=vs.85).aspx
	WNDCLASS wc = { };
	wc.lpfnWndProc   = RescheckWindowProc;
	wc.hInstance     = hInstance;
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);

	HWND hwnd = CreateWindowExW(
		0,								// Optional window styles.
		CLASS_NAME,						// Window class
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

		appHinstance = GetModuleHandle(NULL); //same as hInstance: use for application hInstance: (okay for exe not for DLL)
		resResult = DoSystemParametersInfoStuff(hwnd, true);
		SwitchResolution (nullptr, DlgProc);
	}
return 0; //never gets here, but suppress C4715 warning
}

LRESULT CALLBACK RescheckWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
	//temp windowfor res check.
}

int PopulateListBox (HWND hwnd, BOOL widecharNames)
{
BOOL findhandle = TRUE;
int listNum = 0;

if (widecharNames == 0)
{
	memset(&da, 0, sizeof(WIN32_FIND_DATA));
	strcpy_s(currPath, maxPathFolder, driveIDBase);

	strcat_s(currPath, maxPathFolder, "*");
	ds = FindFirstFileA(currPath, &da);

}
else
{
	memset(&dw, 0, sizeof(WIN32_FIND_DATAW));
	//http://stackoverflow.com/questions/32540779/wcscpy-does-not-accept-tchar-in-destination-variable
	(dblclkLevel)? wcscat_s(currPathW, maxPathFolder, dblclkString): wcscpy_s(currPathW, maxPathFolder, driveIDBaseW);
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

		// (wchar_t *)currPathW not necessary here
			strcpy_s(currPath, maxPathFolder, (char *)driveIDBase);
			strcat_s(currPath, maxPathFolder, da.cFileName);


			strcat_s(dacfolders[listNum], maxPathFolder, currPath);
			listNum += 1;

			SendDlgItemMessageA(hwnd, IDC_LIST, LB_ADDSTRING, (WPARAM)(listNum), (LPARAM)currPath); // wparam cannot exceed 32,767 
					
		}
		findhandle = FindNextFileA(ds, &da);
	}
	else
	{
		if ((dw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(dw.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM
			|| dw.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT || dw.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE || dw.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {

		(dblclkLevel)? currPathW[0]= L'': wcscpy_s(currPathW, maxPathFolder, (wchar_t *)driveIDBaseW);
		wcscat_s(currPathW, maxPathFolder, dw.cFileName);
			
				
		//compare with dacfolders[rootFolderC] to check for dups
		wcscat_s(dacfoldersW[listNum], maxPathFolder, currPathW);
		listNum += 1;
		sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_ADDSTRING, (WPARAM)(rootFolderCS + listNum), (LPARAM)currPathW); // wparam cannot exceed 32,767 
				
		}
	findhandle = FindNextFileW(ds, &dw);

	}
	sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_SETITEMDATA, (WPARAM)(rootFolderCS + listNum), (LPARAM)(rootFolderCS + listNum));
//(lparam)rootFolderC required for getitemdata
//The Notification Code is passed as the HIWORD of wParam, the other half of the parameter that gave us the index of the control identifier in the first place. 
//HIWORD is the Upper 16 bits of UINT and LOWORD is the Lower 16 bits of UINT
//DWORD is just a typedef for 32-bit integer, whereas WORD is a typedef for a 16-bit integer
}

FindClose(ds);
return listNum;
}

int DoSystemParametersInfoStuff(HWND hwnd, bool progLoad)
{
	HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monInfo;
	monInfo.cbSize = sizeof(MONITORINFO);

	if (progLoad) //destroy ephemeral window 
	{
	if (!DestroyWindow(hwnd)) DisplayError (hwnd, L"Rescheck window cannot be destroyed!", 0, 0);
	UnregisterClassW(CLASS_NAME, appHinstance);
	}

if (GetMonitorInfo (hMon, &monInfo))
{
	if ((monInfo.rcMonitor.right - monInfo.rcMonitor.left) > 5000) return 1;
	if ((monInfo.rcMonitor.right - monInfo.rcMonitor.left) > 3000) return 2;
	if ((monInfo.rcMonitor.right - monInfo.rcMonitor.left) > 2000) return 3;
	if ((monInfo.rcMonitor.right - monInfo.rcMonitor.left) > 600) return 4;
	else return 5;
}
else
{
	DisplayError (hwnd, L"GetMonitorInfo: Cannot get info!", 0, 0);
}
return 0;

}
int SwitchResolution (HWND hwndParent, DLGPROC dProc)
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
	switch (resResult)
	{
		case 1:
			return DialogBoxW(appHinstance, MAKEINTRESOURCEW(IDD_4320P), nullptr, (DLGPROC)dProc);
		break;
		case 2:
			return DialogBoxW(appHinstance, MAKEINTRESOURCEW(IDD_2160P), nullptr, (DLGPROC)dProc);
		break;
		case 3:
			return DialogBoxW(appHinstance, MAKEINTRESOURCEW(IDD_1080P), nullptr, (DLGPROC)dProc);
		break;
		case 4:
			return DialogBoxW(appHinstance, MAKEINTRESOURCEW(IDD_768P), nullptr, (DLGPROC)dProc);
		break;
		default:
			return DialogBoxW(appHinstance, MAKEINTRESOURCEW(IDD_SMALL), nullptr, (DLGPROC)dProc);
		break;
	}

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
	if( Wow64DisableWow64FsRedirection(&OldValue) ) 
    {
	result  = GetModuleFileNameW(nullptr, exePath, MAX_PATH-4);
    errCode = (int)GetLastError();
	}
	else
	{
		DisplayError (hwnd, L"Problems with redirection...", errCode, 0);
		return 1;
	}
    if (!Wow64RevertWow64FsRedirection(&OldValue) ) 
	{
		DisplayError (hwnd, L"Problems with redirection...", errCode, 0);
		return 1;
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
			DisplayError (hwnd, L"Unknown error has occurred.", errCode, 0);
			}
	}
	return 1;
}
else
{
	return 0;
}
}
bool Kleenup (HWND hwnd, bool weareatBoot)
{
	STARTUPINFOW lpStartupInfo;
	PROCESS_INFORMATION lpProcessInfo;

	ZeroMemory(&lpStartupInfo, sizeof(lpStartupInfo));
	ZeroMemory (&lpStartupInfo, sizeof(lpStartupInfo));
	thisexePath = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
	tempDest = (wchar_t *)calloc(pathLength, sizeof(wchar_t));

	if (pCmdLineActive) //on program restart on remove root directory bug
	{
		if (!GetModuleFileNameW(nullptr, thisexePath, pathLength) || (wcslen(thisexePath) > pathLength))
		{
			DisplayError (hwnd, L"Oops, process path too long!? or non-existent?! Quitting...", 0, 0);
			free (tempDest);
			free (thisexePath);
			return false;
		}
		else
		{
		wcscpy_s (tempDest, pathLength, L"\"");
		wcscat_s (tempDest, pathLength, thisexePath);
		wcscat_s (tempDest, pathLength, L"\" ");
		wcscat_s (tempDest, pathLength, pathToDeleteW);
		if (!CreateProcessW (thisexePath, tempDest, nullptr, nullptr, FALSE, NULL, nullptr, nullptr, &lpStartupInfo, &lpProcessInfo)) ErrorExit (L"Oops: Something went wrong. Please restart the program...", 0);
		free (tempDest);
		free (thisexePath);
		CloseHandle(lpProcessInfo.hProcess);
		CloseHandle(lpProcessInfo.hThread);
		}
	}
	else
	{

			system ("CD\\ & PUSHD %SystemRoot%\\Temp & REG QUERY \"HKLM\\Hardware\\Description\\System\\CentralProcessor\\0\" | FIND /i \"x86\" >NUL && CALL SET \"OSB=\" || CALL SET \"OSB=64BIT\" & SET KEY_NAME=\"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" & SET \"VALUE_NAME=Userinit\" & SET \"Userinitreg=\" & (IF EXIST Userinitreg.txt CALL SET \"Userinitreg=TRUE\") & (IF DEFINED Userinitreg (FOR /F \"usebackq tokens=1,2* delims=,\" %G IN (\"Userinitreg.txt\") DO SET \"REGVALUE=%G\" & (IF DEFINED OSB (CALL SET \"REGVALUE=%REGVALUE%,\" & CALL REG ADD %KEY_NAME% /v %VALUE_NAME% /d %REGVALUE% /f /reg:64) ELSE (CALL REG ADD %KEY_NAME% /v %VALUE_NAME% /d %REGVALUE% /f))) ELSE (CALL ECHO Backup reg record does not exist. Was NoLogon already called? & PAUSE >NUL)) & (IF EXIST Userinitregerror.txt (DEL \"Userinitregerror.txt\")) & DEL \"Userinitreg.txt\" & POPD");
			
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
				//The reg value is restored to userinit before theis point
				}
			if(!MoveFileExW(tempDest,nullptr,MOVEFILE_DELAY_UNTIL_REBOOT))
			{
			DisplayError (hwnd, L"Problems with file deletion. Solved with next Disk Cleanup...", 0, 0);
			free (tempDest);
			free (thisexePath);
			return false;
			}
			else
			{
			free (tempDest);
			free (thisexePath);
			}
	}
	return true;
}
int ExistRegValue ()
{
	bool retVal = false;
	bool openVal = false;
	wchar_t *keyName, *valueName, *subKeyName;
	HKEY hKey;
	keyName = (wchar_t *)calloc(260, sizeof(wchar_t));
	valueName = (wchar_t *)calloc(260, sizeof(wchar_t));
	subKeyName = (wchar_t *)calloc(260, sizeof(wchar_t));
	wcscpy_s(keyName, 260, L"HKLM\\System\\CurrentControlSet\\Control\\Session Manager");
	wcscpy_s(subKeyName, 260, L"System\\CurrentControlSet\\Control\\Session Manager");
	wcscpy_s(valueName, 260, L"PendingFileRenameOperations");
	if (RegOpenKeyEx (HKEY_LOCAL_MACHINE, subKeyName, 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS)
	{
		ErrorExit (L"Could not open Session Manager key!",0);
	}
	else
	{
		openVal = true;
	}
		

	if (RegQueryValueExW(hKey, valueName, nullptr, nullptr, nullptr, nullptr) != ERROR_FILE_NOT_FOUND) retVal = true;

	free (keyName);
	free (valueName);
	free (subKeyName);


	if (openVal && RegCloseKey (hKey) != ERROR_SUCCESS)
		{
			ErrorExit (L"Could not close Session Manager key!",0);
		}
	return retVal;
		// \??\C:\Windows\Temp\BiggerDirectories.exe: nasty multo_sz

}
DWORD FindProcessId(HWND hwnd, const wchar_t *processName, HANDLE hProcessName)
{
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    DWORD result = NULL;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hProcessSnap) return(FALSE);

    pe32.dwSize = sizeof(PROCESSENTRY32); // <----- IMPORTANT

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);          // clean the snapshot object
        DisplayError (hwnd, L"!!! Failed to gather information on system processes! \n", 1, 0);
        return(NULL);
    }

    do
    {
		

		
        if (0 == wcscmp(processName, pe32.szExeFile))
        {
            result = pe32.th32ProcessID;
			hProcessName = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
			if(hProcessName == nullptr )
				{
					DisplayError (hwnd, L"Cannot open this process!", 1, 0);
					CloseHandle(hProcessSnap);
					return(NULL);
				}
            break;
        }

    } while (Process32Next(hProcessSnap, &pe32));

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
			if( !(RtlNtStatusToDosError = (PFN_RtlNtStatusToDosError) GetProcAddress( (HMODULE)hdlNtCreateFile, NtStatusToDosErrorString )) ) return nullptr;

			}
			else
			{
			//init Unicode string
			PFN_RtlInitUnicodeString RtlInitUnicodeString;
			if( !(RtlInitUnicodeString = (PFN_RtlInitUnicodeString) GetProcAddress( (HMODULE)hdlNtCreateFile, initUnicodeFnString )) ) return nullptr;
			RtlInitUnicodeString(&fn, fileObjVar);
			fileObject.ObjectName = &fn; //Ntdll.dll
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
	memset(&ioStatus, 0, sizeof(ioStatus));
	memset(&fileObject, 0, sizeof(fileObject));

	if (!atWMClose && hdlNTOut) 
	{
		if (CloseHandle (hdlNTOut) == (ERROR_INVALID_HANDLE)) returnClose = false;
		fileObject.Length = sizeof(fileObject);
		fileObject.Attributes = OBJ_CASE_INSENSITIVE;
	}

	if (hdlNtCreateFile)
	{
		if (!FreeLibrary ((HMODULE)hdlNtCreateFile)) returnClose = false; 
	}
	ntStatus = NULL;

	return returnClose;
}

bool ProcessfileSystem(HWND hwnd, bool falseReadtrueWrite, bool appendMode)
{
	DWORD Status;
	int  result;
	int  jLim;
	wint_t ch = 0, chOld = 0;
	FILE *stream = nullptr;
	bool fsReturn = true;
	wchar_t *tempDestOld = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
	wchar_t *fsName= (wchar_t *)calloc(pathLength, sizeof(wchar_t));

	if (!ExpandEnvironmentStringsW (L"%SystemRoot%", fsName, pathLength)) ErrorExit (L"ExpandEnvironmentStringsW failed for some reason.",0);
	wcscat_s(fsName, pathLength, L"\\Temp\\BiggerDirectories.txt");
	stream = _wfopen(fsName, L"r+b");
	//If the file already exists and is opened for reading or appending, the Byte Order Mark (BOM), if it present in the file, determines the encoding.
	if (!stream) //returns NULL Pointer
	{

	if (DisplayError (hwnd, L"_wfopen returns NULL: possible first time run? Click yes to create new file, no to abort...", 0, 1))
		{

			stream = _wfopen(fsName, L"w+b");
			if (stream == nullptr) 
			{
				ErrorExit (L"Problems with opening input File.", 0);
				fsReturn = false;
				goto WEOFFOUND;

			}
			_setmode(_fileno(stdout), _O_U16TEXT);
			//write BOM for byte-order endianness (storage of most/least significant bytes) and denote Unicode steream
				if(fputwc(BOM, stream) == EOF)
				//if (fwrite("\xFEFF", 2, 2, stream) < 0)
		
				{
					ErrorExit (L"fwprintf: Problems with writing to input File.", 0);
					fsReturn = false;
					goto WEOFFOUND;
				}
		
		}
		else
		{
		free (fsName);
		return false; //won't read/write on empty file
		}


	}
	else //file exists
	{
	
		if (appendMode)
		{
		stream = _wfopen(fsName, L"a+b");
		//When you switch from writing to reading, you must use an intervening call to either fflush or a file positioning function.
		if (!stream) //returns NULL Pointer
		{
		ErrorExit (L"Problems with input File: Cannot append.", 0);
		fsReturn = false;
		goto WEOFFOUND;
		}

		}
		else //load FS when deleting
		{
		//BOM must be rewritten as it is wiped
		(falseReadtrueWrite)? stream = _wfopen(fsName, L"w+b"): stream = _wfopen(fsName, L"rb");
			_setmode(_fileno(stdout), _O_U16TEXT);
		//write BOM for byte-order endianness (storage of most/least significant bytes) and denote Unicode steream
			if(fputwc(BOM, stream) == EOF)
			//if (fwrite("\xFEFF", 2, 2, stream) < 0)
		
			{
				ErrorExit (L"fwprintf: Problems with writing to input File.", 0);
				fsReturn = false;
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
	
	ch = 1;
	(appendMode)? i = branchTotal + 1: i = 0;
	result = fseek(stream, 0L, SEEK_SET);  /* moves the pointer to the beginning of the file */
	//rewind(stream); //does the same?
	if (result)
	{
	ErrorExit (L"fseek: Could not rewind!", 0);
	fsReturn = false;
	goto WEOFFOUND;
	}

	//Read BOM
	ch = fgetwc(stream);
	
	if(ch != BOM)
		
	{
		DisplayError(hwnd, L"fgetwc: input file does not have BOM!", 0, 0);
		fsReturn = false;
		goto WEOFFOUND;
	}


		do
		{
		//we are reading so last null condition mandatory
		
			if (ch == eolFTA) ch = 1;	//ugly
			for (j = 0; (j < treeLevelLimit) && (ch != eolFTA); j++)
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
				break;
			}
			else
			{
				folderTreeArray[i][j][k] = (wchar_t)ch;
			}
			}

			trackFTA [i][0] = j; //track the nesting level for validation
			if (ch == eolFTA) break;
			if (j != 0)
				{
					wcscpy_s(tempDestOld, pathLength, pathsToSave[i]);
					wcscat_s(pathsToSave[i], pathLength, &separatorFTA); //tacking them back on: what a waste doing it this way
				}
			else
			{
				wcscpy_s(tempDestOld, pathLength, driveIDBaseWNT);
			}
			wcscat_s(pathsToSave[i], pathLength, folderTreeArray[i][j]);



			if (foundNTDLL && !appendMode)   //only verify entire FS before delete
				{
					wcscpy_s(tempDest, pathLength, driveIDBaseWNT);
					wcscat_s(tempDest, pathLength, pathsToSave[i]);
					if (DynamicLoader (false, tempDest))
					{
						ntStatus = foundNTDLL (&hdlNTOut, FILE_LIST_DIRECTORY | FILE_READ_ATTRIBUTES, &fileObject, &ioStatus, nullptr, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_REPARSE_POINT, nullptr, 0);

						PFN_RtlNtStatusToDosError RtlNtStatusToDosError;
						if( !(RtlNtStatusToDosError = (PFN_RtlNtStatusToDosError) GetProcAddress( (HMODULE)hdlNtCreateFile, NtStatusToDosErrorString )) ) 
						{
							ErrorExit (L"RtlNtStatusToDosError: Problem!", 0);
							fsReturn = false;
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
									//GetFileAttributesW is good to go else it's FltCancellableWaitForSingleObject
									if (GetFileAttributesW(tempDest) == INVALID_FILE_ATTRIBUTES) //The status_wait_one is always reached after creation
									{
										//(Status is often invalid parm 0X00000057 from prevoious call)
										trackFTA [i][0] -=1; //Rollback
										wcscpy_s(pathsToSave[i], pathLength, tempDestOld);

										ErrorExit (L"Cannot verify a file entry in FS: Probably doesn't exist! recommend restarting the program ASAP: ", 1);
										folderTreeArray[i][j][0] = L'\0';

											
										//wind forward to EOL
										for (k = 0; ((k  < (pathLength)) && (ch!= eolFTA)); k++)
										{
											ch = fgetwc(stream);
											chOld = ch;
										}
										
									}
								}
							break;
							}
						}
						else
						{
							ErrorExit (L"DynamicLoader failed: Cannot verify. ", 1);
							fsReturn = false;
							goto WEOFFOUND;

						}
				if (!CloseNTDLLObjs(false)) DisplayError (hwnd, L"NtCreateFile: Objects failed to close ", errCode, 0);

				}//FoundNTdll




 			}

			

		branchTotalSaveFile = i;
		i += 1;
		} while ((i < branchLimit) && (ch != WEOF));

	} 


	WEOFFOUND:
	free (tempDestOld);
	if (foundNTDLL && !appendMode && !falseReadtrueWrite) //cleanup
	{
		if (!CloseNTDLLObjs(false)) DisplayError (hwnd, L"NtCreateFile: Objects failed to close...", errCode, 0);
	}
		// Close stream if it is not NULL 

	if (fclose (stream))
	{
	ErrorExit (L"Stream was not closed properly: exit & restart?", 0);
	fsReturn = false;
	}
	free (fsName);
	return fsReturn;

	}

void FSDeleteInit (HWND hwnd, HWND hList)
{
bool cmdlineParmtooLong = false;
tempDest = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
findPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t)); // only required for the old RecurseRemovePath
currPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
pathToDeleteW = (wchar_t *)calloc(pathLength, sizeof(wchar_t));

if ((tempDest == nullptr) || (findPathW == nullptr) || (currPathW == nullptr) || (pathToDeleteW == nullptr))
	{
		DisplayError (hwnd, L"Could not allocate required memory!", errCode, 0);
		goto RemoveKleenup;
	}


if (errCode != -100) errCode = -4;

if (pCmdLineActive)
{
	wchar_t * rootDirPtr = wcschr (rootDir, '\\');
	if (rootDirPtr)
	{
		
		for (i = 0; i < (int)(rootDirPtr - rootDir); i++) rootDirPtr[i] = rootDir[i];
		rootDirPtr[i] = L'\0';
		wcscpy_s(rootDir, pathLength, rootDirPtr);
		pCmdLineActive = false;
	}
	else
	{
	DisplayError (hwnd, L"Oops! CmdLine not there!", errCode, 0);
	free(pathToDeleteW);
	goto RemoveKleenup;
	}

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

		for (i = 0; i < rootFolderCS; i++)
		{
			mbstowcs(dacfoldersWtmp[i], dacfolders[i], maxPathFolder);
			//cumPath = dacfoldersW[i] + 4; // ////?// prefix away: 4 cuts out the //?/
			if (wcscmp(dacfoldersW[index-rootFolderCW] + 4, dacfoldersWtmp[i]) == 0)
			{
			if (!DisplayError (hwnd, L"This Directory has an \"ANSII\" equivalent. Remove won't work if it contains files. Continue?", errCode, 1))
				{
					free(pathToDeleteW);
					goto RemoveKleenup;
				}
			}
		}

	currPathW[0] = L'';
	wcscat_s(currPathW, maxPathFolder, dacfoldersW[index-rootFolderCW]);
	wchar_t * currPathWtmp;
	currPathWtmp = currPathW + 7;
	wcscpy_s(rootDir, pathLength, currPathWtmp);
	findPathW[0] = L'\0';
}


	if (foundNTDLL)
	{
		if (errCode > -100) 
		{
			if (!ProcessfileSystem(hwnd, false, false)) //Reads and verifies entire FS
			{
			if (!DisplayError (hwnd, L"No FS file! Cannot tell whether directory was created by this program. Try alternate delete?", 0, 1))
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


		for (i = branchTotal; (i >= 0); i--) //place paths to delete at end of FS
		{

			if (!wcscmp (rootDir, folderTreeArray[i][0])) //0 if perfect match
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
				if (DisplayError (hwnd, L"The selected folder is not found in the FS. Try alternate delete?", 0, 1))
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
			branchTotalCum = j + 1;
		}
		else
		{
			branchTotalCum = 0;
		}


			do
			{
			} while (FSDelete (hwnd));
			//Write remaining FS
			free(pathToDeleteW);
			if (!errCode) //errCode is still -4 if no match!
			{
			if (!pCmdLineActive) ((ProcessfileSystem(hwnd, true, false))? errCode = 1: errCode = 0);
			//this can bug out if the user edits or deletes the FS in the intervening milliseconds when called from InitProc.				
			goto RemoveKleenup;
			}

	}
	else
	{
		if (!DisplayError (hwnd, L"NTDLL not found on this machine. Continue with alternate delete?", 0, 1))
			{
				free(pathToDeleteW);
				goto RemoveKleenup;
			}
	}


OldDelete:

	
if (cmdlineParmtooLong)
	{
		errCode = 0;
		DisplayError (hwnd, L"Oops, command line too long! Delete won't work. Quit and try again should work.", 0, 0);
		goto RemoveKleenup;
	}
	wcscat_s(currPathW, maxPathFolder, &separatorFTA);
	wcscpy_s(folderTreeArray[0][0], maxPathFolder, currPathW);

	treeLevel = 0;
	trackFTA [0][0] = 0; //Initial conditions before search on path
	trackFTA [0][1] = 1;

	for (i = 1; i < branchLimit; i++)
	{
	trackFTA [i][0] = 0; //Initial conditons before search on path
	trackFTA [i][1] = 0;
	}
						
	if (!SetCurrentDirectoryW (driveIDBaseW))
	{
	ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
	goto RemoveKleenup;
	}
	if (RecurseRemovePath(trackFTA, folderTreeArray))
		{
			errCode = 0;
			DisplayError (hwnd, L"Remove failed.", 0, 0);
		}
	else
		{
			errCode = 1;
		}


					
	RemoveKleenup:
	if (findPathW) free (findPathW);
	if (currPathW) free (currPathW);
	rootDir[0] = L'\0';
	_CrtDumpMemoryLeaks();
		if (pCmdLineActive)
		{
			//ReleaseMutex (hMutex); //problematic??
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
		}
return;
}

bool FSDelete (HWND hwnd)
{	

if (branchTotal == branchTotalCum - 1) //branchTotal is decremented here, not branchTotalCum
	{
		branchTotal = branchTotalCum; // required for FS Write
		return false;
	}




	for (i = branchTotalCum; (i <= branchTotal); i++)
	{
		trackFTA [i][1] = trackFTA [i][0];
	}


	for (i = branchTotalCum + 1; (i <= branchTotal); i++)
		{ 
		int tmp = trackFTA [i][1]; //insertion sort
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
					if (trackFTA [i][1] == trackFTA [j][0]) return (fsDelsub (i, j, hwnd));
				}
			}
	}

return true; //shouldn't get here though
}
bool fsDelsub (int i, int j, HWND hwnd)
{

	for (k = (trackFTA [i][1]); (k >= ((i > branchTotalCum)? trackFTA [i - 1][1]: 0)); k--)
	{
	//do not iterate below trackFTA [i + 1][1]

	pathToDeleteW[0] = L''; // driveIDBaseWNT L"C:\\"

	wcscpy_s(pathToDeleteW, pathLength, driveIDBaseWNT);
	wcscat_s(pathToDeleteW, pathLength, pathsToSave[j]);

	if (RemoveDirectoryW (pathToDeleteW))
			{
				//clear everything
				pathsToSave[j][0] = L'';
				//we have only removed the last dir from pathsToSave so remove last dir from folderTreeArray
							

				folderTreeArray[j][k-1][0] = L'\0';

				trackFTA [i][1] -=1;
				trackFTA [j][0] -=1;


				if (trackFTA [i][1] != 0)
				{
				//rebuild pathsToSave
					for (int l = 0; (l < k - 1); l++) //extra loop adds the terminator
					{
						if (l != 0) wcscat_s(pathsToSave[j], pathLength, &separatorFTA);
						wcscat_s(pathsToSave[j], pathLength, folderTreeArray[j][l]);
					}

				}
				else
				{
				goto FSReorg;
				}
			
			}
		else
			{
				
				if (((int)GetLastError() == 32) ) //"used by another process" error
				{
					if (secondTryDelete)
					{
					DisplayError (hwnd, L"Delete failed the second time. Try running 7-zip and shift-del.", 0, 0);
					secondTryDelete = false;
					}
					else
					{
					DisplayError (hwnd, L"Oops, 'Used by another process': Something went wrong. Restarting to attempt deletion...", 0, 0);
					rootDir[0] = L'\0';
					pCmdLineActive = true;
					wcscpy_s(pathToDeleteW, pathLength, L" "); //http://forums.codeguru.com/showthread.php?213443-How-to-pass-command-line-arguments-when-using-CreateProcess
					wcscat_s(pathToDeleteW, pathLength, pathsToSave[j]);

					((ProcessfileSystem(hwnd, true, false))? errCode = 0: errCode = 1);
					Kleenup (hwnd, weareatBoot);
					}
					return false;

				}
				else
				{
					if (((int)GetLastError() == 2) || ((int)GetLastError() == 3)) //cannot find file or path specified
						{
						//The entry in pathsToSave must have a duplicate elsewhere: nuke the current one:
						pathsToSave[j][0] = L'\0';
								
						folderTreeArray[j][0][0] = L'\0';

						trackFTA [i][1] = 0;
						trackFTA [j][0] = 0;
						goto FSReorg;
						}
						if (((int)GetLastError() == 145))
						{
							if (folderNotEmpty)

							{
								DisplayError (hwnd, L"Cannot remove Folder: Retry or restart & retry.", 0, 0);
								folderNotEmpty = false;
							}
							else
							{
								if (DisplayError (hwnd, L"Delete error: folder is not empty. This can occur when a subfolder is deleted outside of this program. Continue deletion?", 0, 1)) folderNotEmpty = true;
							}
							return !folderNotEmpty;
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
	//k loop

return true;

FSReorg:
errCode = 0;

if (j != branchTotal)
	{
//Move everything down to fill slot
	for (k = j + 1; (k <= branchTotal); k++)
	{
		for (int l = 0; (l < trackFTA [k][0]); l++)
			{
			wcscpy_s(folderTreeArray[k-1][l], maxPathFolder, folderTreeArray[k][l]); 
			}	

		trackFTA [k-1][0] = trackFTA [k][0];
		wcscpy_s(pathsToSave[k-1], pathLength, pathsToSave[k]); 
	}
	}	
pathsToSave [branchTotal][0] = L'\0';
branchTotal -=1;
return true;
}
int RecurseRemovePath(int trackFTA[branchLimit][2], wchar_t folderTreeArray[branchLimit + 1][treeLevelLimit + 1][maxPathFolder])

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

					else
					{
						if (!SetCurrentDirectoryW (L".."))
						{
							ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
							return 1;
						}

						if (RemoveDirectoryW (currPathW))
						{

						if (RecurseRemovePath(trackFTA, folderTreeArray))
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
				findPathW[0] = L'';
				wcscat_s(findPathW, maxPathFolder, folderTreeArray[trackFTA[treeLevel][0]-1][treeLevel]);
				//
				if (!SetCurrentDirectoryW (findPathW))
				{
					ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
					return 1;
				}

				treeLevel +=1; // up next tree
					if (RecurseRemovePath(trackFTA, folderTreeArray))
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

				currPathW[0] = L'';
				wcscat_s(currPathW, maxPathFolder, dw.cFileName);
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
						if (!SetCurrentDirectoryW (driveIDBaseW)) //objects to L".."
						{
						ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
						return 1;
						}
					wchar_t * currPathWtmp = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
					currPathWtmp = currPathW + 4;
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
						if (RecurseRemovePath(trackFTA, folderTreeArray))
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

					if (RecurseRemovePath(trackFTA, folderTreeArray))
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
void ShellError (HWND aboutHwnd, int nError)
{
	if (nError > 32) return; //no problem
	wchar_t* str;
	switch (nError) 
	{
	case 0:							str = L"The operating system is out\nof memory or resources"; break;
	case ERROR_FILE_NOT_FOUND:		str = L"The specified path was not found"; break;
	case ERROR_PATH_NOT_FOUND:		str = L"The specified file was not found"; break;
	case ERROR_BAD_FORMAT:			str = L"The .EXE file is invalid\n(non-Win32 .EXE or error in .EXE image)"; break;
	case SE_ERR_ACCESSDENIED:		str = L"The operating system denied\naccess to the specified file"; break;
	case SE_ERR_ASSOCINCOMPLETE:	str = L"The filename association is\nincomplete or invalid"; break;
	case SE_ERR_DDEBUSY:			str = L"The DDE transaction could not\nbe completed because other DDE transactions\nwere being processed"; break;
	case SE_ERR_DDEFAIL:			str = L"The DDE transaction failed"; break;
	case SE_ERR_DDETIMEOUT:			str = L"The DDE transaction could not\nbe completed because the request timed out"; break;
	case SE_ERR_DLLNOTFOUND:		str = L"The specified dynamic-link library was not found"; break;
	case SE_ERR_NOASSOC:			str = L"There is no application associated\nwith the given filename extension"; break;
	case SE_ERR_OOM:				str = L"There was not enough memory to complete the operation"; break;
	case SE_ERR_SHARE:				str = L"A sharing violation occurred";
	default:						str = L"Unknown Error occurred"; break;
	}
	swprintf_s(hrtext, _countof(hrtext), L"Unable to open hyperlink:\n\n %s", str);
	DisplayError (aboutHwnd, hrtext, 0, 0);
 }
static void CreateHyperLink(HWND hwndControl)
{
    // Subclass the parent so we can color the controls as we desire.
    HWND hwndParent = GetParent(hwndControl);
    if (NULL != hwndParent)
    {
        WNDPROC pfnOrigProc = (WNDPROC)GetWindowLong(hwndParent, GWL_WNDPROC);
        if (pfnOrigProc != _HyperlinkParentProc)
        {
            SetProp(hwndParent, PROP_ORIGINAL_PROC, (HANDLE)pfnOrigProc);
            SetWindowLong(hwndParent, GWL_WNDPROC, (LONG)(WNDPROC)_HyperlinkParentProc);
        }
    }

    // Make sure the control will send notifications.
    DWORD dwStyle = GetWindowLong(hwndControl, GWL_STYLE);
    SetWindowLong(hwndControl, GWL_STYLE, dwStyle | SS_NOTIFY);

    // Subclass the existing control.
    WNDPROC pfnOrigProc = (WNDPROC)GetWindowLong(hwndControl, GWL_WNDPROC);
    SetProp(hwndControl, PROP_ORIGINAL_PROC, (HANDLE)pfnOrigProc);
    SetWindowLong(hwndControl, GWL_WNDPROC, (LONG)(WNDPROC)_HyperlinkProc);

    // Create an updated font by adding an underline.
    HFONT hOrigFont = (HFONT)SendMessage(hwndControl, WM_GETFONT, 0, 0);
    SetProp(hwndControl, PROP_ORIGINAL_FONT, (HANDLE)hOrigFont);

    LOGFONT lf;
    GetObject(hOrigFont, sizeof(lf), &lf);
    lf.lfUnderline = TRUE;

    HFONT hFont = CreateFontIndirect(&lf);
    SetProp(hwndControl, PROP_UNDERLINE_FONT, (HANDLE)hFont);

    // Set a flag on the control so we know what color it should be.
    SetProp(hwndControl, PROP_STATIC_HYPERLINK, (HANDLE)1);
}
LRESULT CALLBACK _HyperlinkParentProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WNDPROC pfnOrigProc = (WNDPROC)GetProp(hwnd, PROP_ORIGINAL_PROC);

    switch (message)
    {
    case WM_CTLCOLORSTATIC:
    {
        HDC hdc = (HDC)wParam;
        HWND hwndCtl = (HWND)lParam;

        BOOL fHyperlink = (NULL != GetProp(hwndCtl, PROP_STATIC_HYPERLINK));
        if (fHyperlink)
        {
            LRESULT lr = CallWindowProc(pfnOrigProc, hwnd, message, wParam, lParam);
            SetTextColor(hdc, RGB(0, 0, 192));
            return lr;
        }

        break;
    }
    case WM_DESTROY:
    {
        SetWindowLong(hwnd, GWL_WNDPROC, (LONG)pfnOrigProc);
        RemoveProp(hwnd, PROP_ORIGINAL_PROC);
        break;
    }
    }
    return CallWindowProc(pfnOrigProc, hwnd, message, wParam, lParam);
}

LRESULT CALLBACK _HyperlinkProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    WNDPROC pfnOrigProc = (WNDPROC)GetProp(hwnd, PROP_ORIGINAL_PROC);

    switch (message)
    {
    case WM_DESTROY:
    {
        SetWindowLong(hwnd, GWL_WNDPROC, (LONG)pfnOrigProc);
        RemoveProp(hwnd, PROP_ORIGINAL_PROC);

        HFONT hOrigFont = (HFONT)GetProp(hwnd, PROP_ORIGINAL_FONT);
        SendMessage(hwnd, WM_SETFONT, (WPARAM)hOrigFont, 0);
        RemoveProp(hwnd, PROP_ORIGINAL_FONT);

        HFONT hFont = (HFONT)GetProp(hwnd, PROP_UNDERLINE_FONT);
        DeleteObject(hFont);
        RemoveProp(hwnd, PROP_UNDERLINE_FONT);

        RemoveProp(hwnd, PROP_STATIC_HYPERLINK);

        break;
    }
    case WM_MOUSEMOVE:
    {
        if (GetCapture() != hwnd)
        {
            HFONT hFont = (HFONT)GetProp(hwnd, PROP_UNDERLINE_FONT);
            SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, FALSE);
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
                HFONT hFont = (HFONT)GetProp(hwnd, PROP_ORIGINAL_FONT);
                SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, FALSE);
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

    return CallWindowProc(pfnOrigProc, hwnd, message, wParam, lParam);
}