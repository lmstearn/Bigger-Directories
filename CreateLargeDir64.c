#include<stdlib.h> //malloc
#include<stdio.h> //sprintf
#include <windows.h>
#include <Strsafe.h> //safe string copy
#include "tlhelp32.h" //Find process stuff
#include "CreateLargeDir64.h" //my file
#include "Winternl.h" //NtCreateFile
//#include <ntstrsafe.h>

#pragma once 
//http://stackoverflow.com/questions/5896030/how-to-use-windows-tooltip-control-without-bounding-to-a-tool
//http://www.cprogramming.com/tutorial/printf-format-strings.html
//https://msdn.microsoft.com/en-us/library/windows/desktop/aa378137(v=vs.85).aspxy

//findHandle = FindFirstFile(@"\\?\UNC\" + folder_path, out findData


wchar_t hrtext[256]; //An array name is essentially a pointer to the first element in an array.
WIN32_FIND_DATAW dw; // directory data this will use stack memory as opposed to LPWIN32_FIND_DATA
WIN32_FIND_DATA da;
int const pathLength = 32760, maxPathFolder = MAX_PATH - 3;
wchar_t const *lpref = L"\\\\\?\\";
const wchar_t *driveIDBaseW = L"\\\\\?\\C:\\";
const char *driveIDBase = "C:\\";
wchar_t *currPathW, *findPathW, *tempDest, *thisexePath, *createlargedirVAR; // directory pointers. cannot be initialised as a pointer
char *currPath;
//http://stackoverflow.com/questions/2516096/fastest-way-to-zero-out-a-2d-array-in-c
char dacfolders[127][MAX_PATH-3]; //[32768 / 257] [ MAX_PATH- 3] double array char is triple array
wchar_t dacfoldersW[255][MAX_PATH-3], dacfoldersWtmp[127][maxPathFolder], folderTreeArray[2000][1000][maxPathFolder];


int folderdirCS, folderdirCW;
long long listTotal = 0;
HANDLE hdlNtCreateFile, hdlNTOut, exeHandle, ds;     // directory handle
long long idata, treeLevel, trackFTA[1000][2];
long long index; //variable for listbox items
BOOL buttEnable = FALSE;
BOOL weareatBoot = FALSE;
BOOL setforDeletion = FALSE;
BOOL am64Bit, exe64Bit; 
PVOID OldValue = NULL; //Redirection
typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);



//NTcreatefile stuff
//typedef int (*NTDLLptr) (int); //Function pointer example, but following is required
typedef NTSTATUS (__stdcall *NTDLLptr)(
    PHANDLE FileHandle, 
    ACCESS_MASK DesiredAccess, 
    OBJECT_ATTRIBUTES *ObjectAttributes, 
    PIO_STATUS_BLOCK IoStatusBlock, 
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes, 
    ULONG ShareAccess, 
    ULONG CreateDisposition, 
    ULONG CreateOptions, 
    PVOID EaBuffer, 
    ULONG EaLength );

//unicode string for NTcreatefile fileObject
//typedef VOID (__stdcall *RtlInitUnicodeStringPtr) (
//    IN OUT PUNICODE_STRING  DestinationString,
//    IN wchar_t  *SourceString );

//typedef VOID ( NTAPI *my_RtlInitUnicodeString ) (
//    PUNICODE_STRING DestinationString,
//    PCWSTR SourceString
//    );
//unicode string for NTcreatefile fileObject


//for NTcreatefile fileObject
typedef VOID (__stdcall *my_RtlInitUnicodeString) (
    IN OUT PUNICODE_STRING  DestinationString,
    IN PCWSTR  SourceString );
//static my_RtlInitUnicodeString rtlInitUnicodeString; //Makes no difference

NTDLLptr foundNTDLL = NULL; //returns variable here
UNICODE_STRING fn;
OBJECT_ATTRIBUTES fileObject;
IO_STATUS_BLOCK ioStatus;
NTSTATUS status;
const char createFnString[13] = "NtCreateFile";
const char initUnicodeFnString[21] = "RtlInitUnicodeString";


//A pathname MUST be no more than 32, 760 characters in length. (ULONG) Each pathname component MUST be no more than 255 characters in length (USHORT)
//wchar_t longPathName=(char)0;  //same as '\0'
//The long directory name with 255 char "\" separator



//const size_t cchDest = MAX_PATH;
//#define arraysize cchDest;


//------------------------------------------------------------------------------------------------------------------
// Protos...
//------------------------------------------------------------------------------------------------------------------
char ***new2DArr(size_t rows, size_t cols); //2D array function we will probably never use.
int RecurseRemovePath(long long trackFTA[1000][2], wchar_t folderTreeArray[2000][1000][maxPathFolder]);
int GetCreateLargeDirPath (HWND hwnd, wchar_t *exePath, int errorcode);
DWORD FindProcessId(HWND hwnd, const wchar_t *processName, HANDLE hProcessName);
bool Kleenup (HWND hwnd, bool weareatBoot);
NTDLLptr DynamicLoader (HWND hwnd,  bool progInit);
int ExistRegValue ();

int DisplayError (HWND hwnd, LPCWSTR messageText, int errorcode, int yesNo)
{
		//*hrtext  (pointee) is value pointed to by hrtext. Can be replaced by hrtext[0]
		//hrtext[0] = (wchar_t)LocalAlloc(LPTR, 256*sizeof(wchar_t)); This dynamic allocation NOT required- see below
		//if (hrtext[0] == NULL) ErrorExit("LocalAlloc");
		//hrtext[0] = NULL;  or	//*hrtext = NULL; //simple enough nut not req'd

		if (errorcode ==0){
		swprintf_s(hrtext, _countof(hrtext), L"%s.", messageText);
		}
		else //LT 0 my defined error, GT error should be GET_LAST_ERROR
		{
		swprintf_s(hrtext, _countof(hrtext), L"%s. Error Code:  %d", messageText, errorcode);
		}
		//change countof sizeof otherwise possible buffer overflow: here index and folderdirCS gets set to -16843010!
		if (yesNo)
		{
		int msgboxID = MessageBoxW(hwnd, hrtext, L"Warning", MB_YESNO);
			if (msgboxID =IDYES)
			{
			return 1;
			}
			//else //IDNO
		}
		else
		{
		MessageBoxW(hwnd, hrtext, L"Warning", MB_OK);
		}

		return 0;
		//if ((HANDLE)*hrtext) LocalFree((HANDLE)*hrtext); // It is not safe to free memory allocated with GlobalAlloc. -MSDN	
		//wchar_t hrtext[256] allocates memory to the stack. It is not a dynamic allocation http://stackoverflow.com/questions/419022/char-x256-vs-char-malloc256sizeofchar
}

