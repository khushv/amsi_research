#include <iostream>
#include <windows.h>
#include <psapi.h>
#include <tchar.h>

int main(int argc, char * argv[]) {
	if (argc != 2) {
		printf("Usage: %s PID\n", argv[0]);
		return 0;
	}

	int pid = atoi(argv[1]);
	printf("[*] Using PID value of %d.\n", pid);

	printf("[*] Opening process handle.\n");
	HANDLE  processHandle = OpenProcess(
		PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE, 1, GetCurrentProcessId());
	if (processHandle == 0) {
		printf("[*] Error, unable to self process, error is %d", GetLastError());
		return 0;
	}
	
	BOOL process_type;
	if (IsWow64Process(processHandle, &process_type) == 0) {
		printf("[*] Error: Could not determine if process if 32 or 64 bit. Error code is %d.\n", GetLastError());
		return 0;
	}
	if (process_type == 1) {
		printf("[*] Process is 32 bit.\n");
	}
	else {
		printf("[*] Process is 64 bit.\n");
	}

	printf("[*] Getting amsi.dll module handle.\n");

	HMODULE mod_handles_array[16384];
	DWORD bytes_needed;
	if (EnumProcessModulesEx(processHandle, mod_handles_array, sizeof(mod_handles_array), &bytes_needed, 0x02) == 0) {
		printf("[*] Error enumerating process modules. Error is %d.\n", GetLastError());
		return 0;
	}

	for (int i = 0; i < (bytes_needed / sizeof(HMODULE)); i++)
	{
		TCHAR szModName[MAX_PATH];
		// Get the full path to the module's file.
		if (GetModuleFileNameEx(processHandle, mod_handles_array[i], szModName,
			sizeof(szModName) / sizeof(TCHAR)))
		{
			// Print the module name and handle value.
			_tprintf(TEXT("%s (0x%08X)\n"), szModName, mod_handles_array[i]);
		}
	}

	
	SYSTEM_INFO sys_info;
	GetSystemInfo(&sys_info);

	DWORD dwPageSize = sys_info.dwPageSize;
	LPVOID lpMinimumApplicationAddress = sys_info.lpMinimumApplicationAddress;
	LPVOID lpMaximumApplicationAddress = (LPVOID)0x7ff90efa0000; // sys_info.lpMaximumApplicationAddress;
	//(LPVOID) 0x7FFFFFFFFFF;0x7ff90ef30000
	printf("[*] Page size is %d\n", dwPageSize);
	printf("[*] Minimum address is %p\n", lpMinimumApplicationAddress);
	printf("[*] Maximum address is %p\n", lpMaximumApplicationAddress);
	
	TCHAR tofind[] = _T("amsi");

	MEMORY_BASIC_INFORMATION64 test;
	SIZE_T result;
	int* i = (int*)lpMaximumApplicationAddress;
	for (i; i > lpMinimumApplicationAddress; i -= dwPageSize) {
		printf("[*] Trying address %p.\n", i);
		result = VirtualQueryEx(processHandle, (LPCVOID) i, (PMEMORY_BASIC_INFORMATION)&test, sizeof(test));
		if (1== 1) { //test.Type == 0x1000000 || test.Type == 0x40000){
			TCHAR filepath[MAX_PATH];
			DWORD filepath_size;
			_tprintf(filepath, "\n");
			filepath_size = K32GetMappedFileNameW(processHandle, i, (LPWSTR)&filepath, MAX_PATH);


			if (_tcsstr(filepath, tofind) != 0) {
				printf("[*] Result is %d.\n", result);
				printf("[*] State is %x.\n", test.Type);
				printf("[*] Getting filename.\n");

				break;

			}
	
		}
	}
}