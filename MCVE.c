#include <windows.h>
#include <Winternl.h>

typedef NTSTATUS (__stdcall *NTDLLptr)(
    PHANDLE FileHandle, 
    ACCESS_MASK DesiredAccess, 
    POBJECT_ATTRIBUTES ObjectAttributes, 
    PIO_STATUS_BLOCK IoStatusBlock, 
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes, 
    ULONG ShareAccess, 
    ULONG CreateDisposition, 
    ULONG CreateOptions, 
    PVOID EaBuffer, 
    ULONG EaLength );


typedef VOID (__stdcall *my_RtlInitUnicodeString) (
    IN OUT PUNICODE_STRING  DestinationString,
    IN PCWSTR  SourceString );

my_RtlInitUnicodeString RtlInitUnicodeString = NULL; //error C2365: 'RtlInitUnicodeString' : redefinition; previous definition was 'function'	
//comment out above line and remove comment from line 53 it compiles.
NTDLLptr foundNTDLL = NULL;
UNICODE_STRING fn;
OBJECT_ATTRIBUTES fileObject;
IO_STATUS_BLOCK ioStatus;
const char createFnString[13] = "NtCreateFile";
const char initUnicodeFnString[21] = "RtlInitUnicodeString";
wchar_t *tempDest;


NTDLLptr DynamicLoader ();

int main( int argc, const char* argv[] )
{
DynamicLoader ();
}


NTDLLptr DynamicLoader ()
{
	HMODULE hdlNtCreateFile = LoadLibraryW(L"NtDll.dll");
	foundNTDLL = (NTDLLptr) GetProcAddress (hdlNtCreateFile, createFnString);
	if (foundNTDLL)
	{

		memset(&ioStatus, 0, sizeof(ioStatus));
		memset(&fileObject, 0, sizeof(fileObject));
		fileObject.Length = sizeof(fileObject);
		fileObject.Attributes = OBJ_CASE_INSENSITIVE;

		//my_RtlInitUnicodeString RtlInitUnicodeString = (my_RtlInitUnicodeString) GetProcAddress(hdlNtCreateFile, initUnicodeFnString);
		RtlInitUnicodeString = (my_RtlInitUnicodeString) GetProcAddress(hdlNtCreateFile, initUnicodeFnString); //error C2659: '=' : function as left operand	
		RtlInitUnicodeString(&fn, tempDest);
		fileObject.ObjectName = &fn;
	}

	FreeLibrary (hdlNtCreateFile);
	return foundNTDLL;

}

 

