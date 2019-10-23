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

#include "Mona/UnitTest.h"
#include "Mona/Date.h"
#include "Mona/Timezone.h"
#include "Mona/Util.h"
#include "Mona/Logs.h"

using namespace Mona;
using namespace std;

namespace DateTest {

static String	_Out;
static Date		_Date;



Exception _Ex;

bool Check(Int32 year, UInt8 month, UInt8 day,UInt8 weekDay, UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, Int32 offset) {
	if (offset == Timezone::GMT) {
		if (_Date.offset()!=0 || !_Date.isGMT() || _Date.isDST())
			return false;
	} else if (offset == Timezone::LOCAL) {
		bool isDST(false);
		if (_Date.isDST()) {
			if (Timezone::Offset(_Date, isDST) != _Date.offset() || !isDST)
				return false;
		} else if (Timezone::Offset(_Date,isDST) != _Date.offset() || isDST)
			return false;
		if (_Date.isGMT())
			return false;
	} else if (_Date.offset() != offset || _Date.isGMT())
		return false;

	return _Date.year() == year && _Date.month() == month && _Date.day() == day && _Date.hour() == hour && _Date.minute() == minute && _Date.second() == second && _Date.millisecond() == millisecond && _Date.weekDay() == weekDay;
}

bool Parse(const char * stDate, Int32 year, UInt8 month, UInt8 day,UInt8 weekDay, UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond,Int32 offset,const char* format=NULL) {
	bool bIsParseOk = _Date.update(_Ex,stDate,format);
	if (!bIsParseOk) {
		DEBUG("Error during parsing of ", stDate, ", ",_Ex);
		return false;
	}
	return Check(year, month, day, weekDay, hour, minute, second, millisecond, offset);
}


bool Update(Int32 year, UInt8 month, UInt8 day, UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, Int32 offset,UInt8 weekDay) {
	_Date.update(year,month,day,hour,minute,second,millisecond,offset);
	return Check(year, month, day, weekDay, hour, minute, second, millisecond, offset);
}


ADD_TEST(General) {

	_Date.update();
	Int32 year(_Date.year());

	CHECK(Update(1583, 1, 1, 17, 59, 58, 123, Timezone::LOCAL,6));
	CHECK(!Update(1583, 0, 1, 17, 59, 58, 123, Timezone::GMT,6));
	CHECK(!Update(1583, 1, 0, 17, 59, 58, 123, Timezone::GMT,6));

	CHECK(Update(1970, 1, 1, 0, 0, 0, 0, Timezone::GMT,4) && _Date==0); // unix reference, Thursday
	_Date += 1000000000000; // 2001-09-09 01:46:40 Sunday
	CHECK(_Date == 1000000000000)
	Date date(_Date);
	CHECK(date == _Date);

	date.update(2001,9,9,1,46,40,0,Timezone::GMT);

	CHECK( date == _Date && date.weekDay()==_Date.weekDay() && date.weekDay()==0);
	date.setOffset(2000);
	CHECK(date == (_Date - 2000))

	_Date.update(-101, 12, 31,Timezone::LOCAL);
	UInt32 weekDay(_Date.weekDay()); 
	 
	for (int i = -10; i <= (year+30); ++i) {
		for (int m = 1; m <= 12; ++m) {
			for (int d = 1; d <= 31; ++d) {
				if (d == 31) {
					if (m < 8) {
						if (!(m & 0x01))
							continue;
					} else {
						if (m & 0x01)
							continue;
					}	
				}
				if (m == 2) {
					if (d>29)
						continue;
					if (d == 29 && !Date::IsLeapYear(i))
						continue;
				}
				UInt16 random(Util::Random<UInt16>());
				UInt8 hour(random % 23), minute(random % 59), second(random % 59);
				UInt16 millisecond(random % 1000);
				_Date.update(i, m, d,hour, minute, second, millisecond);
				// 1 day progression (on weekDay)!
				CHECK(Check(i, m, d, (++weekDay) % 7, hour, minute, second, millisecond, Timezone::LOCAL));
				if (_Date == 0) // unix reference, thursday
					CHECK(weekDay == 4);
			}
		}
		
	}

}


ADD_TEST(ParseISO8601) {

	CHECK(Parse("2005-01-08T12:30:00Z", 2005, 1, 8, 6, 12, 30, 0, 0,Timezone::GMT));
	CHECK(_Date.format(Date::FORMAT_ISO8601,_Out.clear()) == "2005-01-08T12:30:00Z");

	CHECK(Parse("2005-01-08T12:30:00+01:00", 2005, 1, 8,6, 12, 30, 0, 0,3600000));
	CHECK(_Date.format(Date::FORMAT_ISO8601,_Out.clear()) == "2005-01-08T12:30:00+01:00");

	CHECK(Parse("2005-01-08T12:30:00-01:00", 2005, 1, 8, 6, 12, 30, 0, 0, -3600000));
	CHECK(_Date.format(Date::FORMAT_ISO8601,_Out.clear()) == "2005-01-08T12:30:00-01:00");

	CHECK(Parse("2005-01-08T12:30:00", 2005, 1, 8, 6, 12, 30, 0, 0, Timezone::LOCAL));

	CHECK(Parse("2005-01-08", 2005, 1, 8,6, 0, 0, 0, 0, Timezone::LOCAL));

	CHECK(Parse("2005", 2005, 1, 1, 6, 0, 0, 0, 0, Timezone::LOCAL,Date::FORMAT_ISO8601));
}

ADD_TEST(ParseISO8601Frac) {
	
	CHECK(Parse("2005-01-08T12:30:00.1Z", 2005, 1, 8,6, 12, 30, 0, 100, Timezone::GMT));
	CHECK(_Date.format(Date::FORMAT_ISO8601_FRAC, _Out.clear()) == "2005-01-08T12:30:00.100Z");
	
	CHECK(Parse("2005-01-08T12:30:00.123+01:00", 2005, 1, 8,6, 12, 30, 0, 123, 3600000));
	CHECK(_Date.format(Date::FORMAT_ISO8601_FRAC, _Out.clear()) == "2005-01-08T12:30:00.123+01:00");
	
	CHECK(Parse("2005-01-08T12:30:00Z", 2005, 1, 8,6, 12, 30, 0, 0, Timezone::GMT));
	CHECK(_Date.format(Date::FORMAT_ISO8601_FRAC, _Out.clear()) == "2005-01-08T12:30:00.000Z");
	
	CHECK(Parse("2005-01-08T12:30:00+01:00", 2005, 1, 8,6, 12, 30, 0, 0, 3600000));
	CHECK(_Date.format(Date::FORMAT_ISO8601_FRAC, _Out.clear()) == "2005-01-08T12:30:00.000+01:00");

	CHECK(Parse("2014-04-22T06:00:00.000+02:00", 2014, 4, 22, 2, 6, 0, 0, 0, 7200000));
	CHECK(_Date.format(Date::FORMAT_ISO8601_FRAC, _Out.clear()) == "2014-04-22T06:00:00.000+02:00");

	CHECK(!_Ex);

	CHECK(Parse("2005-01-08T12:30:00.12345-01:00", 2005, 1, 8,6, 12, 30, 0, 123, -3600000));
	CHECK(_Date.format(Date::FORMAT_ISO8601_FRAC, _Out.clear()) == "2005-01-08T12:30:00.123-01:00");
	
	CHECK(Parse("2010-09-23T16:17:01.2817002+02:00", 2010, 9, 23, 4, 16, 17, 1, 281, 7200000));
	CHECK(_Date.format(Date::FORMAT_ISO8601_FRAC, _Out.clear()) == "2010-09-23T16:17:01.281+02:00");
	
	CHECK(Parse("2005-01-08T12:30:00.123456", 2005, 1, 8,6, 12, 30, 0, 123, Timezone::LOCAL));

	CHECK(Parse("2005-01-08T12:30:00.012034", 2005, 1, 8,6, 12, 30, 0, 12, Timezone::LOCAL));
	_Date.setOffset(4020000);
	CHECK(_Date.offset()==4020000);
	CHECK(_Date.format(Date::FORMAT_ISO8601_FRAC, _Out.clear()) == "2005-01-08T12:30:00.012+01:07");

	CHECK(_Ex); // warning on microsecond lost information
	_Ex = NULL;
}


ADD_TEST(ParseRFC822) {

	CHECK(Parse("Sat, 8 Jan 05 12:30:00 GMT", 2005, 1, 8,6, 12, 30, 0, 0, Timezone::GMT));
	CHECK(_Date.format(Date::FORMAT_RFC822,_Out.clear()) == "Sat, 8 Jan 05 12:30:00 GMT");

	CHECK(Parse("Sat, 8 Jan 05 12:30:00 +0100", 2005, 1, 8,6, 12, 30, 0, 0, 3600000));
	CHECK(_Date.format(Date::FORMAT_RFC822, _Out.clear()) == "Sat, 8 Jan 05 12:30:00 +0100");

	CHECK(Parse("Sat, 8 Jan 05 12:30:00 -0100", 2005, 1, 8,6, 12, 30, 0, 0, -3600000));
	CHECK(_Date.format(Date::FORMAT_RFC822, _Out.clear()) == "Sat, 8 Jan 05 12:30:00 -0100");

	bool isDST(false);

	CHECK(Parse("Tue, 18 Jan 05 12:30:00 CET", 2005, 1, 18,2, 12, 30, 0, 0, Timezone::Offset("CET",isDST)));
	CHECK(_Date.isDST()==isDST && isDST==false);

	CHECK(Parse("Wed, 12 Sep 73 02:01:12 CEST", 1973, 9, 12,3, 2, 1, 12, 0, Timezone::Offset("CEST",isDST)));
	CHECK(_Date.isDST()==isDST && isDST==true);
	
	CHECK(Parse("12 Sep 73 02:01:12 CEST", 1973, 9, 12,3 ,2, 1, 12, 0, Timezone::Offset("CEST",isDST)));
	CHECK(_Date.isDST()==isDST && isDST==true);

	CHECK(Parse("12 Sep 73 02:01:12 CEST", 1973, 9, 12,3, 2, 1, 12, 0, Timezone::Offset("CEST",isDST),Date::FORMAT_RFC822));  // [%w, ]%e %b %y %H:%M[:%S] %Z
	CHECK(_Date.isDST()==isDST && isDST==true);
}


ADD_TEST(ParseRFC1123) {

	CHECK(Parse("Sat, 8 Jan 2005 12:30:00 GMT", 2005, 1, 8,6, 12, 30, 0, 0, Timezone::GMT));
	CHECK(_Date.format(Date::FORMAT_RFC1123,_Out.clear()) == "Sat, 8 Jan 2005 12:30:00 GMT");

	CHECK(Parse("Sat, 8 Jan 2005 12:30:00 +0100", 2005, 1, 8,6, 12, 30, 0, 0, 3600000));
	CHECK(_Date.format(Date::FORMAT_RFC1123, _Out.clear()) == "Sat, 8 Jan 2005 12:30:00 +0100");

	CHECK(Parse("Sat, 8 Jan 2005 12:30:00 -0100", 2005, 1, 8,6, 12, 30, 0, 0, -3600000));
	CHECK(_Date.format(Date::FORMAT_RFC1123, _Out.clear()) == "Sat, 8 Jan 2005 12:30:00 -0100");

	bool isDST(false);
	CHECK(Parse("Sun, 20 Jul 1969 16:17:30 EDT", 1969, 7, 20,0, 16, 17, 30, 0, Timezone::Offset("EDT",isDST)));
	CHECK(_Date.isDST()==isDST && isDST==true);

	CHECK(Parse("Sun, 20 Jul 1969 16:17:30 GMT+01:00", 1969, 7, 20,0, 16, 17, 30, 0,3600000));
}


ADD_TEST(ParseHTTP) {

	CHECK(Parse("Sat, 08 Jan 2005 12:30:00 GMT", 2005, 1, 8,6, 12, 30, 0, 0, Timezone::GMT));
	CHECK(_Date.format(Date::FORMAT_HTTP,_Out.clear()) == "Sat, 08 Jan 2005 12:30:00 GMT");

	CHECK(Parse("Sat, 08 Jan 2005 12:30:00 +0100", 2005, 1, 8,6, 12, 30, 0, 0, 3600000));
	CHECK(_Date.format(Date::FORMAT_HTTP, _Out.clear()) == "Sat, 08 Jan 2005 12:30:00 +0100");

	CHECK(Parse("Sat, 08 Jan 2005 12:30:00 -0100", 2005, 1, 8,6, 12, 30, 0, 0, -3600000));
	CHECK(_Date.format(Date::FORMAT_HTTP, _Out.clear()) == "Sat, 08 Jan 2005 12:30:00 -0100");
}


ADD_TEST(ParseRFC850) {
	
	CHECK(Parse("Saturday, 8-Jan-05 12:30:00 GMT", 2005, 1, 8,6, 12, 30, 0, 0, Timezone::GMT));
	CHECK(_Date.format(Date::FORMAT_RFC850, _Out.clear()) == "Saturday, 8-Jan-05 12:30:00 GMT");

	CHECK(Parse("Saturday, 8-Jan-05 12:30:00 +0100", 2005, 1, 8,6, 12, 30, 0, 0, 3600000));
	CHECK(_Date.format(Date::FORMAT_RFC850, _Out.clear()) == "Saturday, 8-Jan-05 12:30:00 +0100");

	CHECK(Parse("Saturday, 8-Jan-05 12:30:00 -0100", 2005, 1, 8,6, 12, 30, 0, 0, -3600000));
	CHECK(_Date.format(Date::FORMAT_RFC850, _Out.clear()) == "Saturday, 8-Jan-05 12:30:00 -0100");

	bool isDST(false);
	CHECK(Parse("Wed, 12-Sep-73 02:01:12 CEST", 1973, 9, 12, 3, 2, 1, 12, 0, Timezone::Offset("CEST",isDST)));
	CHECK(_Date.isDST()==isDST && isDST==true);
}


ADD_TEST(ParseRFC1036) {

	CHECK(Parse("Saturday, 8 Jan 05 12:30:00 GMT", 2005, 1, 8,6, 12, 30, 0, 0, Timezone::GMT));
	CHECK(_Date.format(Date::FORMAT_RFC1036,_Out.clear()) == "Saturday, 8 Jan 05 12:30:00 GMT");

	CHECK(Parse("Saturday, 8 Jan 05 12:30:00 +0100", 2005, 1, 8,6, 12, 30, 0, 0, 3600000));
	CHECK(_Date.format(Date::FORMAT_RFC1036, _Out.clear()) == "Saturday, 8 Jan 05 12:30:00 +0100");

	CHECK(Parse("Saturday, 8 Jan 05 12:30:00 -0100", 2005, 1, 8,6, 12, 30, 0, 0, -3600000));
	CHECK(_Date.format(Date::FORMAT_RFC1036, _Out.clear()) == "Saturday, 8 Jan 05 12:30:00 -0100");
}



ADD_TEST(ParseASCTIME) {

	CHECK(Parse("Sat Jan  8 12:30:00 2005", 2005, 1, 8,6, 12, 30, 0, 0, Timezone::LOCAL));
	CHECK(_Date.format(Date::FORMAT_ASCTIME,_Out.clear()) == "Sat Jan  8 12:30:00 2005");
}


ADD_TEST(ParseSORTABLE) {

	CHECK(Parse("2005-01-08 12:30:00", 2005, 1, 8,6, 12, 30, 0, 0, Timezone::LOCAL));
	CHECK(_Date.format(Date::FORMAT_SORTABLE,_Out.clear()) == "2005-01-08 12:30:00");

	CHECK(Parse("2005-01-08", 2005, 1, 8,6, 0, 0, 0, 0, Timezone::LOCAL));
	CHECK(_Date.format(Date::FORMAT_SORTABLE,_Out.clear()) == "2005-01-08 00:00:00");
}


ADD_TEST(ParseCustom){

	CHECK(Parse("18-Jan-2005", 2005, 1, 18,2, 0, 0, 0, 0, Timezone::LOCAL,"%d-%b-%Y"));
	CHECK(_Date.format("%d-%b-%Y",_Out.clear()) == "18-Jan-2005");
	
	CHECK(Parse("01/18/05", 2005, 1, 18,2, 0, 0, 0, 0, Timezone::LOCAL,"%m/%d/%y"));
	CHECK(_Date.format("%m/%d/%y",_Out.clear()) == "01/18/05");

	CHECK(Parse("12:30 am", 0, 1, 1, 6, 0,30, 0, 0, Timezone::LOCAL,"%h:%M %a"));
	CHECK(_Date.format("%h:%M %a",_Out.clear()) == "12:30 am");

	CHECK(Parse("12:30 PM", 0, 1, 1,6, 12, 30, 0, 0, Timezone::LOCAL,"%h:%M %a"));
	CHECK(_Date.format("%h:%M %A",_Out.clear()) == "12:30 PM");

	CHECK(Parse("Sat/January/08/2005/12/PM/30/00/250000/GMT/%", 2005, 1, 8,6, 12, 30, 0, 250, Timezone::GMT,"%w/%B/%d/%Y/%h/%A/%M/%S/%F/%Z/%%"));
	CHECK(_Date.format("%w/%W/%b/%B/%d/%e/%f/%m/%n/%o/%y/%Y/%H/%h/%a/%A/%M/%S/%i/%c/%z/%Z/%%",_Out.clear()) ==
		"Sat/Saturday/Jan/January/08/8/ 8/01/1/ 1/05/2005/12/12/pm/PM/30/00/250/2/Z/GMT/%");
}


ADD_TEST(ParseGuess) {

	CHECK(Parse("2005-01-08T12:30:00Z", 2005, 1, 8,6, 12, 30, 0, 0, Timezone::GMT));

	CHECK(Parse("20050108T123000Z", 2005, 1, 8,6, 12, 30, 0, 0, Timezone::GMT));

	CHECK(Parse("20050108T123000", 2005, 1, 8,6, 12, 30, 0, 0, Timezone::LOCAL));

	CHECK(Parse("2005-01-08T12:30:00+01:00", 2005, 1, 8,6, 12, 30, 0, 0, 3600000));

	CHECK(Parse("2005-01-08T12:30:00.123456Z", 2005, 1, 8,6, 12, 30, 0, 123, Timezone::GMT));

	CHECK(Parse("2005-01-08T12:30:00,123456Z", 2005, 1, 8,6, 12, 30, 0, 123, Timezone::GMT));

	CHECK(Parse("20050108T123000,123456Z", 2005, 1, 8,6, 12, 30, 0, 123, Timezone::GMT));

	CHECK(Parse("20050108T123000.123+0200", 2005, 1, 8, 6,12, 30, 0, 123, 7200000));

	CHECK(Parse("2005-01-08T12:30:00.123456-02:00", 2005,1,8, 6, 12, 30, 0, 123, -7200000));
}

}
