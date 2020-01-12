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

#pragma once

#include "Mona/Mona.h"
#include "Mona/SocketAddress.h"
#include "Mona/SDP.h"
#include "Mona/Time.h"
#include "Mona/DataReader.h"
#include <set>

namespace Mona {

struct Peer;
struct RelayServer;
struct ICE : virtual Object {
	ICE(const Peer& initiator,const Peer& remote);
	virtual ~ICE();

	void			setCurrent(Peer& current);
	UInt16			mediaIndex;

	void			fromSDPLine(const std::string& line,SDPCandidate& publicCandidate);
	void			fromSDPLine(const std::string& line, SDPCandidate& publicCandidate, std::vector<SDPCandidate>& relayCurrentCandidates, std::vector<SDPCandidate>& relayRemoteCandidates) { fromSDPLine(line, publicCandidate, relayCurrentCandidates, relayRemoteCandidates, true); }

	void			reset();
	bool			obsolete() { return _time.isElapsed(120000); }

	// TODO static bool ProcessSDPPacket(DataReader& packet,Peer& current,Writer& currentWriter,Peer& remote,Writer& remoteWriter);

private:
	enum Type {
		INITIATOR,
		REMOTE
	};
	
	void fromSDPLine(const std::string& line,SDPCandidate& publicCandidate,std::vector<SDPCandidate>& relayCurrentCandidates,std::vector<SDPCandidate>& relayRemoteCandidates,bool relayed);
	void fromSDPCandidate(const std::string& candidate,SDPCandidate& publicCandidate,std::vector<SDPCandidate>& relayCurrentCandidates,std::vector<SDPCandidate>& relayRemoteCandidates,bool relayed);

	std::map<UInt16,std::map<UInt8,std::set<SocketAddress> > >	_initiatorAddresses;
	std::map<UInt16,std::map<UInt8,std::set<SocketAddress> > >	_remoteAddresses;
	std::map<UInt16, std::map<UInt8,std::set<UInt16> > >		_relayPorts;
	Time														_time;
	bool														_first;
	Type														_type;
	std::string													_publicHost;

	const Peer&													_initiator;
	const Peer&													_remote;
};


} // namespace Mona
