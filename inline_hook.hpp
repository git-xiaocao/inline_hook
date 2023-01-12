// Repository: https://github.com/git-xiaocao/inline_hook
// Author: 小草
// Date: 2023/1/12

#pragma once

#include <Windows.h>
#include <functional>
#ifdef _WIN64
// mov rax, addr
// jmp rax
constexpr SIZE_T byte_code_length = 2 + 2 + sizeof(LONG_PTR);//12
#else
// jmp addr
constexpr SIZE_T byte_code_length = 1 + sizeof(LONG_PTR);//5
#endif

template<typename F>
class InlineHook
{
private:

	//原始函数字节码
	BYTE original_bytes_[byte_code_length];
	//asm jmp 字节码
	BYTE jmp_bytes_[byte_code_length];

	// 原始地址
	ULONG_PTR original_address_;
	// 目标地址
	ULONG_PTR self_address_;

	F original_func_;

	DWORD MotifyMemoryAttributes(ULONG_PTR address, DWORD attributes = PAGE_EXECUTE_READWRITE)
	{
		DWORD old_attributes;
		VirtualProtect(reinterpret_cast<void*>(address), byte_code_length, attributes, &old_attributes);
		return old_attributes;
	}

public:

	InlineHook(ULONG_PTR original_address, F self_func)
	{
		original_func_ = reinterpret_cast<F>(original_address);
		original_address_ = original_address;
		self_address_ = reinterpret_cast<ULONG_PTR>(self_func);


#ifdef _WIN64
		//mov rax
		jmp_bytes_[0] = 0x48;
		jmp_bytes_[1] = 0xB8;

		*(PLONG_PTR)(jmp_bytes_ + 2) = self_address_;

		//jmp rax
		*(PBYTE)(jmp_bytes_ + 2 + sizeof(self_address_)) = 0xFF;
		*(PBYTE)(jmp_bytes_ + 2 + sizeof(LONG_PTR) + 1) = 0xE0;
#else
		//计算偏移
		LONG_PTR offset = self_address_ - (original_address_ + byte_code_length);
		//jmp offset
		jmp_bytes_[0] = 0xE9;
		
		*(PLONG_PTR)(jmp_bytes_ + 1) = offset;
#endif
		//修改内存属性
		DWORD attributes = MotifyMemoryAttributes(original_address_);

		//保存原始函数地址的字节码
		RtlCopyMemory(original_bytes_, reinterpret_cast<PBYTE>(original_address_), byte_code_length);

		//恢复内存属性
		MotifyMemoryAttributes(original_address_, attributes);


	}

	~InlineHook() {
		Restore();
	}

	//调用原始函数
	template<typename... Args>
	auto CallOriginalFunc(Args&&... args) {
		Restore();
		auto result = original_func_(std::forward<Args>(args)...);
		Motify();
		return result;
	}


	//修改地址
	void Motify()
	{
		DWORD attributes = MotifyMemoryAttributes(original_address_);

		//写入字节码
		RtlCopyMemory(reinterpret_cast<PBYTE>(original_address_), jmp_bytes_, byte_code_length);

		MotifyMemoryAttributes(original_address_, attributes);
	}

	//恢复字节码
	void Restore()
	{
		DWORD attributes = MotifyMemoryAttributes(original_address_);

		RtlCopyMemory(reinterpret_cast<PBYTE>(original_address_), original_bytes_, byte_code_length);

		MotifyMemoryAttributes(original_address_, attributes);
	}

};
