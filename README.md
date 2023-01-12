# Inline Hook
Head only, User mode inline hook support x86 x64

### Example

hook `LoadLibraryW`

```cpp
#include <stdio.h>
#include <Windows.h>
#include "inline_hook.hpp"


HMODULE WINAPI MyLoadLibraryW(LPCWSTR lpLibFileName);


//LoadLibraryW的函数指针
typedef HMODULE(WINAPI* LoadLibraryWType)(LPCWSTR lpLibFileName);


auto inline_hook = InlineHook<LoadLibraryWType>(reinterpret_cast<ULONG_PTR>(LoadLibraryW), MyLoadLibraryW);


//自己的LoadLibraryW
HMODULE WINAPI MyLoadLibraryW(LPCWSTR lpLibFileName)
{
	printf("MyLoadLibraryW:%ws\n", lpLibFileName);
	
	//拒绝ntdll.dll
	if (wcsstr(lpLibFileName, L"ntdll.dll"))
	{
		return reinterpret_cast<HMODULE>(0x114514);
	}
	
	
	//调用原始函数
	auto original_func_result = inline_hook.CallOriginalFunc(lpLibFileName);

	printf("original func result:%p\n", original_func_result);

	return original_func_result;
}



int main() {
	
	inline_hook.Motify();
	auto ntdll_module = LoadLibraryW(L"ntdll.dll");
	auto user32_module = LoadLibraryW(L"user32.dll");
	
	printf("ntdll module:%p\n", ntdll_module);    //ntdll module:0000000000114514
	printf("user32 module:%p\n", user32_module);  //user32 module:00007FFF8B080000

	
	return 0;
}

```