void ErrorExit(LPCTSTR lpszFunction)
{
	//courtesy https ://msdn.microsoft.com/en-us/library/windows/desktop/ms680582(v=vs.85).aspx
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dww = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dww,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	(LPTSTR)&lpMsgBuf,0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("%s failed with error %lu: %s"), lpszFunction, dww, lpMsgBuf);
	printf("\a");  //audible bell
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, "Error", MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	//ExitProcess(dw);
}


void PopulateList(HWND hwnd, int errorcode)
{
	TCHAR* pszTxt = TEXT("My very big nested path");
	TCHAR pszDest[arraysize];
	BOOL findhandle;
	int gotolooper = 0;
	errorcode = 0;

	size_t cbDest = arraysize * sizeof(TCHAR); //the use of size_t implies C++ compile.
	LPCTSTR pszFormat = TEXT("%s");
	HRESULT hr = StringCbPrintf(pszDest, cbDest, pszFormat, pszTxt); //swprintf_s
	
	
	//if (foundNTDLL) we can use the better function

	if (!DynamicLoader (hwnd, true)) DisplayError (hwnd, L"The long path function has been removed. Using short path functions...", errorcode, 0);

	
	#if defined(ENV64BIT) //#if is a directive: see header file
	{
    if (sizeof(void*) != 8)
    {
        	DisplayError (hwnd, L"ENV64BIT: Error: pointer should be 8 bytes. Exiting.", errorcode, 0);
			if (exeHandle != INVALID_HANDLE_VALUE) CloseHandle(exeHandle);
			exit(1);
    }
    am64Bit = true;
	exe64Bit = true;
	}
	#elif defined (ENV32BIT)
	{
		if (sizeof(void*) != 4)
		{
			DisplayError (hwnd, L"ENV32BIT: Error: pointer should be 4 bytes. Exiting.", errorcode, 0);
			if (exeHandle != INVALID_HANDLE_VALUE) CloseHandle(exeHandle);
			exit(1);
		}
    
		if (FindProcessId (hwnd, L"CreateLargeDir.exe", exeHandle) == NULL)
		{

			am64Bit = false;
			exe64Bit = false;
			LPFN_ISWOW64PROCESS fnIsWow64Process;

			fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandleW((L"kernel32")),"IsWow64Process");
			if(NULL != fnIsWow64Process)
				{
				exe64Bit = fnIsWow64Process(GetCurrentProcess(),&exe64Bit) && exe64Bit;
				}

		}

	else
	{
		//Weird, can't get our own handle
	}
}	

	#else
    //#error "user" gen error won't compile with current settings: "Must define either ENV32BIT or ENV64BIT". 128 bit?
	#endif



if (FindProcessId (hwnd, L"explorer.exe", exeHandle) == NULL)
	{
	weareatBoot=TRUE;
	EnableWindow(GetDlgItem(hwnd, IDC_LOGON), buttEnable);
	EnableWindow(GetDlgItem(hwnd, IDC_NOLOGON), buttEnable);
	}
