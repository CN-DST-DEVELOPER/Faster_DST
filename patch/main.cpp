#include <forward_list>
#include <utility>
#include <dlfcn.h>
#include <cassert>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cstring>
#include <algorithm>
#include <openssl/md5.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <link.h>

#include "Cache.h"
#include "FunctionLocater/FunctionLocater.hpp"

#define PAGE_SIZE sysconf(_SC_PAGESIZE)
#define PAGE_MASK (~(PAGE_SIZE - 1))
#define PAGE_ALIGN(addr) (unsigned char *)(((uintptr_t)(addr) + PAGE_SIZE - 1) & PAGE_MASK)

#define PROC_ALIGN 0x10
#define SIGN_SIZE 30
#define SIGN_MIN_LENGTH 12
#define MAX_SINGLE_INSTR_LENGTH 15

#ifdef DEBUG_DIR
#define PATH_LUAJIT DEBUG_DIR "libluajit.so"
#define PATH_LUA51 DEBUG_DIR "liblua.so"
#define PATH_SELF DEBUG_DIR "libpreload.so"
#define PATH_CACHE DEBUG_DIR "../temp/fast_dst.cache"
#define DEBUG(...) printf("[DST_LUAJIT_DEBUG] " __VA_ARGS__)
#else
#define PATH_LUAJIT "./libluajit.so"
#define PATH_LUA51 "./liblua.so"
#define PATH_SELF "./libpreload.so"
#define PATH_CACHE "./fast_dst.cache"
#define DEBUG(...) ((void)0)
#endif

#define INFO(...) printf("[DST_LUAJIT] " __VA_ARGS__)

const std::unordered_set<char *> allFunctions = {

	"luaL_addlstring",
	"luaL_addstring",
	"luaL_addvalue",
	"luaL_argerror",
	"luaL_buffinit",
	"luaL_callmeta",
	"luaL_checkany",
	"luaL_findtable",
	"luaL_gsub",
	// "luaL_loadbuffer", //missing function
	// "luaL_loadfile", //missing function
	// "luaL_loadstring", //missing function
	"luaL_newmetatable",
	// "luaL_newstate", //被内联
	// "luaL_openlibs", //missing function
	// "luaL_optinteger", //missing function
	// "luaL_optlstring", //missing function
	// "luaL_optnumber", //missing function
	"luaL_prepbuffer",
	"luaL_pushresult",
	"luaL_ref",
	"luaL_register",
	"luaL_unref",
	"luaL_where",
	"lua_equal",
	// "lua_getallocf", //missing function
	"lua_getupvalue",
	"luaL_checkudata",
	"lua_pushvfstring",
	// "lua_setallocf", 这个函数实现完全相同，无需替换
	"lua_setupvalue",
	"internal_error",
	"luaL_checktype",
	"luaL_error",
	"luaL_typerror",
	"lua_atpanic",
	"lua_call",
	"lua_checkstack",
	"lua_close",
	"lua_pushlstring",
	"lua_dump",
	"lua_error",
	"lua_gc",
	"lua_getfenv",
	"lua_getfield",
	"lua_getinfo",
	"lua_getlocal",
	"lua_getmetatable",
	"lua_getstack",
	"lua_gettable",
	"lua_gettop",
	"lua_insert",
	"lua_iscfunction",
	"lua_isnumber",
	// "lua_isstring", //missing function
	"lua_lessthan",
	"lua_load",
	"lua_newstate",
	"lua_newthread",
	"lua_newuserdata",
	"lua_next",
	"lua_objlen",
	"lua_pcall",
	"lua_pushboolean",
	"lua_pushcclosure",
	"lua_pushfstring",
	"lua_pushinteger",
	"lua_pushlightuserdata",
	"lua_createtable",
	"lua_pushnil",
	"lua_pushnumber",
	"lua_pushstring",
	"lua_pushthread",
	"lua_pushvalue",
	"lua_rawequal",
	"lua_rawget",
	"lua_rawgeti",
	"lua_rawset",
	"lua_rawseti",
	"lua_remove",
	"lua_replace",
	"lua_resume",
	"lua_setfenv",
	"lua_setfield",
	"lua_sethook",
	"lua_setlocal",
	"lua_setmetatable",
	"lua_settable",
	"lua_settop",
	"lua_toboolean",
	"lua_tocfunction",
	"lua_tointeger",
	"lua_tolstring",
	"lua_tonumber",
	"lua_topointer",
	"lua_tothread",
	"lua_touserdata",
	"lua_type",
	"lua_typename",
	"lua_concat",
	"lua_xmove",
	"lua_yield",
	"luaopen_base",
	"luaopen_debug",
	"luaopen_io", // missing function
	"luaopen_math",
	"luaopen_os",
	"luaopen_package",
	"luaopen_string",
	"luaopen_table",
};

