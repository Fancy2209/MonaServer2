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
#include "Mona/TCPSession.h"
#include "Mona/FlashMainStream.h"
#include "Mona/RTMP/RTMPWriter.h"
#include "Mona/RTMP/RTMPDecoder.h"

namespace Mona {

struct RTMPSession : TCPSession, virtual Object {
	RTMPSession(Protocol& protocol, const shared<Socket>& pSocket);

private:
	bool			manage();
	void			flush();
	void			kill(Int32 error=0, const char* reason = NULL) override;
	
	Socket::Decoder*			newDecoder();
	RTMPDecoder::OnRequest	   _onRequest;

	bool						_first;
	std::map<UInt32,RTMPWriter>	_writers;
	RTMPWriter					_controller;
	FlashMainStream				_mainStream;
	shared<RC4_KEY>				_pEncryptKey;
};



} // namespace Mona