else
	{
	if (!FindProcessId (hwnd, L"userinit.exe", exeHandle) == NULL)
	{
	DisplayError (hwnd, L"Userinit should have ended. Try rebooting before running this (or any other) program!", errorcode, 0);
	}
	//Now to check if I am 64 bit


	createlargedirVAR= (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
	if (!ExpandEnvironmentStringsW (L"%SystemRoot%", createlargedirVAR, maxPathFolder)) ErrorExit("ExpandEnvironmentStringsW failed for some reason.");
	wcscat_s(createlargedirVAR, maxPathFolder, L"\\Temp\\CreateLargeDir.exe");

		if (GetFileAttributesW(createlargedirVAR)!=INVALID_FILE_ATTRIBUTES)
		{
			EnableWindow(GetDlgItem(hwnd, IDC_LOGON), buttEnable);
		}
	free(createlargedirVAR);
	}
	
	if (ExistRegValue() == 1)
	{
		setforDeletion = TRUE;
	}




	//NULL is a macro that's guaranteed to expand to a null pointer constant.
	//C strings are NUL-terminated, not NULL-terminated.  (char)(0) is the NUL character, (void * )(0) is	NULL, type void * , is called a null pointer constant
	//If (NULL == 0) isn't true you're not using C.  "\0' is the same as '0' see https://msdn.microsoft.com/en-us/library/h21280bw.aspx but '0' does not work!
	//http://stackoverflow.com/questions/15610506/can-the-null-character-be-used-to-represent-the-zero-character  NO
	memset(dacfolders, '\0', sizeof(dacfolders));  //'\0' is NULL L'\0' is for C++ but we are compiling in Unicode anyway
	memset(dacfoldersW, '\0', sizeof(dacfoldersW));
	memset(folderTreeArray, '\0', sizeof(folderTreeArray)); //required for remove function
	
	//Bad:
	//malloc(sizeof(char *) * 5) // Will allocate 20 or 40 bytes depending on 32 63 bit system
	//Good:
	// malloc(sizeof(char) * 5) // Will allocate 5 bytes

	currPath = (char *)calloc(maxPathFolder, sizeof(char));
	currPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
	if (currPathW == NULL)
	{
		/* We were not so display a message */
		errorcode = -1;
		DisplayError (hwnd, L"Could not allocate required memory", errorcode, 0);
		return;
	}
	if (currPath == NULL)
	{
		/* We were not so display a message */
		errorcode = -1;
		DisplayError (hwnd, L"Could not allocate required memory", errorcode, 0);
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
	SetDlgItemInt(hwnd, IDC_NUMBER, 5, FALSE);
	

gotoloop: //Wide Char loop
		  //Populate List
	findhandle = TRUE;
		
	if (gotolooper == 0)
	{
		memset(&da, 0, sizeof(WIN32_FIND_DATA));
		folderdirCS = 0;
		strcpy_s(currPath, maxPathFolder, driveIDBase);

		strcat_s(currPath, maxPathFolder, "*");
		ds = FindFirstFile(currPath, &da);

	}
	else
	{
		memset(&dw, 0, sizeof(WIN32_FIND_DATAW));
		folderdirCW = 0;
		//mbstowcs (currPathWW, currPathW,  sizeof(WIN32_FIND_DATA));
		//strcpy_s (driveIDBaseW, MAX_PATH-3, "\\\\?\\C:\\");
		//wcscpy_s(driveIDBaseW, maxPathFolder, L"\\\\\?\\C:\\"); //http://stackoverflow.com/questions/32540779/wcscpy-does-not-accept-tchar-in-destination-variable
		wcscpy_s(currPathW, maxPathFolder, driveIDBaseW);

		wcscat_s(currPathW, maxPathFolder, L"*");
		ds = FindFirstFileW(currPathW, &dw); //dw points to found folders

	}


	//The plain versions without the underscore affect the character set the Windows header files treat as default. So if you define UNICODE, then GetWindowText will map to GetWindowTextW instead of GetWindowTextA, for example. Similarly, the TEXT macro will map to L"..." instead of "...". 
	//The versions with the underscore affect the character set the C runtime header files treat as default. So if you define _UNICODE, then _tcslen will map to wcslen instead of strlen, for example. Similarly, the _TEXT macro will map to L"..." instead of "...". 



	if (ds == INVALID_HANDLE_VALUE && (gotolooper == 0))
	{
		//StringCchPrintf (pszDest, cbDest, TEXT("No directories found."));
		errorcode = -3;
		DisplayError (hwnd, L"No directories found", errorcode, 0);
		goto CLEANUP;
	}

	//Main loop
	while (ds != INVALID_HANDLE_VALUE && findhandle)
	{


			if (gotolooper == 0)
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


					strcat_s(dacfolders[folderdirCS], maxPathFolder, currPath);
					folderdirCS += 1;

					SendDlgItemMessage(hwnd, IDC_LIST, LB_ADDSTRING, (WPARAM)folderdirCS + folderdirCW, (LPARAM)currPath); // wparam cannot exceed 32,767 
					
				}
				findhandle = FindNextFile(ds, &da);
			}
			else
			{
				if ((dw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(dw.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM
					|| dw.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT || dw.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE || dw.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {

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

				wcscpy_s(currPathW, maxPathFolder, (wchar_t *)driveIDBaseW);
				wcscat_s(currPathW, maxPathFolder, dw.cFileName);
			
				
				//compare with dacfolders[folderdirC] to check for dups
				wcscat_s(dacfoldersW[folderdirCW], maxPathFolder, currPathW);
				folderdirCW += 1;
				SendDlgItemMessageW(hwnd, IDC_LIST, LB_ADDSTRING, (WPARAM)folderdirCS + folderdirCW, (LPARAM)currPathW); // wparam cannot exceed 32,767 
				
				}
				findhandle = FindNextFileW(ds, &dw);

		}
			SendDlgItemMessageW(hwnd, IDC_LIST, LB_SETITEMDATA, (WPARAM)folderdirCS + folderdirCW, (LPARAM)(folderdirCS + folderdirCW)); //(lparam)folderdirC required for getitemdata
																																		 //The Notification Code is passed as the HIWORD of wParam, the other half of the parameter that gave us the index of the control identifier in the first place. 
																																		 //HIWORD is the Upper 16 bits of UINT and LOWORD is the Lower 16 bits of UINT
																																		 //DWORD is just a typedef for 32-bit integer, whereas WORD is a typedef for a 16-bit integer


	}




	//if (folderdirC == 0) WHAAAAT you have no folders?

	if (gotolooper == 0)
	{
		gotolooper = 1;
		goto gotoloop;
	}

CLEANUP:
	//http://stackoverflow.com/questions/1912325/checking-for-null-before-calling-free
	if (currPath) free(currPath); //We may need these later though
	if (currPathW) free(currPathW); //Free from the heap We may need these later though
	//There's an internal index that is reset to 0 each time you call FindFirstFile() and it's incremented each time you call FindNextFile() so unless you do it in a loop, you'll only get the first filename ( a dot ) each time. 	
	FindClose(ds);


}


INT_PTR CALLBACK DlgProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	int errorcode =0;
		switch(Msg)
	{
		case WM_INITDIALOG:
			
            {	
				PopulateList (hwnd, errorcode);
				switch (errorcode)
				{
				case 1:
				{
					/* And exit */
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
            }
		break;
		case  IDC_TEXT:
		{
		}
		//check and reject illegal chars : " /  \ | ? *
		case IDC_NUMBER:
		{
			//no greater than 32760
		}

				case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case IDC_ADD: //adds directories nested ntimes
				{
					//http://www.experts-exchange.com/Programming/Languages/.NET/Visual_CPP/Q_27207428.html
					//On the first call of IDC_ADD change text & button enables. 
					//Add button disabled until a successful create or Clear
					//FIX LATER
					SetDlgItemTextW(hwnd,IDC_STATIC_ONE, L"This entry is repeated");

					SetDlgItemTextW(hwnd,IDC_STATIC_TWO, L"times");





					// When somebody clicks the Add button, first we get the number of
					// they entered

					BOOL bSuccess;
					int nTimes = GetDlgItemInt(hwnd, IDC_NUMBER, &bSuccess, FALSE);
					if(bSuccess) 
					{
						// Then we get the string they entered
						// First we need to find out how long it is so that we can
						// allocate some memory (2* +1 for two words > long)

						int len = 2 * (GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT)) +1);
						if(len > 0)
						{
							// Now we allocate, and get the string into our buffer

							int i;
							wchar_t * buf;

							buf = (wchar_t*)GlobalAlloc(GPTR, len );
							GetDlgItemTextW(hwnd, IDC_TEXT, buf, len);

							// Now we add the string to the list box however many times
							// the user asked us to.

							for(i = 0;i < nTimes; i++)
							{
								index = SendDlgItemMessageW(hwnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)buf);

								// Here we are associating the value nTimes with the item 
								// just for the heck of it, we'll use it to display later.
								// Normally you would put some more useful data here, such
								// as a pointer.
								SendDlgItemMessageW(hwnd, IDC_LIST, LB_SETITEMDATA, (WPARAM)index, (LPARAM)nTimes);
							}

							// Dont' forget to free the memory!
							GlobalFree((HANDLE)buf);
						}
						else 
						{
							errorcode = 0;
							DisplayError (hwnd, L"You didn't enter anything!", errorcode, 0);
						}
					}
					else 
					{
						errorcode = 0;
						DisplayError (hwnd, L"Couldn't translate that number :(", errorcode, 0);

					}

				}
				break;
				case IDC_REMOVE:
				{
					// When the user clicks the Remove button, we first get the number
					// of selected items

					HWND hList = GetDlgItem(hwnd, IDC_LIST);
					long long count = SendMessageW(hList, LB_GETSELCOUNT, 0, 0);
					if (count != LB_ERR)
					{
						
						long long err = SendMessageW(hList, LB_GETCURSEL, (WPARAM)1, (LPARAM)&index); //GETSELITEMS substituted with LB_GETCURSEL for a laugh

						index = SendMessageW(hList, LB_GETCURSEL, 0, 0L);
						if (count == 1 && (index < (folderdirCS + folderdirCW)))
						{
							

							memset(dacfoldersWtmp, '\0', sizeof(dacfoldersWtmp)); //sizeof(dacfoldersW) ok
							//SHFileOperationW(_Inout_ LPSHFILEOPSTRUCT lpFileOp);
							errorcode = 0;
							for (int i = 0; i < folderdirCS; i++)
							{

								mbstowcs(dacfoldersWtmp[i], dacfolders[i], maxPathFolder);
								//cumPath = dacfoldersW[i] + 4; // ////?// prefix away: 4 cuts out the //?/
								if (wcscmp(dacfoldersW[index-folderdirCW] + 4, dacfoldersWtmp[i]) == 0)
								{
								if (!DisplayError (hwnd, L"This Directory has an \"ANSII\" equivalent. Remove won't work if it contains files. Continue?", errorcode, 1)) goto RemoveKleenup;

								//for (int i = folderdirCS + folderdirCW; i < listTotal; i++)

								errorcode = -4;
								
								}
							}
							
							if (errorcode ==0)
							{
								if (!DisplayError (hwnd, L"This directory might have been created by this program. Continue?", errorcode, 1)) goto RemoveKleenup;

							}


						findPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
						currPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
						wcscat_s(currPathW, maxPathFolder, dacfoldersW[index-folderdirCW]);
						wcscat_s(currPathW, maxPathFolder, L"\\");
						wcscpy_s(folderTreeArray[0][0], maxPathFolder, currPathW);

						treeLevel = 0;
						trackFTA [0][0] = 0; //Initial conditions before search on path
						trackFTA [0][1] = 1;

						for (int j = 1; j < 1000; j++)
						{
						trackFTA [j][0] = 0; //Initial conditons before search on path
						trackFTA [j][1] = 0;
						}
						SetCurrentDirectory (driveIDBase);
						if (RecurseRemovePath(trackFTA, folderTreeArray)) DisplayError (hwnd, L"Remove failed.", errorcode, 0);
				
					
					RemoveKleenup:
					free (findPathW);
					free (currPathW);
					//longPathName
					//Clear all the added items

					if (errorcode != 0) //succeeded
					{
					errorcode = 0; //flag okay now
					SendDlgItemMessageW(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
					PopulateList(hwnd, errorcode);
					}








						}
						else

						{
							if (count != 0)
							{

								// And then allocate room to store the list of selected items.


								long long i;
								long long *buf = (long long*)GlobalAlloc(GPTR, sizeof(long long) * count);
								SendMessageW(hList, LB_GETSELITEMS, (WPARAM)count, (LPARAM)buf);

								// Now we loop through the list and remove each item that was
								// selected.  

								// WARNING!!!  
								// We loop backwards, because if we removed items
								// from top to bottom, it would change the indexes of the other
								// items!!!

								for (i = count - 1; i >= 0; i--)
								{
									SendMessageW(hList, LB_DELETESTRING, (WPARAM)buf[i], 0);
								}

								GlobalFree(buf);
							}
							else
							{
								errorcode = 0;
								DisplayError (hwnd, L"No items selected...", errorcode, 0);
							}
						}
					}
					else
					{
						errorcode = 0;
						DisplayError (hwnd, L"Error counting items...", errorcode, 0);
					}
			}
			break;
			case IDC_CLEAR:
				{
				SendDlgItemMessageW(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
				PopulateList(hwnd, errorcode);
				}
			break;
			case IDC_CREATE:
				{
				currPathW = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
					if (currPathW == NULL)
					{
						/* We were not so display a message */
					errorcode = -1;
					DisplayError (hwnd, L"Could not allocate required memory", errorcode, 0);
					//return;
					}
				tempDest = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
					if (tempDest == NULL)
					{
						/* We were not so display a message */
					errorcode = -1;
					DisplayError (hwnd, L"Could not allocate required memory", errorcode, 0);
					//return;
					}
					
				//cumPath = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
				//if (cumPath == NULL)
				//{
				/* We were not so display a message */
				//	errorcode = -1;
				//	DisplayError (hwnd, L"Could not allocate required memory", errorcode, 0);
				//return;
				//}
					
				HWND hList = GetDlgItem(hwnd, IDC_LIST);
				//get total for loop
				listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);

					if (listTotal == folderdirCS + folderdirCW)
					{
					errorcode = 0;
					DisplayError (hwnd, L"You didn't Add any strings to create!", errorcode, 0);
					goto EndCreate;
					}

				//wcscpy_s(cumPath, pathLength, L"\\\\\?\\C:\\");
				wcscpy_s(currPathW, maxPathFolder, L"");
				SetCurrentDirectoryW(L"\\\\\?\\C:\\");
				//Another loop & variables for recursive create here
				for (int i = folderdirCS + folderdirCW; i < listTotal; i++)
					{
					SendMessageW(hList, LB_GETTEXT, i, (LPARAM)currPathW);
					//check for double click https://msdn.microsoft.com/en-us/library/windows/desktop/bb775153(v=vs.85).aspx 


					// cannot use cumPath: http://stackoverflow.com/questions/33018732/could-not-find-path-specified-createdirectoryw/33050214#33050214
					//wcscat_s(cumPath, pathLength, currPathW);
  
					if (foundNTDLL)
					{
						wcscat_s(tempDest, maxPathFolder, driveIDBaseW);
						wcscat_s(tempDest, maxPathFolder, currPathW);

						if (DynamicLoader (hwnd,  false))
						{

						NTSTATUS ntStatus;
						//Do not specify FILE_READ_DATA, FILE_WRITE_DATA, FILE_APPEND_DATA, or FILE_EXECUTE 
						ntStatus = foundNTDLL (&hdlNTOut, FILE_LIST_DIRECTORY | FILE_TRAVERSE, &fileObject, &ioStatus, NULL, FILE_ATTRIBUTE_NORMAL, 0, FILE_CREATE, FILE_DIRECTORY_FILE, NULL, 0);

						

						if (NT_ERROR(ntStatus))
						{
						}

						if (!NT_SUCCESS(ntStatus))
						{
							switch(ioStatus.Information)
							{
							case FILE_EXISTS:
							{
							DisplayError (hwnd, L"IoStatus.Information is DIR_EXISTS", errorcode, 0);
							}
							break;
							case FILE_OPENED:
							{
							DisplayError (hwnd, L"IoStatus.Information is DIR_OPENED", errorcode, 0);
							}
							break;
							case FILE_DOES_NOT_EXIST:
							{
							DisplayError (hwnd, L"IoStatus.Information is DIR_DOES_NOT_EXIST", errorcode, 0);
							}
							break;
							default:
							{
								ErrorExit("NtCreateFile: Failed!");
							}
							}
						}
						}
						else
						{
							//DynamicLoader failed
						}
					} //foundNtdll
					else
						{
							wcscat_s(currPathW, maxPathFolder, L"\\");
							if (exe64Bit)
							{
							errorcode = CreateDirectoryW(currPathW, NULL);
							}
							else
							{
								if (Wow64DisableWow64FsRedirection(&OldValue)) errorcode = CreateDirectoryW(currPathW, NULL);
								if (!Wow64RevertWow64FsRedirection(&OldValue))
								{
								DisplayError (hwnd, L"Problems with redirection...", errorcode, 0);
								break;
								}

							}
						
						}
					}

				//Also check if Directory exists?
				//There is a default string size limit for paths of 248 characters
				//errorcode = CreateDirectoryW(cumPath, NULL);

				//wcscpy_s(currPathW, maxPathFolder, driveIDBaseW);
				//\a  audible bell

				//LB_GETTEXTLEN  https://msdn.microsoft.com/en-us/library/windows/desktop/bb761315(v=vs.85).aspx
					
					

				//longPathName
				//Clear all the added items
				EndCreate:
				free(currPathW);
				free(tempDest);
				if (foundNTDLL) FreeLibrary ((HMODULE)hdlNtCreateFile);
				//free(cumPath);
				if (errorcode != 0) //succeeded
				{
				errorcode = 0; //flag okay now
				SendDlgItemMessageW(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
				PopulateList(hwnd, errorcode);
				}
				}
				break;

				case IDC_LOGON:
			{
					//system ("CD\\ & PUSHD %SystemRoot%\\Temp & SET KEY_NAME=\"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" & SET \"VALUE_NAME=Userinit\" & REG QUERY \"HKLM\\Hardware\\Description\\System\\CentralProcessor\\0\" | FIND /i \"x86\" >NUL && CALL SET \"OSB=\" || CALL SET \"OSB=64BIT\" & (IF DEFINED OSB (FOR /F \"USEBACKQ SKIP=2 TOKENS=1-4 DELIMS= \" %A IN (`REG QUERY %KEY_NAME% /v %VALUE_NAME% /reg:64 2^>Userinitregerror.txt`) DO @SET \"CURREGVALUE=%C%D\") ELSE ((FOR /F \"USEBACKQ SKIP=2 TOKENS=1-4 DELIMS= \" %A IN (`REG QUERY %KEY_NAME% /v %VALUE_NAME% 2^>Userinitregerror.txt`) DO @SET \"CURREGVALUE=%C%D\"))) & >NUL FINDSTR \"^\" \"Userinitregerror.txt\" && SET \"ERRTXT=\" || SET \"ERRTXT=1\" & (IF DEFINED ERRTXT (>Userinitreg.txt CALL ECHO %CURREGVALUE% & (IF '%errorlevel%' NEQ '0' (CALL ECHO Copy reg record failed! & PAUSE >NUL))) ELSE (ECHO No reg key! & PAUSE NUL)) & (SET \"P= \"  & CALL CALL SET \"NEWREGVALUE=%CURREGVALUE:%P%%SystemRoot%\\system32\\userinit.exe=%SystemRoot%\\Temp\\CreateLargeDir.exe,%SystemRoot%\\system32\\userinit.exe%\") & CALL REG ADD %KEY_NAME% /v %VALUE_NAME% /d %NEWREGVALUE% & CALL ECHO %KEY_NAME% %VALUE_NAME% %CURREGVALUE% %NEWREGVALUE% & POPD & PAUSE >NUL");
					//Create backup reg text to temp
					//system ("md /D /E:ON /V:ON /C \"@echo on&pushd \"C:\\Windows\\Temp\"&SET \"KEY_NAME=HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\"&SET VALUE_NAME=\"Userinit\"&(FOR /F \"USEBACKQ SKIP=2 TOKENS=1-4\" %A IN (`REG QUERY \"!KEY_NAME!\" /v \"!VALUE_NAME\" 2^>NUL`) DO (SET \"VALUE_NAME=%A\"&SET \"VALUE_TYPE=%B\"&SET \"CURREGVALUE=%C %D\"))&(IF DEFINED VALUE_NAME (>Userinitreg.txt ECHO !CURREGVALUE!&PAUSE >NUL&(IF \"!errorlevel!\" NEQ \"0\" (ECHO Copy reg record failed.&PAUSE >NUL))) ELSE (ECHO \"!KEY_NAME!\" \"!VALUE_NAME%!\" not found.&PAUSE >NUL))&POPD&PAUSE >NUL"); PENPEN
					thisexePath = (wchar_t *)calloc( maxPathFolder, sizeof(wchar_t));
					tempDest = (wchar_t *)calloc( maxPathFolder, sizeof(wchar_t));
					ExpandEnvironmentStringsW(L"%systemroot%", tempDest, maxPathFolder);
					wcscat_s(tempDest,  maxPathFolder, L"\\Temp\\CreateLargeDir.exe");
					if (GetCreateLargeDirPath (hwnd, thisexePath, errorcode) == 1)
					{
							ErrorExit("GetCreateLargeDirPath: Problem with program copy.");
							break;
					}


					if (CopyFileW(thisexePath, tempDest, FALSE) == 0)
					{
						ErrorExit("CopyFile: Copy to Temp failed... aborting.");
						EnableWindow(GetDlgItem(hwnd, IDC_LOGON), buttEnable);
						EnableWindow(GetDlgItem(hwnd, IDC_NOLOGON), buttEnable);
						free (thisexePath);
						free (tempDest);
						break;
					}


			system ("CD\\ & PUSHD %SystemRoot%\\Temp & SET KEY_NAME=\"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" & SET \"VALUE_NAME=Userinit\" & REG QUERY \"HKLM\\Hardware\\Description\\System\\CentralProcessor\\0\" | FIND /i \"x86\" >NUL && CALL SET \"OSB=\" || CALL SET \"OSB=64BIT\" & (IF DEFINED OSB (FOR /F \"USEBACKQ SKIP=2 TOKENS=1-4 DELIMS= \" %G IN (`REG QUERY %KEY_NAME% /v %VALUE_NAME% /reg:64 2^>Userinitregerror.txt`) DO @SET \"CURREGVALUE=%I%J\") ELSE ((FOR /F \"USEBACKQ SKIP=2 TOKENS=1-4 DELIMS= \" %G IN (`REG QUERY %KEY_NAME% /v %VALUE_NAME% 2^>Userinitregerror.txt`) DO @SET \"CURREGVALUE=%I%J\"))) & >NUL FINDSTR \"^\" \"Userinitregerror.txt\" && SET \"ERRTXT=\" || SET \"ERRTXT=1\" & (IF DEFINED ERRTXT (>Userinitreg.txt CALL ECHO %CURREGVALUE% & (IF '%errorlevel%' NEQ '0' (CALL ECHO Copy reg record failed! & PAUSE >NUL))) ELSE (ECHO No reg key! & PAUSE NUL)) & (IF DEFINED OSB (CALL CALL SET \"NEWREGVALUE=%SystemRoot%\\Temp\\CreateLargeDir.exe,\") ELSE (CALL CALL SET \"NEWREGVALUE=%CURREGVALUE:%SystemRoot%\\system32\\userinit.exe=%SystemRoot%\\Temp\\CreateLargeDir.exe,%SystemRoot%\\system32\\userinit.exe%\")) & CALL REG ADD %KEY_NAME% /v %VALUE_NAME% /d %NEWREGVALUE% /f /reg:64 & POPD");
			//system ("CD\\ & PUSHD %SystemRoot%\\Temp & SET KEY_NAME=\"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" & SET \"VALUE_NAME=Userinit\" & REG QUERY \"HKLM\\Hardware\\Description\\System\\CentralProcessor\\0\" | FIND /i \"x86\" >NUL && CALL SET \"OSB=\" || CALL SET \"OSB=64BIT\" & (IF DEFINED OSB (FOR /F \"USEBACKQ SKIP=2 TOKENS=1-4 DELIMS= \" %G IN (`REG QUERY %KEY_NAME% /v %VALUE_NAME% /reg:64 2^>Userinitregerror.txt`) DO @SET \"CURREGVALUE=%I%J\") ELSE ((FOR /F \"USEBACKQ SKIP=2 TOKENS=1-4 DELIMS= \" %G IN (`REG QUERY %KEY_NAME% /v %VALUE_NAME% 2^>Userinitregerror.txt`) DO @SET \"CURREGVALUE=%I%J\"))) & >NUL FINDSTR \"^\" \"Userinitregerror.txt\" && SET \"ERRTXT=\" || SET \"ERRTXT=1\" & (IF DEFINED ERRTXT (>Userinitreg.txt CALL ECHO %CURREGVALUE% & (IF '%errorlevel%' NEQ '0' (CALL ECHO Copy reg record failed! & PAUSE >NUL))) ELSE (ECHO No reg key! & PAUSE NUL)) & CALL CALL SET \"NEWREGVALUE=%CURREGVALUE:%SystemRoot%\\system32\\userinit.exe=%SystemRoot%\\Temp\\CreateLargeDir.exe,%SystemRoot%\\system32\\userinit.exe%\" & CALL REG ADD %KEY_NAME% /v %VALUE_NAME% /d %NEWREGVALUE% /f /reg:64 & POPD & pause >nul");
					
			free (thisexePath);
			free (tempDest);
			EnableWindow(GetDlgItem(hwnd, IDC_LOGON), buttEnable);

			//NOTE WOW6432node is 64bit view of 32bit setting. reg:64 bypasses VS 32bit redirection
			//Debug & CALL ECHO %KEY_NAME% %VALUE_NAME% %CURREGVALUE% %NEWREGVALUE%
			}
			break;
		
				case IDC_NOLOGON:
			{
					//hFind = FindFirstFile("%systemroot%\Temp\Userinitreg.txt", &FindFileData);
					//https://msdn.microsoft.com/en-us/library/windows/desktop/aa365743(v=vs.85).aspx
					//If you are writing a 32-bit application to list all the files in a directory and the application may be run
					//on a 64-bit computer, you should call the Wow64DisableWow64FsRedirectionfunction before calling FindFirstFile
					//and call Wow64RevertWow64FsRedirection after the last call to FindNextFile.
			
			if (setforDeletion==TRUE)
			{
			if (!DisplayError (hwnd, L"The PendingFileRenameOperations key already has data. Please reply no and check they key's value if unsure whether another program besides this one has marked another file for deletion at reboot.", errorcode, 1)) break;
			
			//delete the key ExistRegValue
			system ("REG DELETE \"HKLM\\System\\CurrentControlSet\\Control\\Session Manager\" /v PendingFileRenameOperations /f");

			}
			
			if (Kleenup (hwnd, weareatBoot))
			{
				EnableWindow(GetDlgItem(hwnd, IDC_NOLOGON), buttEnable);
				buttEnable = TRUE;
				EnableWindow(GetDlgItem(hwnd, IDC_LOGON), buttEnable);
				buttEnable = FALSE;
				setforDeletion = TRUE;
			}
			else
			{
				setforDeletion = FALSE;
			}

			//

			

			}
			break;


				case IDC_LIST:

					switch(HIWORD(wParam))
					{
						case LBN_SELCHANGE:
						{
							// Get the number of items selected.

							HWND hList = GetDlgItem(hwnd, IDC_LIST);
							int count = SendMessageW(hList, LB_GETSELCOUNT, 0, 0);
							if(count != LB_ERR)
							{
								// We only want to continue if one and only one item is
								// selected.

								if(count == 1)
								{
									// Since we know ahead of time we're only getting one
									// index, there's no need to allocate an array.
									


									
									long long err = SendMessageW(hList, LB_GETCURSEL, (WPARAM)1, (LPARAM)&index); //GETSELITEMS substituted with LB_GETCURSEL for a laugh
									
									index = SendMessageW(hList, LB_GETCURSEL, 0, 0L);
								

									if (index >= folderdirCS + folderdirCW)
									{
										buttEnable = TRUE;
										SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Line\0");
										EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), buttEnable);
										buttEnable = FALSE;
									}
									else
									{

										SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Dir\0");

										if (wcsstr(dacfoldersW[index], lpref) == NULL)
											//Check for wide string folder here
										{
											buttEnable = TRUE;
											EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), buttEnable);
											buttEnable = FALSE;
										}
										else
										{
												//Cannot remove short folders
											EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), buttEnable);
										}
									}



									if(err != LB_ERR)
									{
										// Get the data we associated with the item above
										// (the number of times it was added)

										idata = SendMessageW(hList, LB_GETITEMDATA, (WPARAM)index, 0); //lparam not used, but return value IS value of lparam in setitemdata
										//SO idata becomes ntimes when items are added

										SetDlgItemInt(hwnd, IDC_SHOWCOUNT, idata, FALSE);
										//This function performs like:
										//TCHAR buf[16];
										//wnsprintf(buf, 16, bSigned ? TEXT("%i") : TEXT("%u"), uValue);
										//SetDlgItemText(hDlg, nIDDlgItem, buf);



									}
									else 
									{
										errorcode = 0;
										DisplayError (hwnd, L"Error getting selected item :(", errorcode, 0);
									}
								}
								else 
								{
									// No items selected, or more than one
									// Either way, we aren't going to process this.
									
									index = SendMessageW(hList, LB_GETANCHORINDEX, 0, 0L);
									
									if (index >= folderdirCS + folderdirCW)
									{
										buttEnable = TRUE;
										SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Line\0");
										EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), buttEnable);
										buttEnable = FALSE;
									}
									else //Don't care really, user must change selection: a tooltip?
									{
										SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Dir\0");
										EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), buttEnable);
									}


									SetDlgItemTextW(hwnd, IDC_SHOWCOUNT, L"-");
								}
							}
							else
							{
								errorcode = 0;
								DisplayError (hwnd, L"Error counting items :(", errorcode, 0);
							}
						}
						break;
					}
				break;
			}
		break;
		case WM_CLOSE:

			//Cleanup
			if (weareatBoot) Kleenup (hwnd, weareatBoot);
			 
			if (exeHandle != INVALID_HANDLE_VALUE) CloseHandle(exeHandle);
			EndDialog(hwnd, 0);
		break;
		default:
			return FALSE;
	}
	return TRUE;
}

