/*
This file is a part of MonaSolutions Copyright 2017
mathieu.poux[a]gmail.com
jammetthomas[a]gmail.com

This program is free software: you can redistribute it and/or
modify it under the terms of the the Mozilla Public License v2.0.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
Mozilla Public License v. 2.0 received along this program for more
details (or else see http://mozilla.org/MPL/2.0/).

*/

#include "Mona/Mona.h"
#include "Mona/String.h"
#include <mutex>
#if !defined(_WIN32)
#include <cxxabi.h>
#include <signal.h>
#endif


namespace Mona {

using namespace std;

const UInt16 ASCII::_CharacterTypes[128] =  {
	/* 00 . */ CONTROL,
	/* 01 . */ CONTROL,
	/* 02 . */ CONTROL,
	/* 03 . */ CONTROL,
	/* 04 . */ CONTROL,
	/* 05 . */ CONTROL,
	/* 06 . */ CONTROL,
	/* 07 . */ CONTROL,
	/* 08 . */ CONTROL,
	/* 09 . */ CONTROL | SPACE | BLANK,
	/* 0a . */ CONTROL | SPACE,
	/* 0b . */ CONTROL | SPACE,
	/* 0c . */ CONTROL | SPACE,
	/* 0d . */ CONTROL | SPACE,
	/* 0e . */ CONTROL,
	/* 0f . */ CONTROL,
	/* 10 . */ CONTROL,
	/* 11 . */ CONTROL,
	/* 12 . */ CONTROL,
	/* 13 . */ CONTROL,
	/* 14 . */ CONTROL,
	/* 15 . */ CONTROL,
	/* 16 . */ CONTROL,
	/* 17 . */ CONTROL,
	/* 18 . */ CONTROL,
	/* 19 . */ CONTROL,
	/* 1a . */ CONTROL,
	/* 1b . */ CONTROL,
	/* 1c . */ CONTROL,
	/* 1d . */ CONTROL,
	/* 1e . */ CONTROL,
	/* 1f . */ CONTROL,
	/* 20   */ SPACE | BLANK | PRINT,
	/* 21 ! */ PUNCT | GRAPH | PRINT,
	/* 22 " */ PUNCT | GRAPH | PRINT,
	/* 23 # */ PUNCT | GRAPH | PRINT,
	/* 24 $ */ PUNCT | GRAPH | PRINT,
	/* 25 % */ PUNCT | GRAPH | PRINT,
	/* 26 & */ PUNCT | GRAPH | PRINT,
	/* 27 ' */ PUNCT | GRAPH | PRINT,
	/* 28 ( */ PUNCT | GRAPH | PRINT,
	/* 29 ) */ PUNCT | GRAPH | PRINT,
	/* 2a * */ PUNCT | GRAPH | PRINT,
	/* 2b + */ PUNCT | GRAPH | PRINT | B64,
	/* 2c , */ PUNCT | GRAPH | PRINT,
	/* 2d - */ PUNCT | GRAPH | PRINT | XML | B64URL,
	/* 2e . */ PUNCT | GRAPH | PRINT | XML,
	/* 2f / */ PUNCT | GRAPH | PRINT | B64,
	/* 30 0 */ DIGIT | HEXDIGIT | GRAPH | PRINT | XML | B64 | B64URL,
	/* 31 1 */ DIGIT | HEXDIGIT | GRAPH | PRINT | XML | B64 | B64URL,
	/* 32 2 */ DIGIT | HEXDIGIT | GRAPH | PRINT | XML | B64 | B64URL,
	/* 33 3 */ DIGIT | HEXDIGIT | GRAPH | PRINT | XML | B64 | B64URL,
	/* 34 4 */ DIGIT | HEXDIGIT | GRAPH | PRINT | XML | B64 | B64URL,
	/* 35 5 */ DIGIT | HEXDIGIT | GRAPH | PRINT | XML | B64 | B64URL,
	/* 36 6 */ DIGIT | HEXDIGIT | GRAPH | PRINT | XML | B64 | B64URL,
	/* 37 7 */ DIGIT | HEXDIGIT | GRAPH | PRINT | XML | B64 | B64URL,
	/* 38 8 */ DIGIT | HEXDIGIT | GRAPH | PRINT | XML | B64 | B64URL,
	/* 39 9 */ DIGIT | HEXDIGIT | GRAPH | PRINT | XML | B64 | B64URL,
	/* 3a : */ PUNCT | GRAPH | PRINT | XML,
	/* 3b ; */ PUNCT | GRAPH | PRINT,
	/* 3c < */ PUNCT | GRAPH | PRINT,
	/* 3d = */ PUNCT | GRAPH | PRINT,
	/* 3e > */ PUNCT | GRAPH | PRINT,
	/* 3f ? */ PUNCT | GRAPH | PRINT,
	/* 40 @ */ PUNCT | GRAPH | PRINT,
	/* 41 A */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 42 B */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 43 C */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 44 D */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 45 E */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 46 F */ HEXDIGIT | ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 47 G */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 48 H */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 49 I */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 4a J */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 4b K */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 4c L */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 4d M */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 4e N */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 4f O */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 50 P */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 51 Q */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 52 R */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 53 S */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 54 T */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 55 U */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 56 V */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 57 W */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 58 X */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 59 Y */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 5a Z */ ALPHA | UPPER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 5b [ */ PUNCT | GRAPH | PRINT,
	/* 5c \ */ PUNCT | GRAPH | PRINT,
	/* 5d ] */ PUNCT | GRAPH | PRINT,
	/* 5e ^ */ PUNCT | GRAPH | PRINT,
	/* 5f _ */ PUNCT | GRAPH | PRINT | XML | B64URL,
	/* 60 ` */ PUNCT | GRAPH | PRINT,
	/* 61 a */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 62 b */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 63 c */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 64 d */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 65 e */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 66 f */ HEXDIGIT | ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 67 g */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 68 h */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 69 i */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 6a j */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 6b k */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 6c l */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 6d m */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 6e n */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 6f o */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 70 p */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 71 q */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 72 r */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 73 s */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 74 t */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 75 u */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 76 v */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 77 w */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 78 x */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 79 y */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 7a z */ ALPHA | LOWER | GRAPH | PRINT | XML | B64 | B64URL,
	/* 7b { */ PUNCT | GRAPH | PRINT,
	/* 7c | */ PUNCT | GRAPH | PRINT,
	/* 7d } */ PUNCT | GRAPH | PRINT,
	/* 7e ~ */ PUNCT | GRAPH | PRINT,
	/* 7f DEL */ CONTROL
};

const string& TypeOf(const type_info& info) {
	static map<size_t, string> Types;
	static std::atomic_flag  _FastMutex; // keep mutex because and not thread_local because can be used for Runner!
	while (_FastMutex.test_and_set(std::memory_order_acquire))
		this_thread::yield();
	string& type = Types[info.hash_code()];
	if (!type.empty()) {
		_FastMutex.clear(std::memory_order_release);
		return type;
	}
#if defined(_WIN32)
	type = info.name();
#else
	int status = -4;
	char* name = abi::__cxa_demangle(info.name(), NULL, NULL, &status);
	type = name ? name : info.name();
	free(name);
#endif
	for (size_t i = 0; (i + 4) < type.size();) {
		if (String::ICompare(&type[i], EXPAND("Mona::")) == 0) {
			type.erase(i, 6);
			continue;
		}
		if (String::ICompare(&type[i], EXPAND("std::")) == 0) {
			type.erase(i, 5);
			continue;
		}
		if (String::ICompare(&type[i], EXPAND("class ")) == 0) {
			type.erase(i, 6);
			continue;
		}
		if (String::ICompare(&type[i], EXPAND("struct ")) == 0) {
			type.erase(i, 7);
			continue;
		}
		++i;
	}
	_FastMutex.clear(std::memory_order_release);
	return type;
}

const char *strrstr(const char* where, const char* what) {
	size_t  whereSize = strlen(where);
	size_t  whatSize = strlen(what);
	const char* value;
	if (whatSize > whereSize)
		return NULL;
	for (value = where + whereSize - whatSize; value >= where; --value)
		if (strncmp(value, what, whatSize) == 0)
			return value;
	return NULL;
}

const char* strrpbrk(const char* value, const char* markers) {
	const char* begin(value);
	value += strlen(value);
	while (--value >= begin) {
		const char* marker(markers);
		while (*marker) {
			if (*marker++ == *value)
				return value;
		}
	}
	return NULL;
}


#if defined(_WIN32) && defined(_DEBUG)

#define FALSE   0
#define TRUE    1


_CRT_REPORT_HOOK prevHook;
 
int reportingHook(int reportType, char* userMessage, int* retVal) {
  // This function is called several times for each memory leak.
  // Each time a part of the error message is supplied.
  // This holds number of subsequent detail messages after
  // a leak was reported
  const int numFollowupDebugMsgParts = 2;
  static bool ignoreMessage = false;
  static int debugMsgPartsCount = numFollowupDebugMsgParts;
  static bool firstMessage = true;

  if( strncmp(userMessage,"Detected memory leaks!\n", 10)==0) {
	ignoreMessage = true;
	return TRUE;
  } else if(strncmp(userMessage,"Dumping objects ->\n", 10)==0) {
	  return TRUE;
  } else if (ignoreMessage) {

    // check if the memory leak reporting ends
    if (strncmp(userMessage,"Object dump complete.\n", 10) == 0) {
		  _CrtSetReportHook(prevHook);
		  ignoreMessage = false;
    }

	if(debugMsgPartsCount++ < numFollowupDebugMsgParts)
		// give it back to _CrtDbgReport() to be printed to the console
		return FALSE;

 
    // something from our own code?
	if(strstr(userMessage, ".cpp") || strstr(userMessage, ".h") || strstr(userMessage, ".c")) {
		if(firstMessage) {
			OutputDebugStringA("Detected memory leaks!\nDumping objects ->\n");
			firstMessage = false;
		}
		
		debugMsgPartsCount = 0;
      // give it back to _CrtDbgReport() to be printed to the console
       return FALSE;
    } else
		return TRUE;

 } else
    // give it back to _CrtDbgReport() to be printed to the console
    return FALSE;

  
};
 
void DetectMemoryLeak() {
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
  //change the report function to only report memory leaks from program code
  prevHook = _CrtSetReportHook(reportingHook);
}

#else
void DetectMemoryLeak() {}
#endif

#if !defined(_WIN32)
struct Init {
	Init() {
		// SIGPIPE sends a signal that if unhandled (which is the default)
		// will crash the process.
		// In order to have sockets behave the same across platforms, it is
		// best to just ignore SIGPIPE all together.
		struct sigaction act;
		memset(&act, 0, sizeof(struct sigaction));
		act.sa_handler = SIG_IGN;
		act.sa_flags = SA_RESTART;
		sigaction(SIGPIPE, &act, NULL);
	}
} _Init;

#endif


} // namespace Mona
