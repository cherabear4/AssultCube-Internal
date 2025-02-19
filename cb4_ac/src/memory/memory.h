#pragma once
#include "../../includes.h"

namespace mem
{
	inline uintptr_t readTargetModule(const char* moduleName)
	{
			HMODULE hModule = GetModuleHandle(moduleName);
			if (!hModule)
			{
				throw std::runtime_error("Module not found.");
			}
			return reinterpret_cast<uintptr_t>(hModule);
	}

	inline bool IsValidPtr(uintptr_t ptr)
	{
		MEMORY_BASIC_INFORMATION mbi;
		return (VirtualQuery(reinterpret_cast<LPCVOID>(ptr), &mbi, sizeof(mbi)) &&
			(mbi.Protect & PAGE_READWRITE) &&
			!(mbi.Protect & PAGE_NOACCESS));
	}

	template<typename T>
	inline T ReadMemory(uintptr_t address) {
		if (!IsValidPtr(address)) return T();
		return *reinterpret_cast<T*>(address);
	}

	template<typename T>
	inline bool ReadMemory(uintptr_t address, T* buffer, size_t size = sizeof(T)) {
		if (!IsValidPtr(address)) return false;
		memcpy(buffer, reinterpret_cast<void*>(address), size);
		return true;
	}

	template<typename T>
	inline void WriteMemory(uintptr_t baseAddress, uintptr_t offset, T value)
	{
		*(T*)(baseAddress + offset) = value;
	}

	template<typename T>
	inline T ReadMemory(uintptr_t baseAddress, uintptr_t offset)
	{
		return *(T*)(baseAddress + offset);
	}
    
	inline bool readRaw(uintptr_t address, void* buffer, size_t size) {
		memcpy(buffer, reinterpret_cast<void*>(address), size);
		return true;
	}
}