#include <stdio.h>
#include <string.h>
#include <lauxlib.h>

static void l_phpserz_serialize_value(lua_State *L, int pos, luaL_Buffer *B) {
  switch (lua_type(L, pos)) {
  case LUA_TNIL:
		luaL_addstring(B, "N;");
    break;
  case LUA_TNUMBER: {
    const char *num;
    lua_pushvalue(L, pos);
    num = lua_tostring(L, -1);
		if (strcmp(num, "-inf") == 0) {
			luaL_addstring(B, "d:-INF;");
		}
		else if (strcmp(num, "inf") == 0) {
			luaL_addstring(B, "d:INF;");
		}
		else if (strchr(num, '.') != NULL) {
      luaL_addlstring(B, "d:", 2);
			luaL_addstring(B, num);
      luaL_addlstring(B, ";", 1);
		}
		else {
      luaL_addlstring(B, "i:", 2);
			luaL_addstring(B, num);
      luaL_addlstring(B, ";", 1);
		}
		lua_pop(L, 1);
    break;
  }
	case LUA_TBOOLEAN:
		if (lua_toboolean(L, pos)) {
			luaL_addstring(B, "b:1;");
		}
		else {
			luaL_addstring(B, "b:0;");
		}
		break;
	case LUA_TSTRING: {
    size_t len;
		const char *str;
    const char *num;
		lua_pushvalue(L, pos); // work on a copy of the string
		str = lua_tolstring(L, -1, &len);
    luaL_addlstring(B, "s:", 2);
    lua_pushnumber(L, len);
    num = lua_tostring(L, -1);
	  luaL_addstring(B, num);
    luaL_addlstring(B, ":\"", 2);
	  luaL_addlstring(B, str, len);
    luaL_addlstring(B, "\";", 2);
		break;
  }
  case LUA_TTABLE: {
    const char *num;
    const char *str;
    int counter = 0;
	  luaL_Buffer TB;
	  luaL_buffinit(L, &TB);
    luaL_addlstring(B, "a:", 2);

    /* table is in the stack at index 'pos' */
    lua_pushnil(L);  /* first key */
    while (lua_next(L, pos) != 0) {
			/* push key-value pair to stack base */
			lua_insert(L, 1);
			lua_insert(L, 1);

	    l_phpserz_serialize_value(L, 1, &TB);
	    l_phpserz_serialize_value(L, 2, &TB);
			/* restore key to stack top */
			lua_pushvalue(L, 1);
			lua_remove(L, 1);
			/* pop value */
			lua_remove(L, 1);
      counter++;
    }
    lua_pushnumber(L, counter);
    num = lua_tostring(L, -1);
	  luaL_addstring(B, num);
    luaL_addlstring(B, ":{", 2);
	  luaL_pushresult(&TB);
    str = lua_tostring(L, -1);
    luaL_addstring(B, str);
    luaL_addlstring(B, "}", 1);
    break;
  }
  default:
		lua_pushstring(L, "Cannot serialize data to php");
		lua_error(L);
  }
}

static int l_phpserz_serialize(lua_State *L) {
	luaL_Buffer B;
	luaL_checkany(L, -1);
	luaL_buffinit(L, &B);
	l_phpserz_serialize_value(L, lua_gettop(L), &B);
	luaL_pushresult(&B);
	return 1;
}

static const struct luaL_Reg phpserzlib[] = {
	{ "serialize", l_phpserz_serialize },
	{ NULL, NULL }
};

int luaopen_phpserz(lua_State *L) {
	luaL_register(L, "phpserz", phpserzlib);
	return 1;
}
