#pragma once

extern "C" {
#include <lua/lauxlib.h>
#include <lua/lua.h>
#include <lua/luaconf.h>
#include <lua/lualib.h>
}
#include <iostream>

namespace wc {
	
	class Lua{
	public:
		Lua() {}
		Lua(const char* file) { Load(file); }
		void Load(const char* file) {
			L = luaL_newstate();
			luaL_openlibs(L);
			if (!CheckLua(luaL_dofile(L, file))) {
				std::cout << GetErrorMessage();
			}
		}
		int GetNumber(const char* value) {
			lua_getglobal(L, value);
			if (lua_isnumber(L, -1))
				return lua_tonumber(L, -1);
			else return 0;
		}
		float GetFloat(const char* value) {
			lua_getglobal(L, value);
			if (lua_isnumber(L, -1))
				return lua_tonumber(L, -1);
			else return 0;
		}
		bool GetBool(const char* value) {
			lua_getglobal(L, value);
			if (lua_isboolean(L, -1))
				return lua_toboolean(L, -1);
			else
				return false;
		}
		const char* GetString(const char* value) {
			lua_getglobal(L, value);
			if (lua_isstring(L, -1)) 
				return lua_tostring(L, -1);
		}
		//auto GetValue(const char* value) {
		//	lua_getglobal(L, value);
		//
		//	if (lua_isstring(L, -1))
		//		return lua_tostring(L, -1);
		//	if (lua_isboolean(L, -1))
 		//		return lua_toboolean(L, -1);
		//	if (lua_isnumber(L, -1))
		//		return (float)lua_tonumber(L, -1);
		//
		//
		//}
		void Close() {
			lua_close(L);
		}
		~Lua() {
			Close();
		}
		bool CheckLua(int32_t r) {
		if (r != LUA_OK) return false;
		else return true;
		}
		const char* GetErrorMessage() {
			const char* errormsg = lua_tostring(L, -1);
			return errormsg;
		}

	private:
		lua_State* L;
	};

}