class Bridge
{
private:
	void Hook(void *src, void *dst)
	{
		// inline hook
		unsigned char code[14] = {0x00};
		uint8_t temp[MAX_SINGLE_INSTR_LENGTH] = {0x00};
		TLengthDisasm instr;
		uint8_t len;

		// push目标地址低8位入栈
		memset(&instr, 0x00, sizeof(TLengthDisasm));
		instr.Opcode[0] = 0x68;
		instr.OpcodeSize = 1;
		instr.ImmediateDataSize = 4;
		instr.ImmediateData.ImmediateData32 = (long)dst & 0xffffffff;
		instr.Flags |= F_IMM;
		len = LengthAssemble(temp, &instr);
		memcpy(code, &temp, len);

		// mov 高8位
		memset(&instr, 0x00, sizeof(TLengthDisasm));
		instr.Opcode[0] = 0xc7;
		instr.OpcodeSize = 1;
		instr.ModRMByte = 0x44;
		instr.SIBByte = 0x24;
		instr.DisplacementSize = 1;
		instr.AddressDisplacement.Displacement08 = 0x04;
		instr.ImmediateDataSize = 4;
		instr.ImmediateData.ImmediateData64 = (long)dst >> 32 & 0xffffffff;
		instr.Flags |= F_MODRM | F_SIB | F_DISP | F_IMM;
		memcpy(code + len, &temp, LengthAssemble(temp, &instr));

		code[13] = 0xc3;

		mprotect(PAGE_ALIGN(src) - PAGE_SIZE, PAGE_SIZE * 2, PROT_READ | PROT_WRITE | PROT_EXEC);
		memcpy(src, code, 14);
		mprotect(PAGE_ALIGN(src) - PAGE_SIZE, PAGE_SIZE * 2, PROT_READ | PROT_EXEC);
	}

	void doRedirect(void *lua51, void *luajit)
	{
		INFO("Searching for all function entry...\n");

		std::unordered_set<uint8_t *> func_addr;
		FunctionLocater::GetFunctionEntry(func_addr, dst_text);

		INFO("Calculating function signature similarity... This might take a while...\n");

		std::forward_list<FunctionLocater::Entry> alternative_list;
		uint8_t alternative[SIGN_SIZE];

		for (auto p = func_addr.begin(); p != func_addr.end(); p++)
		{
			memset(alternative, 0x00, SIGN_SIZE);
			std::forward_list<FunctionLocater::Sign> *func_sign = new std::forward_list<FunctionLocater::Sign>();
			FunctionLocater::GetFunctionSignature(*p, func_sign, dst_data, SIGN_SIZE);
			alternative_list.push_front(FunctionLocater::Entry{*p, func_sign});
		}

		//                  目标地址, 源地址, 匹配度, 函数名
		std::unordered_map<uint8_t *, std::unordered_map<uint8_t *, std::pair<int, std::string>>> hook_point;

		for (std::unordered_set<char *>::const_iterator name = allFunctions.begin(); name != allFunctions.end(); name++)
		{

			uint8_t *address_luajit = (uint8_t *)dlsym(luajit, *name);
			uint8_t *address_lua51 = (uint8_t *)dlsym(lua51, *name);
			assert(address_luajit != nullptr);
			assert(address_lua51 != nullptr);

			std::forward_list<FunctionLocater::Sign> func_signature;
			FunctionLocater::GetFunctionSignature(address_lua51, &func_signature, lua51_data, SIGN_SIZE);

			for (auto it = alternative_list.begin(); it != alternative_list.end(); it++)
			{
				uint8_t count = FunctionLocater::GetSignatureLCS(it->sign, &func_signature);
				hook_point[address_luajit][it->address] = std::make_pair(count, *name);
			}
		}
		for (std::forward_list<FunctionLocater::Entry>::iterator it = alternative_list.begin(); it != alternative_list.end(); it++)
		{
			FunctionLocater::DelSign(it->sign);
		}

		// 源地址, 目标地址
		std::unordered_map<uint8_t *, uint8_t *> hook_list;
		std::unordered_map<std::string, uint8_t *> cache_data;

		INFO("Locating hook point...\n");
		FunctionLocater::MatchFunctions(hook_point, hook_list);

		INFO("Saving hook point cache and Redirecting functions...\n");

		for (auto hook_point_it = hook_list.begin(); hook_point_it != hook_list.end(); hook_point_it++)
		{
			std::string name = hook_point[hook_point_it->second][hook_point_it->first].second;
			cache_data[name] = hook_point_it->first;
			// DEBUG("%-24sHooked\t[%p](%d)\n", name.c_str(), hook_point_it->first, hook_point[hook_point_it->second][hook_point_it->first].first);

			Hook(hook_point_it->second, hook_point_it->first);
		}

		cache->SetMode(Cache::WRITE);
		cache->SaveCache(cache_data);
	}

