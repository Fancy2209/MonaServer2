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

#include "Mona/HTTP/HTTPFileSender.h"

using namespace std;


namespace Mona {


HTTPFileSender::HTTPFileSender(const shared<const HTTP::Header>& pRequest, const shared<Socket>& pSocket,
	const Path& file, Parameters& properties) : HTTPSender("HTTPFileSender", pRequest, pSocket),
		File(file, File::MODE_READ), _properties(move(properties)), _mime(MIME::TYPE_UNKNOWN),
		_pos(0), _step(properties.count()), _stage(0) {
		_result = _properties.begin(); // do it here to get compatible _properties.begin() and not properties.begin()
		_protocol = pSocket->isSecure() ? "https://" : "http://";
}


bool HTTPFileSender::load(Exception& ex) {
	if (loaded())
		return true;

	DEBUG(peerAddress(), " GET 200 ", pRequest->path, File::name());
	if (File::load(ex)) {
		/// not modified if there is no parameters file (impossible to determinate if the parameters have changed since the last request)
		if (!_properties.count() && pRequest->ifModifiedSince >= lastChange()) {
			// NOT MODIFIED
			DEBUG(peerAddress(), " GET 304 ", pRequest->path, File::name());
			send(HTTP_CODE_304);
			return false;
		}
		return true;
	}

	
	string buffer;
	if (ex.cast<Ex::Unfound>()) {
		ex = nullptr;
		// File doesn't exists but maybe folder exists?
		if (FileSystem::Exists(MAKE_FOLDER(path()))) {
			/// Redirect to the real folder path
			// Full URL required here relating RFC2616 section 10.3.3
			String::Assign(buffer, _protocol, pRequest->host, pRequest->path, File::name(), '/');
			HTTP_BEGIN_HEADER(this->buffer())
				HTTP_ADD_HEADER("Location", buffer);
			HTTP_END_HEADER
			HTML_BEGIN_COMMON_RESPONSE(this->buffer(), "Moved Permanently")
				String::Append(this->buffer(), "The document has moved <a href=\"", buffer, "\">here</a>.");
			HTML_END_COMMON_RESPONSE(buffer)
			send(HTTP_CODE_301, MIME::TYPE_TEXT, "html; charset=utf-8");
		} else
			sendError(HTTP_CODE_404, "The requested URL ", pRequest->path, File::name(), " was not found on the server");
	} else
		sendError(HTTP_CODE_401, "Impossible to open ", pRequest->path, File::name(), " file");

	return false;
}

UInt32 HTTPFileSender::decode(shared<Buffer>& pBuffer, bool end) {
	
	deque<Packet> packets;
	Packet packet(pBuffer); // capture and hold buffer until end of life of packets

	UInt32 size;
	if (!_properties.count()) {
		size = packet.size();
		packets.emplace_back(packet.buffer());
	} else // properties => want parse files to replace properties!
		size = generate(packet, packets);

	// HEADER
	if (!_mime) {
		_mime = MIME::Read(self, _subMime);
		if (!_mime) {
			_mime = MIME::TYPE_APPLICATION;
			_subMime = "octet-stream";
		}
		if (!send(HTTP_CODE_200, _mime, _subMime, end ? size : UINT64_MAX)) {
			this->end(); // to avoid to read again
			return 0;
		}
	}
	// CONTENT
	if (pRequest->type != HTTP::TYPE_HEAD) {
		for (Packet& packet : packets) {
			if (!send(packet)) {
				this->end(); // to avoid to read again
				return 0;
			}
		}
		if(!end)
			return HTTPSender::flushing() ? 0 : 0xFFFF; // wait next!
	}
	// END
	this->end();
	return 0;
}

const string* HTTPFileSender::search(char c) {
	if (!c) {
		// reset
		const string* pResult = (_result == _properties.end() || _result->first.size() != _pos) ? NULL : &_result->second;
		_pos = 0;
		_result = _properties.begin();
		_step = _properties.count();
		return pResult;
	}

	while (_result != _properties.end()) {
		Int16 diff = _result->first.size() > _pos ? (c - _result->first[_pos]) : 1;
		if (!diff) {
			++_pos; // wait next
			return &_result->second;
		}
		if (diff < 0 && _result == _properties.begin()) {
			_result = _properties.end();
			return NULL;
		}
		auto it = _result;
		do {
			if (!(_step /= 2)) {
				_result = _properties.end();
				return NULL;
			}
			advance(it, (diff & 0x8000) ? -_step : _step);
			diff = _result->first.compare(0, _pos, it->first, 0, _pos);
		} while (diff);
		_result = it;
	};
	return NULL;
}

UInt32 HTTPFileSender::generate(const Packet& packet, deque<Packet>& packets) {

	// iterate on content to replace "<% key %>" fields
	
	const UInt8* begin = packet.data();
	const UInt8* cur = begin;
	const UInt8* end = begin + packet.size();
	UInt32		 size(0);

	while (cur<end) {

		if (*cur == '<') {
			// reset to get "<% name %>" from "<% test <% name %>" (more sense rather searching "test <% name" in the properties!
			if(_stage>2)
				search(0);
			_stage = 1;
			++cur;
			continue;
		}

		switch (_stage) {
			case 1:
				if (*cur == '%') {
					if ((cur - 1) > begin) {
						packets.emplace_back(packet, begin, cur - begin - 1);
						size += packets.back().size();
					}
					++_stage;
				} else
					--_stage;
				break;
			case 2:
				if (isspace(*cur))
					break;
				++_stage;
			case 3:
				if (*cur == '%')
					++_stage;
				else if (isspace(*cur))
					_stage = 5;
				else
					search(*cur);
				break;
			case 4:
				if (*cur == '>') {
					const string* pValue = search(0);
					if (pValue) {
						packets.emplace_back(pValue->data(), pValue->size());
						size += pValue->size();
					}
					begin = cur + 1;
					_stage = 0;
					break;
				}
				--_stage;
				search('%');
				continue;
			default:
				if (_stage) { // _stage>4 => spaces!
					if (!isspace(*cur)) {
						while (--_stage > 4)
							search(' ');
						_stage = 3;
						continue;
					}
					++_stage;
				}
				break;
		}
		++cur;
	}

	// Add rest size?
	if (cur <= begin)
		return size;
	packets.emplace_back(packet, begin, cur - begin);
	return size += cur - begin;
}

} // namespace Mona
