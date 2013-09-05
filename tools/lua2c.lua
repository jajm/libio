local description = [=[
Usage: lua lua2c.lua [-o output_name] [-f function_name] filename

]=]

local output, funcname, filename

local a = table.remove(arg, 1)
while a do
	if a == "-o" then
		output = table.remove(arg, 1)
	elseif a == "-f" then
		funcname = table.remove(arg, 1)
	else
		filename = a
	end
	a = table.remove(arg, 1)
end

if not filename then
	io.stderr:write("Error: filename not given\n")
	io.stderr:write(description)
	return
end

if not output then
	output = filename
end

if not funcname then
	funcname = filename
end
funcname = string.gsub(funcname, "[^a-zA-Z0-9_]", "_")

local content = string.dump(assert(loadfile(filename)))

local dump do
  local numtab={}; for i=0,255 do numtab[string.char(i)]=("%3d,"):format(i) end
  function dump(str)
    return (str:gsub(".", numtab):gsub(("."):rep(80), "%0\n"))
  end
end

local headerfilename = output..".h"
local headermacro = string.gsub(headerfilename, "[^a-zA-Z0-9_]", "_") .. "_included"
local headerfile = io.open(headerfilename, "w")
headerfile:write(string.format([=[
#ifndef %s
#define %s

#include <lua.h>

int %s(lua_State *L);

#endif /* ! %s */
]=], headermacro, headermacro, funcname, headermacro))
headerfile:close()

local sourcefile = io.open(output..".c", "w")
sourcefile:write(string.format([=[
/* code automatically generated by lua2c -- DO NOT EDIT */
#include <lua.h>
#include <lauxlib.h>
int %s(lua_State *L)
{
	int ret;

	static const unsigned char B1[] = {
]=], funcname, filename), dump(content), string.format([=[
	};

	ret = luaL_loadbuffer(L, (const char*)B1, sizeof(B1), %q);
	if (!ret) {
		ret = lua_pcall(L, 0, LUA_MULTRET, 0);
	}

	return ret;
}
]=], filename))
sourcefile:close()