#pragma once
#include <Windows.h>
#include <map>
#include <atomic>
#include <memory>
#if _M_IX86
#define OFFSET_TO_ADDRESS 0x1
#elif _M_X64
#define OFFSET_TO_ADDRESS 0x2
#endif
namespace hook
{
	inline void write_to_readonly(void* addr, void* data, int size)
	{
		DWORD old_flags;
		if (!VirtualProtect(
			addr,
			size,
			PAGE_EXECUTE_READWRITE,
			&old_flags
		)) return;

		memcpy(addr, data, size);

		VirtualProtect(
			addr,
			size,
			old_flags,
			&old_flags
		);
	}
	class detour
	{
	public:
		detour(void* addr_to_hook, void* jmp_to, bool enable = true)
			: hook_addr(addr_to_hook), detour_addr(jmp_to), hook_installed(false)
		{
			//setup hook
			memcpy(
				jmp_code + OFFSET_TO_ADDRESS,
				&jmp_to,
				sizeof(jmp_to)
			);
			//save bytes
			memcpy(
				org_bytes,
				hook_addr,
				sizeof(org_bytes)
			);
			if (enable)
				install();
		}
		void install()
		{
			if (hook_installed.load())
				return;
			write_to_readonly(hook_addr, jmp_code, sizeof(jmp_code));
			hook_installed.exchange(true);
		}
		void uninstall()
		{
			if (!hook_installed.load())
				return;
			write_to_readonly(hook_addr, org_bytes, sizeof(org_bytes));
			hook_installed.exchange(false);
		}
		~detour() { uninstall(); }
		bool installed() { return hook_installed; }
		void* hook_address() { return hook_addr; }
		void* detour_address() { return detour_addr; }
	private:
		std::atomic<bool> hook_installed;
		void* hook_addr, * detour_addr;
#if _M_IX86
		/*
			0:  b8 ff ff ff ff          mov    eax, 0xffffffff
			5:  ff e0                   jmp    eax
		*/
		unsigned char jmp_code[7] = {
			0xb8, 0x0, 0x0, 0x0, 0x0,
			0xFF, 0xE0
		};
#elif _M_X64
		/*
			0:  48 c7 c0 ff ff ff ff    mov    rax,0xffffffffffffffff
			7:  ff e0                   jmp    rax
		*/
		unsigned char jmp_code[12] = {
			0x48, 0xb8,
			0x0,
			0x0,
			0x0,
			0x0,
			0x0,
			0x0,
			0x0,
			0x0,
			0xff, 0xe0
		};
#endif
		std::uint8_t org_bytes[sizeof(jmp_code)];
	};
	inline std::map<void*, std::unique_ptr<detour>> hooks{};
	inline void make_hook(void* addr_to_hook, void* jmp_to_addr, bool enable = true)
	{
		if (!addr_to_hook)
			return;
		hooks.insert({
			addr_to_hook,
			std::make_unique<detour>(
				addr_to_hook,
				jmp_to_addr,
				enable
			) }
		);
	}
	inline void enable(void* addr)
	{
		if (!addr)
			return;
		hooks.at(addr)->install();
	}
	inline void disable(void* addr)
	{
		if (!addr)
			return;
		hooks.at(addr)->uninstall();
	}
	inline void remove(void* addr)
	{
		if (!addr)
			return;
		hooks.erase(addr);
	}
}