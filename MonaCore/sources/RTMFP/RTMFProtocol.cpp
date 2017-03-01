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

#include "Mona/RTMFP/RTMFProtocol.h"
#include "Mona/Util.h"
#include "Mona/ServerAPI.h"
#include "Mona/RTMFP/RTMFPSession.h"

using namespace std;


namespace Mona {


RTMFProtocol::RTMFProtocol(const char* name, ServerAPI& api, Sessions& sessions) : UDProtocol(name, api, sessions) {
	memcpy(_certificat, "\x01\x0A\x41\x0E", 4);
	Util::Random(&_certificat[4], 64);
	memcpy(&_certificat[68], "\x02\x15\x02\x02\x15\x05\x02\x15\x0E", 9);

	setNumber("port", 1935);
	setNumber("keepalivePeer",   10);
	setNumber("keepaliveServer", 15);

	_onHandshake = [this](RTMFP::Handshake& handshake) {
		BinaryReader reader(handshake.data(), handshake.size());

		// Fill peer infos
		shared<Peer> pPeer(new Peer(this->api, "RTMFP"));
		string serverAddress;
		{
			const char* url = STR reader.current();
			reader.next(reader.available() - 16);
			String::Scoped scoped(STR reader.current());
			Util::UnpackUrl(url, serverAddress, (string&)pPeer->path, (string&)pPeer->query);
		}
		pPeer->setAddress(handshake.address);
		pPeer->setServerAddress(serverAddress);
		Util::UnpackQuery(pPeer->query, pPeer->properties());

		// prepare response
		
		shared<Buffer> pBuffer;
		BinaryWriter writer(initBuffer(pBuffer));
		writer.write8(16).write(reader.current(), 16); // tag

		SocketAddress redirection;
		if (pPeer->onHandshake(redirection)) {
			// redirection
			RTMFP::WriteAddress(writer, redirection, RTMFP::LOCATION_REDIRECTION);
			send(0x71, pBuffer, handshake.address, handshake.pResponse);
		}else {
			// Create session and write id in cookie response!
			writer.write8(RTMFP::SIZE_COOKIE);
			writer.write32(this->sessions.create<RTMFPSession>(*this, this->api, pPeer).id());
			writer.writeRandom(RTMFP::SIZE_COOKIE - 4);
			// instance id (certificat in the middle)
			writer.write(_certificat, sizeof(_certificat));
			send(0x70, pBuffer, handshake.address, handshake.pResponse);
		}
	};
	_onSession = [this](shared<RTMFP::Session>& pSession) {
		RTMFPSession* pClient = this->sessions.find<RTMFPSession>(pSession->id);
		if (pClient)
			pClient->init(pSession);
		else
			ERROR("Session ", pSession->id, " unfound");
	};
}

shared<Socket::Decoder>	RTMFProtocol::newDecoder() {
	shared<RTMFPDecoder> pDecoder(new RTMFPDecoder(api.handler, api.threadPool));
	pDecoder->onSession = _onSession;
	pDecoder->onHandshake = _onHandshake;
	return pDecoder;
}

RTMFProtocol::~RTMFProtocol() {
	_onHandshake = nullptr;
	_onSession = nullptr;
	if (groups.empty())
		return;
	CRITIC("Few peers are always member of deleting groups");
	for (auto& it : groups)
		delete it.second;
}

bool RTMFProtocol::load(Exception& ex) {

	if (!UDProtocol::load(ex))
		return false;
	
	if (getNumber<UInt16,10>("keepalivePeer") < 5) {
		WARN("Value of RTMFP.keepalivePeer can't be less than 5 sec")
		setNumber("keepalivePeer", 5);
	}
	if (getNumber<UInt16,15>("keepaliveServer") < 5) {
		WARN("Value of RTMFP.keepaliveServer can't be less than 5 sec")
		setNumber("keepaliveServer", 5);
	}

	return true;
}

Buffer& RTMFProtocol::initBuffer(shared<Buffer>& pBuffer) {
	pBuffer.reset(new Buffer(6));
	return BinaryWriter(*pBuffer).write8(0x0B).write16(RTMFP::TimeNow()).next(3).buffer();
}

void RTMFProtocol::send(UInt8 type, shared<Buffer>& pBuffer, const SocketAddress& address, shared<Packet>& pResponse) {
	struct Sender : Runner, virtual Object {
		Sender(const shared<Socket>& pSocket, shared<Buffer>& pBuffer, const SocketAddress& address, shared<Packet>& pResponse) : _address(address), _pResponse(move(pResponse)), _pSocket(pSocket), _pBuffer(move(pBuffer)), Runner("RTMFPProtocolSender") {}
	private:
		bool run(Exception& ex) {
			Packet packet(RTMFP::Engine::Encode(_pBuffer, 0, _address));
			_pResponse->set(move(packet));
			_pResponse.reset(); // free response before sending to avoid "38 before 30 handshake" error
			RTMFP::Send(*_pSocket, packet, _address);
			return true;
		}
		shared<Buffer>	_pBuffer;
		shared<Socket>	_pSocket;
		SocketAddress	_address;
		shared<Packet>  _pResponse;
	};
	Exception ex;
	BinaryWriter(pBuffer->data() + 9, 3).write8(type).write16(pBuffer->size() - 12);
	AUTO_ERROR(api.threadPool.queue(ex, make_shared<Sender>(socket(), pBuffer, address, pResponse)), "RTMFPSend");
}


} // namespace Mona