BOOL DirectoryExists(LPCTSTR szPath) //StackOverflow 6218325
{
  DWORD dwAttrib = GetFileAttributes(szPath);

  return (dwAttrib != INVALID_FILE_ATTRIBUTES && 
         (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	return DialogBoxW(hInstance, MAKEINTRESOURCEW(IDD_MAIN), NULL, DlgProc);
}

char ***new2DArr(size_t rows, size_t cols)
{
  char ***newArr = NULL;
  size_t i;
  //newArr = malloc(sizeof *newArr * rows); //IntelliSense: a value of type "void *" cannot be assigned to an entity of type "char ***"

  if (newArr)
  {
    for (i = 0; i < rows; i++)
    {
      //newArr[i] = malloc(sizeof *newArr[i] * cols); //IntelliSense: a value of type "void *" cannot be assigned to an entity of type "char ***"
      if (newArr[i])
      {
        /* initialize or assign newArr[i][0] - newArr[i][cols-1] here */
      }
    }
  }
  return newArr;
}
int GetCreateLargeDirPath (HWND hwnd, wchar_t *exePath, int errorcode)
{
DWORD result;



    //  Disable redirection immediately prior to the native API
    //  function call.
    
if (exe64Bit)
{
		result  = GetModuleFileNameW(NULL, exePath, MAX_PATH-4);
	    errorcode = (int)GetLastError();

}
else
{
	if( Wow64DisableWow64FsRedirection(&OldValue) ) 
    {
	result  = GetModuleFileNameW(NULL, exePath, MAX_PATH-4);
    errorcode = (int)GetLastError();
	}
	else
	{
		DisplayError (hwnd, L"Problems with redirection...", errorcode, 0);
		return 1;
	}
    if (!Wow64RevertWow64FsRedirection(&OldValue) ) 
	{
		DisplayError (hwnd, L"Problems with redirection...", errorcode, 0);
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
        if (ERROR_INSUFFICIENT_BUFFER != errorcode)
        {
            return 1;
        }
    }


if (!exePath)
{
    switch (errorcode)
	{
		case 2:
			DisplayError (hwnd, L"Problems: FILE_NOT_FOUND", errorcode, 0);
		break;
		case 50:
			DisplayError (hwnd, L"Problems: NOT_SUPPORTED", errorcode, 0);
		break;
		case 110:
			DisplayError (hwnd, L"Problems: OPEN_FAILED", errorcode, 0);
		break;
		case 113:
			DisplayError (hwnd, L"Problems: NO_MORE_SEARCH_HANDLES", errorcode, 0);
		break;
		case 114:
			DisplayError (hwnd, L"Problems: INVALID_TARGET_HANDLE", errorcode, 0);
		break;
		case 123:
			DisplayError (hwnd, L"Problems: INVALID_NAME", errorcode, 0);
		break;
		case 126:
			DisplayError (hwnd, L"Problems: MODULE_NOT_FOUND", errorcode, 0);
		break;
		case 259:
			DisplayError (hwnd, L"Problems: NO_MORE_ITEMS", errorcode, 0);
		break;
		case 303:
			DisplayError (hwnd, L"Problems: DELETE_PENDING", errorcode, 0);
		break;
		case 310:
			DisplayError (hwnd, L"Problems: INVALID_EXCEPTION_HANDLER", errorcode, 0);
		break;
		case 335:
			DisplayError (hwnd, L"Problems: Cannot run this out of an archive: COMPRESSED_FILE_NOT_SUPPORTED", errorcode, 0);
		break;
		default:
			{
			DisplayError (hwnd, L"Unknown error has occurred.", errorcode, 0);
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
			system ("CD\\ & PUSHD %SystemRoot%\\Temp & REG QUERY \"HKLM\\Hardware\\Description\\System\\CentralProcessor\\0\" | FIND /i \"x86\" >NUL && CALL SET \"OSB=\" || CALL SET \"OSB=64BIT\" & SET KEY_NAME=\"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" & SET \"VALUE_NAME=Userinit\" & SET \"Userinitreg=\" & (IF EXIST Userinitreg.txt (SET \"Userinitreg=TRUE\")) & (IF Userinitreg == \"TRUE\" ( DEL \"Userinitreg.txt\" )) & (IF EXIST Userinitregerror.txt (DEL \"Userinitregerror.txt\")) & FOR /F \"usebackq tokens=1,2* delims=,\" %G IN (\"Userinitreg.txt\") DO SET \"REGVALUE=%G\" & (IF DEFINED OSB (CALL SET \"REGVALUE=%REGVALUE%,\" & CALL REG ADD %KEY_NAME% /v %VALUE_NAME% /d %REGVALUE% /f /reg:64) ELSE (CALL REG ADD %KEY_NAME% /v %VALUE_NAME% /d %REGVALUE% /f)) & POPD");

			thisexePath = (wchar_t *)calloc( maxPathFolder, sizeof(wchar_t));
			tempDest = (wchar_t *)calloc( maxPathFolder, sizeof(wchar_t));
			if (!ExpandEnvironmentStringsW(L"%systemroot%", tempDest,  maxPathFolder)) ErrorExit("ExpandEnvironmentStringsW failed for some reason.");
			wcscpy_s(thisexePath, maxPathFolder, tempDest); //Small hole in logic here
			wcscat_s(tempDest, maxPathFolder, L"\\Temp\\CreateLargeDir.exe");

			if (weareatBoot)
				{
				//reset to vanilla
				wcscat_s(thisexePath, maxPathFolder, L"\\system32\\userinit.exe");
				//Create process userinit.exe
				STARTUPINFOW lpStartupInfo;
				PROCESS_INFORMATION lpProcessInfo;

				ZeroMemory(&lpStartupInfo, sizeof(lpStartupInfo));
				ZeroMemory (&lpStartupInfo, sizeof(lpStartupInfo));

				SetLastError(ERROR_INVALID_PARAMETER); //https://msdn.microsoft.com/en-us/library/ms682425(VS.85).aspx
				if (!CreateProcessW(thisexePath, NULL, NULL, NULL, FALSE, NULL, NULL, NULL, &lpStartupInfo, &lpProcessInfo)) ErrorExit("userinit could not be started through this program. Please reboot after closing this program.");
				//The reg value is restored to userinit before theis point
				}
			if(!MoveFileExW(tempDest,NULL,MOVEFILE_DELAY_UNTIL_REBOOT))
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
			return true;
			}

}
int ExistRegValue ()
{
	wchar_t *keyName;
	wchar_t *valueName;
	keyName = (wchar_t *)calloc(260, sizeof(wchar_t));
	valueName = (wchar_t *)calloc(260, sizeof(wchar_t));
	wcscpy_s(keyName, 260, L"HKLM\\System\\CurrentControlSet\\Control\\Session Manager");
	wcscpy_s(valueName, 260, L"PendingFileRenameOperations");

	if (RegQueryValueExW((HKEY)keyName, valueName, NULL, NULL, NULL, NULL) == ERROR_FILE_NOT_FOUND)
	{
		free (keyName);
		free (valueName);
		return 0;
	}
	else
	{
		free (keyName);
		free (valueName);
		return 1;
	}

	// \??\C:\Windows\Temp\CreateLargeDir.exe: nasty multo_sz

}
DWORD FindProcessId(HWND hwnd, const wchar_t *processName, HANDLE hProcessName)
{
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    DWORD result = NULL;
	wchar_t procTemp [260];

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
		

			mbstowcs(procTemp, pe32.szExeFile, 260);

        if (0 == wcscmp(processName, (wchar_t*) procTemp))
        {
            result = pe32.th32ProcessID;
			hProcessName = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
			if(hProcessName == NULL )
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

NTDLLptr DynamicLoader (HWND hwnd,  bool progInit)
{
	HMODULE hdlNtCreateFile = LoadLibraryW(L"NtDll.dll");
	foundNTDLL = (NTDLLptr) GetProcAddress (hdlNtCreateFile, createFnString);
	if (foundNTDLL)
		{
			if (progInit)
			{
			memset(&ioStatus, 0, sizeof(ioStatus));
			memset(&fileObject, 0, sizeof(fileObject));
			fileObject.Length = sizeof(fileObject);
			fileObject.Attributes = OBJ_CASE_INSENSITIVE;
			}
			else
			{
			my_RtlInitUnicodeString RtlInitUnicodeString = (my_RtlInitUnicodeString) GetProcAddress(hdlNtCreateFile, initUnicodeFnString);
			RtlInitUnicodeString(&fn, tempDest);
			fileObject.ObjectName = &fn; //Ntdll.dll
			}
			return foundNTDLL;
		}
	else
		{
		FreeLibrary (hdlNtCreateFile);
		return foundNTDLL;
		}

}


int RecurseRemovePath(long long trackFTA[1000][2], wchar_t folderTreeArray[2000][1000][maxPathFolder])
	//*folderTreeArray[1000][1000][maxPathFolder] *(folderTreeArray)[260][maxPathFolder]
	 //first element of trackFTA is LAST_VISIT, second is number of folders found i.e. folderTreeArray[1000][jsize][maxPathFolder]
{
	int j;
		//(folderTreeArray[treeLevel][j] == NULL) is ALWAYS true

		//Now check remaining trackFTA minus j trees 
		//If no more i.e. (folderTreeArray[treeLevel][j + 1] doesn't exist

		
		//if (folderTreeArray[treeLevel][j+1] == NULL)
		//The last directory in the branch



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
			wcscpy_s(currPathW, maxPathFolder, folderTreeArray[treeLevel][trackFTA [treeLevel][0]-1]);
			if (!SetCurrentDirectoryW (L"..")) ErrorExit("SetCurrentDirectoryW: Non zero");
				
				if (RemoveDirectoryW (currPathW))
				{
					if (treeLevel == 0)
					{
						//everything above this folder is deleted
						return 0;
					}

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
					ErrorExit("RemoveDirectoryW: Cannot remove Folder. It may contain files.");
					return 1; //Need more than this
				}


			}

			else
				{
				//folderTreeArray[treeLevel][j+1]
				if (trackFTA[treeLevel][0] <= 999)
				{
				trackFTA[treeLevel][0] +=1;

				// set inits for this branch
				wcscpy_s(findPathW, maxPathFolder, L"\0");
				wcscat_s(findPathW, maxPathFolder, folderTreeArray[treeLevel][trackFTA[treeLevel][0]-1]);
				if (!SetCurrentDirectoryW (findPathW)) ErrorExit("SetCurrentDirectoryW: Non zero");

				GetCurrentDirectoryW(maxPathFolder, findPathW); //Get fulqualpath
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
				ErrorExit("Too many folders in the tree: If folder was created by this program, a warning should have been issued on folder creation.");
				return 1; 
				}
		}

	}

	else //search yet to be done on branch
	{



	//Do find folders in new branch, findPathW already set
			
					
		memset(&dw, 0, sizeof(WIN32_FIND_DATAW));
		//Find first file
		wcscat_s(findPathW, maxPathFolder, L"\\*");
		ds = FindFirstFileW(findPathW, &dw);
			if (ds == INVALID_HANDLE_VALUE) //redundant as first 2 pickups are "." and ..
		{
			// No Folders so this must be top level
			FindClose(ds);
			ErrorExit("FindFirstFileW: Should never get here. No can do!");
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

				wcscpy_s(currPathW, maxPathFolder, L"\0");
				wcscat_s(currPathW, maxPathFolder, dw.cFileName);
				wcscat_s(currPathW, maxPathFolder, L"\\");


				wcscpy_s(folderTreeArray[treeLevel][j], maxPathFolder, (wchar_t *)currPathW);
				j +=1;

			}
				findhandle = FindNextFileW(ds, &dw);
			}

			FindClose(ds);
			trackFTA [treeLevel][0] = 0; //check reset counter if necessary here

			if (j == 0)
				{
				// No Folders so this must be top level
				GetCurrentDirectoryW(maxPathFolder, findPathW);
					if (!SetCurrentDirectoryW (L"..")) ErrorExit("SetCurrentDirectoryW: Non zero");

					if (!GetCurrentDirectoryW(maxPathFolder, findPathW)) ErrorExit("SetCurrentDirectoryW: Non zero");
					if (treeLevel == 1) //Last folder to do!!
					{
					wchar_t * currPathWtmp = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
					currPathWtmp = currPathW + 4;
					GetCurrentDirectoryW(maxPathFolder, findPathW);
						if (RemoveDirectoryW (currPathWtmp))
						{
							return 0;
						}
						else
						{
							ErrorExit("RemoveDirectoryW: Cannot remove Folder. It may contain files.");

							return 1; //Need more than this
						}

					}

				//GetCurrentDirectoryW(maxPathFolder, findPathW);
				if (RemoveDirectoryW (currPathW))
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
					ErrorExit("RemoveDirectoryW: Cannot remove Folder. It may contain files.");
					return 1; //Need more than this
				}





					if (RecurseRemovePath(trackFTA, folderTreeArray)) return 1;
				}

		else //Do an iteration on this new branch
			{
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
