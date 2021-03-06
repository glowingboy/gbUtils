#include "luastate.h"
extern "C"
{
#include <lua/lauxlib.h>
#include <lua/lualib.h>
}
#include <cassert>
#include "logger.h"
#include "string.h"
#include <thread>

using namespace gb::utils;

luastate::luastate():
    _Lock(false)
{
    _l = luaL_newstate();
    assert(_l != nullptr);
    luaL_openlibs(_l);
}

luastate::~luastate()
{
    lua_close(_l);
}
bool luastate::dofile(const char* szLuaFile)
{
    if(!_Lock)
    {
	lua_pushcfunction(_l, _debug_traceback);
	const int errCode = luaL_loadfile(_l, szLuaFile);
	if(errCode !=0 )
	{
	    logger::Instance().error(string("luastate::dofile luaL_loadfile err@ ")
				     + lua_tostring(_l, -1)
				     + ", file@ " + szLuaFile);
	    return false;
	}
	return lua_pcall(_l, 0, 1, -2) == 0 ? true : false;
    }
    else
    {
	logger::Instance().error(string("luastate::dofile locked szLuaFile@ ") + szLuaFile);
	return false;
    }
}

bool luastate::dostring(const char* szLua)
{
    if(!_Lock)
    {
	lua_pushcfunction(_l, _debug_traceback);
	const int errCode = luaL_loadstring(_l, szLua);
	if(errCode !=0 )
	{
	    logger::Instance().error(string("luastate::dostring luaL_loadstring err@ ")
				     + lua_tostring(_l, -1)
				     + ", lua@ " + szLua);
	    return false;
	}
	return lua_pcall(_l, 0, 1, -2) == 0 ? true : false;
    }
    else
    {
	logger::Instance().error(string("luastate::dostring locked szLua@ ") + szLua);
	return false;
    }
}

int luastate::_debug_traceback(lua_State* l)
{
    lua_getfield(l, LUA_GLOBALSINDEX, "debug");
    lua_getfield(l, -1, "traceback");
    lua_pushvalue(l, 1);
    lua_call(l, 1, 1);
    logger::Instance().error(string("_debug_traceback@ ") + lua_tostring(l, -1));
    return 1;
}

void luastate::append_packagepath(const char* path)
{
    if(!_Lock)
    {
	lua_getglobal(_l, "package");
	lua_getfield(_l, -1, "path");
	string cur_path(lua_tostring(_l, -1));
	cur_path += ";";
	cur_path += path;
	cur_path += "/?.lua";
	lua_pop(_l, 1);
	lua_pushstring(_l, cur_path);
	lua_setfield(_l, -2, "path");
	lua_pop(_l, 1);
    }
    else
	logger::Instance().error(string("luastate::append_packagepath locked path@ ") + path);
}

lua_State* luastate::getstate()
{
    return _l;
}

// luastate_mgr::luastate_mgr():
//     _threadCount(0),
//     _config_l(nullptr),
//     _logic_l(nullptr)
// {
// }

#if GB_UTILS_MULTI_THREADS
std::uint8_t luastate_mt::_threadCount = std::thread::hardware_concurrency();
#else
std::uint8_t luastate_mt::_threadCount = 1;
#endif

luastate_mt::luastate_mt():
    _ls(nullptr)
{
    _ls = new luastate[_threadCount];
}

luastate_mt::~luastate_mt()
{
    GB_SAFE_DELETE_ARRAY(_ls);
}



luastate& luastate_mt::operator[](const std::uint8_t threadIdx)
{
    assert(threadIdx < _threadCount);
    return _ls[threadIdx];
}

void luastate_mt::dofile(const char* szLuaFile)
{
    for(std::uint8_t i = 0; i < _threadCount; i++)
    {
	_ls[i].dofile(szLuaFile);
    }
}

void luastate_mt::dostring(const char* szLua)
{
    for(std::uint8_t i = 0; i < _threadCount; i++)
    {
	_ls[i].dostring(szLua);
    }
}

// luastate_mgr::~luastate_mgr()
// {
//     GB_SAFE_DELETE_ARRAY(_config_l);
//     GB_SAFE_DELETE_ARRAY(_logic_l);
// }
// void luastate_mgr::initialize(const unsigned char threadCount)
// {
//     assert(_threadCount == 0);
// 	_threadCount = threadCount == 0 ? std::thread::hardware_concurrency() : threadCount;
//     _config_l = new luastate[_threadCount];
//     _logic_l = new luastate[_threadCount];
    
// }

// luastate& luastate_mgr::getconfig_state(const unsigned char threadIdx)
// {
//     assert(threadIdx >= 0 && threadIdx < _threadCount);
    
//     return _config_l[threadIdx];
// }

// luastate& luastate_mgr::getlogic_state(const unsigned char threadIdx)
// {
//     assert(threadIdx >= 0 && threadIdx < _threadCount);

//     return _logic_l[threadIdx];
// }