	static int phdr_callback(struct dl_phdr_info *info, size_t size, void *offset)
	{
		if (*info->dlpi_name == '\0')
		{
			((std::pair<uint64_t, uint64_t> *)offset)->first = info->dlpi_addr;
		}
		else if (strstr(info->dlpi_name, "/liblua.so"))
		{
			((std::pair<uint64_t, uint64_t> *)offset)->second = info->dlpi_addr;
		}

		return 0;
	}

	// dst_offset, lua51_offset
	std::pair<uint64_t, uint64_t> offset;

	FunctionLocater::SectionArea dst_text;
	FunctionLocater::SectionArea dst_data;
	FunctionLocater::SectionArea lua51_data;
	int nullfd;
	char dst_path[128];
	std::set<char *> cache_key = {
		PATH_LUAJIT,
		PATH_LUA51,
		PATH_SELF,
		dst_path,
	};
	Cache *cache;

public:
	Bridge()
	{
		int ret = readlink("/proc/self/exe", dst_path, sizeof(dst_path) - 1);
		dst_path[ret] = '\0';
		if (strstr(dst_path, "dontstarve_") == nullptr)
		{
			return;
		}

		INFO("DST_LUAJIT Injected!\n");
		void *dst = dlopen(nullptr, RTLD_GLOBAL);
		void *lua51 = dlopen(PATH_LUA51, RTLD_LOCAL | RTLD_NOW);
		void *luajit = dlopen(PATH_LUAJIT, RTLD_GLOBAL | RTLD_LAZY);
		if (lua51 == nullptr || luajit == nullptr)
		{
			INFO("################################################################\n");
			INFO("##   DST_LUAJIT Failed To Load!\n");
			INFO("##   Run with default Lua Interpreter.\n");
			INFO("##   Please read repository Readme for more information.\n");
			INFO("##   If this continue to fail, please open an issue and report.\n");
			INFO("################################################################\n");
		}
		else
		{
			// DEBUG("Main handle: %p\n", dst);
			// DEBUG("Lua51 handle: %p\n", lua51);
			// DEBUG("LuaJIT handle: %p\n", luajit);

			cache = new Cache(PATH_CACHE, cache_key);

			INFO("Tring to load hook point cache...\n");
			cache->SetMode(Cache::READ);
			if (cache->VaildCache())
			{
				// 函数名, 源地址
				std::unordered_map<char *, uint8_t *> hook_list;
				cache->LoadCache(hook_list);
				for (auto it = hook_list.begin(); it != hook_list.end(); it++)
				{
					Hook(it->second, dlsym(luajit, it->first));
				}
				INFO("Hook with cache data successfully!...\n");
			}
			else
			{
				INFO("Cache load failed!\n");
				INFO("Starting to relocate entry address...\n");
				dl_iterate_phdr(phdr_callback, &offset);
				FunctionLocater::GetSectionArea(dst_path, &dst_text, &dst_data, offset.first);
				FunctionLocater::GetSectionArea(PATH_LUA51, nullptr, &lua51_data, offset.second);

				nullfd = open("/dev/null", O_WRONLY);

				if (dst_text.start < dst_text.end)
				{
					doRedirect(lua51, luajit);
				}

				close(nullfd);
			}
			delete cache;
			dlclose(lua51);
			INFO("Function hooking successfully, enjoy your game!\n");
		}
	}
};

#ifndef SINGLE_DEBUG
Bridge bridge;
#else
int main()
{
	return 0;
}
#endif
