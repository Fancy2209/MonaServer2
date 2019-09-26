/*
This file is a part of MonaSolutions Copyright 2017
mathieu.poux[a]gmail.com
jammetthomas[a]gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License received along this program for more
details (or else see http://www.gnu.org/licenses/).

*/

#include "LUANetStats.h"
#include "LUAMap.h"
#include "LUAWriter.h"
#include "Mona/Client.h"


using namespace std;
using namespace Mona;


Net::Stats&   LUANetStats<Client>::NetStats(Client& client) { return client; }
Writer&		  LUAWriter<Client>::Writer(Client& client) { return client.writer(); }

static int __tostring(lua_State *pState) {
	SCRIPT_CALLBACK(Client, client);
		SCRIPT_WRITE_DATA(client.id, Entity::SIZE);
	SCRIPT_CALLBACK_RETURN;
}

static int ping(lua_State *pState) {
	SCRIPT_CALLBACK(Client, client);
		SCRIPT_WRITE_INT(client.ping());
	SCRIPT_CALLBACK_RETURN;
}
static int rto(lua_State *pState) {
	SCRIPT_CALLBACK(Client, client);
		SCRIPT_WRITE_INT(client.rto());
	SCRIPT_CALLBACK_RETURN;
}

template<> void Script::ObjInit(lua_State* pState, Mona::Client& client) {
	struct Mapper : LUAMap<const Parameters>::Mapper<Mona::Client>, virtual Object {
		Mapper(Mona::Client& client, lua_State* pState) : LUAMap<const Parameters>::Mapper<Mona::Client>(client, client.properties(), pState) {}
		Parameters::const_iterator set(const string& key, string&& value, DataReader& parameters) { return obj.setProperty(key, move(value), parameters); }
		bool erase(const Parameters::const_iterator& first, const Parameters::const_iterator& last) {
			for (auto it = first; it != last; ++it) {
				if (!obj.eraseProperty(it->first))
					return false;
			}
			return true;
		}
	};

	Script::AddComparator<Mona::Client>(pState);

	// properties
	lua_pushliteral(pState, "properties");
	AddObject(pState, client.properties());
	lua_getmetatable(pState, -1);
	lua_setmetatable(pState, -6); // metatable of parameters becomes metatable of __index of subscription object!

	SCRIPT_BEGIN(pState)
		SCRIPT_DEFINE_FUNCTION("__call", &LUAMap<const Parameters>::Call<Mapper>);
		SCRIPT_DEFINE_FUNCTION("__pairs", &LUAMap<const Parameters>::Pairs<Mapper>);
		SCRIPT_DEFINE_DATA("id", client.id, Entity::SIZE)
		SCRIPT_DEFINE_STRING("protocol", client.protocol);
		SCRIPT_DEFINE_DOUBLE("connection", client.connection);
		SCRIPT_DEFINE_STRING("path", client.path);
		SCRIPT_DEFINE_STRING("query", client.query);
		SCRIPT_DEFINE("address", NewObject(pState, new SocketAddress(client.address)));
		SCRIPT_DEFINE("serverAddress", NewObject(pState, new SocketAddress(client.serverAddress)));
		SCRIPT_DEFINE("writer", AddObject(pState, client.writer()));

		SCRIPT_DEFINE_FUNCTION("__tostring", &__tostring);
		SCRIPT_DEFINE_FUNCTION("ping", &ping);
		SCRIPT_DEFINE_FUNCTION("rto", &rto);
		SCRIPT_DEFINE_WRITER(Mona::Client);
		SCRIPT_DEFINE_NETSTATS(Mona::Client);

	SCRIPT_END
}
template<> void Script::ObjClear(lua_State *pState, Mona::Client& client) {
	RemoveObject(pState, client.properties());
	RemoveObject(pState, client.writer());
}
