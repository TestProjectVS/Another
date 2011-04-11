#include "game.h"
#include <iostream>

/*
The include path needs to point to the lua include directory, depending on where its installed
To link it successfully you need to link with both lua5.1.lib and lua51.lib
To run, the program needs to have access to the lua5.1.dll and lua51.dll, the easiest way is to copy them to the binary directory.
*/


extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

using namespace std;

/*
Declare the Lua libraries we wish to use.
Any libraries used from the lua scripts loaded later must be present here.
*/
static const luaL_reg lualibs[] =
{
	{ "base",       luaopen_base },
	{ NULL,         NULL }
};

/* A function to open up all the Lua libraries you declared above. */
static void openlualibs(lua_State *l)
{
	const luaL_reg *lib;

	for (lib = lualibs; lib->func != NULL; lib++)
	{
		lib->func(l);
		lua_settop(l, 0);
	}
}

int MyLuaError(lua_State *l)
{
	cerr << "Failed to execute the lua script:" << endl;
	cerr << lua_tostring(l, -1) << endl;
	lua_pop(l, 1); // pop the error msg from the stack
	return 1;
}

int CallLuaFunc(lua_State *l)
{
	lua_getglobal(l, "func");
	lua_pushstring(l, "aa");
	lua_pushstring(l, "bb");
	lua_pushstring(l, "cc");
	lua_pcall(l,3,LUA_MULTRET,0);

	int argc = lua_gettop(l);
	cerr << "func returned " << argc << " arguments" << endl;
	for ( int n=2; n<=argc; ++n ) {
		cerr << "-- argument " << n << ": "	<< lua_tostring(l, n) << std::endl;
	}
	
	lua_pop(l, argc); // remove the return values

	return 0;
}

int CallGameTick(lua_State *l, int time)
{
	cout << "gametick" << endl;
	lua_getglobal(l, "tick");
	if (!lua_isfunction(l, -1)) luaL_error(l, "function not found");
	lua_pushinteger(l, time);
	if (lua_pcall(l,1,1,0) != 0) luaL_error(l, "error running tick: %s", lua_tostring(l, -1));

	int rv = 0;
	if (lua_isboolean(l, -1))			rv = lua_toboolean(l, -1);
	else if (lua_isnumber(l, -1))	rv = lua_tointeger(l, -1);
	else luaL_error(l, "unexpected type");
	
	int retvals = lua_gettop(l);
	lua_pop(l, retvals); // pop the return value! we dont want to blow the stack
	return rv;
}

int my_function(lua_State *l)
{
	int argc = lua_gettop(l);

	cerr << "-- my_function() called with " << argc << " arguments:" << std::endl;

	for ( int n=1; n<=argc; ++n ) {
		cerr << "-- argument " << n << ": "	<< lua_tostring(l, n) << std::endl;
	}
	
	lua_pop(l, argc); // remove the arguments
	lua_pushnumber(l, 123); // return value
		
	return 1; // number of return values
}

int main(int argc, char** argv)
{
	Game game;

	char inline_script[] = "print(\"I'm in your lua, scriptzoring your bitz\"); ";

	cout << "C++ says hello" << endl;

	/* Declare a Lua State, open the Lua State and load the libraries (see above). */
	lua_State *l;
	l = lua_open();

	/*
	load all needed lua libraries
	or just call them in sequence, check lualib.h
	*/

	openlualibs(l);

	// register global function my_function
	lua_register(l, "my_function", my_function);

	game.RegisterLuaInterface(l);

	int rv = 0;

	// this is how you execute inline lua scripts
	rv = luaL_dostring(l, inline_script);
	if (rv != 0) return luaL_error(l, "Unable to execute inline script");

	// this is how you execute external lua script files
	rv = luaL_dofile(l, "script.lua");
	if (rv != 0) return MyLuaError(l);

	CallLuaFunc(l);

	// check if we done anything bad by printing the number of things on the stack, this should be 0
	cerr << "top after CallLuaFunc: " << lua_gettop(l) << endl;

	int time = 0;
	while(CallGameTick(l, time++))
	{
		game.PrintValue(l);
	}

	/* Remember to destroy the Lua State */
	lua_close(l);

	cout << "Closing down" << endl;
	return 0;
}