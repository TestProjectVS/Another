#ifndef __GAME_H__
#define __GAME_H__

#include <string>
#include <map>

/*	there should only be one game instance
	but as singletons are evil, let the user of it manage it, maybe
*/


// We don't want to show the whole world the lua_State type, so just forward declare it
struct lua_State;

class Game
{
public:
	typedef std::map<std::string, std::string> tValues;

	int RegisterLuaInterface(lua_State *l);
	void SetProperty(std::string key, std::string value);
	std::string GetProperty(std::string key);
	void PrintValue(lua_State *l);
private:
	static int lua_getproperty(lua_State *L);
	static int lua_setproperty(lua_State *L);
	tValues mValues;
};

#endif