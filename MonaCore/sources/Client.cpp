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

#include "Mona/Client.h"
#include "Mona/SplitReader.h"

using namespace std;


namespace Mona {

Client::Client(const char* protocol) :
	protocol(protocol), _pData(NULL), connection(Time::Now()),
	disconnection(0), _pNetStats(&Net::Stats::Null()), _pWriter(&Writer::Null()),
	_rttvar(0), _rto(Net::RTO_INIT), _ping(0) {
}
Client::Client() : // class children has to assign "protocol" field itself!
	_pData(NULL), connection(0),
	disconnection(Time::Now()), _pNetStats(&Net::Stats::Null()), _pWriter(&Writer::Null()),
	_rttvar(0), _rto(Net::RTO_INIT), _ping(0) {
}

Parameters::const_iterator Client::setProperty(const string& key, string&& value, DataReader& parameters) {
	StringReader valueReader(value.data(), value.size());
	SplitReader reader(valueReader, parameters);
	return onSetProperty(key, reader) ? _properties.emplace(key, move(value)).first : _properties.end();
}
bool Client::eraseProperty(const string& key) {
	if (!onSetProperty(key, DataReader::Null()))
		return false;
	_properties.erase(key);
	return NULL;
}


UInt16 Client::setPing(UInt64 value) {
	if (value == 0)
		value = 1;
	else if (value > 0xFFFF)
		value = 0xFFFF;

	// Smoothed Round Trip time https://tools.ietf.org/html/rfc2988

	if (!_rttvar)
		_rttvar = value / 2.0;
	else
		_rttvar = ((3*_rttvar) + abs(_ping - UInt16(value)))/4.0;

	if (_ping == 0)
		_ping = UInt16(value);
	else if (value != _ping)
		_ping = UInt16((7*_ping + value) / 8.0);

	_rto = (UInt32)(_ping + (4*_rttvar) + 200);
	if (_rto < Net::RTO_MIN)
		_rto = Net::RTO_MIN;
	else if (_rto > Net::RTO_MAX)
		_rto = Net::RTO_MAX;

	return _ping;
}


} // namespace Mona
