#include "game.h"

#include <string>
#include <iostream>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

using namespace std;

int game_print(lua_State *l)
{
	string output;
	int argc = lua_gettop(l);

	for ( int n=1; n<=argc; ++n ) {
		output += lua_tostring(l, n);
	}

	cout << "Game:print: " << output.c_str() << endl;
	return 0; // number of return values
}

int game_version(lua_State* l)
{
	int argc = lua_gettop(l);
	lua_pushstring(l, "0.1");
	return 1; // number of return values
}

// global functions
static const luaL_reg game_funcs[] = 
{
	{ "print",			game_print },
	{ "version",		game_version },
	{ NULL,				NULL }
};

int Game::lua_getproperty(lua_State *l)
{
	int argc = lua_gettop(l);
	if (argc != 1) luaL_error(l, "getproperty needs one arguments");
	string key = lua_tostring(l, -1);
	Game *g = (Game*)lua_touserdata(l, lua_upvalueindex(1));
	lua_pushstring(l, g->GetProperty(key).c_str()); // push the return value
	return 1;
}

int Game::lua_setproperty(lua_State *l)
{
	int argc = lua_gettop(l);
	if (argc != 2) luaL_error(l, "setproperty needs two arguments");
	string key = lua_tostring(l, 1);
	string value = lua_tostring(l, 2);
	Game *g = (Game*)lua_touserdata(l, lua_upvalueindex(1));
	g->SetProperty(key, value);
	cerr << "top after lua_setproperty: " << lua_gettop(l) << endl;	
	return 0;
}

int Game::RegisterLuaInterface(lua_State *l)
{
	luaL_register(l, "game", game_funcs);
	lua_settop(l, 0); // reset stack after registering.. for some reason we can get junk on the stack
	
	lua_getglobal(l, "game"); // lets modify the just registered table with some special things of our own

	// register some c++ functions with a special upvalue pointing to this instance
	// we need this to call back to this instance later, because of the c api
	lua_pushstring(l, "getproperty"); 
	lua_pushlightuserdata(l, (void*)this); 
	lua_pushcclosure(l, Game::lua_getproperty, 1); 
	lua_settable(l, -3);
	
	lua_pushstring(l, "setproperty"); 
	lua_pushlightuserdata(l, (void*)this); 
	lua_pushcclosure(l, Game::lua_setproperty, 1); 
	lua_settable(l, -3);
	
	lua_settop(l, 0); // reset the stack
	
	return 1;
}

void Game::SetProperty(std::string key, std::string value)
{
	cout << "setting property: " << key << "=" << value << endl;
	mValues[key] = value;
}

std::string Game::GetProperty(std::string key)
{
	tValues::iterator finder = mValues.find(key);
	if (finder != mValues.end()) return finder->second;
	return "";
}

void Game::PrintValue(lua_State *l)
{
	lua_getglobal(l, "i");
	int i = lua_tonumber(l,1);
	lua_pop(l, 1);
	cout << "The lua variable i = " << i << endl;
}
