#line 1 "src/system/Time.re"

#include "Config.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>

#include "StrOperation.h"
#include "TimeOperation.h"
#include "SQLException.h"
#include "AssertException.h"


/**
 * Implementation of the Time interface
 *
 * ISO 8601: http://en.wikipedia.org/wiki/ISO_8601
 * @file
 */


/* ----------------------------------------------------------- Definitions */

#ifndef HAVE_TIMEGM

/* Counter the number of leap year in the range [0, y). The |y| is the
 year, including century (e.g., 2012) */
static int count_leap_year(int y)
{
        y -= 1;
        return y/4-y/100+y/400;
}


/* Returns nonzero if the |y| is the leap year. The |y| is the year,
 including century (e.g., 2012) */
static int is_leap_year(int y)
{
        return y%4 == 0 && (y%100 != 0 || y%400 == 0);
}


/* The number of days before ith month begins */
static int daysum[] = {
        0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};


/* Based on the algorithm of Python 2.7 calendar.timegm. */
time_t timegm(struct tm *tm)
{
        int days;
        int num_leap_year;
        int64_t t;
        if(tm->tm_mon > 11) {
                return -1;
        }
        num_leap_year = count_leap_year(tm->tm_year + 1900) - count_leap_year(1970);
        days = (tm->tm_year - 70) * 365 +
        num_leap_year + daysum[tm->tm_mon] + tm->tm_mday-1;
        if(tm->tm_mon >= 2 && is_leap_year(tm->tm_year + 1900)) {
                ++days;
        }
        t = ((int64_t)days * 24 + tm->tm_hour) * 3600 + tm->tm_min * 60 + tm->tm_sec;
        if(sizeof(time_t) == 4) {
                if(t < INT32_MIN || t > INT32_MAX) {
                        return -1;
                }
        }
        return t;
}
#endif /* !HAVE_TIMEGM */

#if HAVE_STRUCT_TM_TM_GMTOFF
#define TM_GMTOFF tm_gmtoff
#else
#define TM_GMTOFF tm_wday
#endif

#define i2a(i) (x[0] = ((i) / 10) + '0', x[1] = ((i) % 10) + '0')


/* --------------------------------------------------------------- Private */


static inline int a2i(const char *a, int l) {
        int n = 0;
        for (; *a && l--; a++)
                n = n * 10 + (*a) - '0';
        return n;
}


/* ----------------------------------------------------- Protected methods */


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility push(hidden)
#endif


time_t Time_toTimestamp(const char *s) {
        if (STR_DEF(s)) {
                struct tm t = {0};
                if (Time_toDateTime(s, &t)) {
                        t.tm_year -= 1900;
                        return timegm(&t);
                }
        }
	return 0;
}


struct tm *Time_toDateTime(const char *s, struct tm *t) {
        assert(t);
        assert(s);
        struct tm tm = {.tm_isdst = -1};
        int has_date = false, has_time = false;
        const char *limit = s + strlen(s), *marker, *token, *cursor = s;
	while (true) {
		if (cursor >= limit) {
                        if (has_date || has_time) {
                                *(struct tm*)t = tm;
                                return t;
                        }
                        THROW(SQLException, "Invalid date or time");
                }
                token = cursor;

#line 185 "<stdout>"
{
	unsigned char yych;
	unsigned int yyaccept = 0;
	static const unsigned char yybm[] = {
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
	};

	yych = *cursor;
	if (yych <= ',') {
		if (yych == '+') goto yy4;
		goto yy5;
	} else {
		if (yych <= '-') goto yy4;
		if (yych <= '/') goto yy5;
		if (yych >= ':') goto yy5;
	}
	yyaccept = 0;
	yych = *(marker = ++cursor);
	if (yych <= '/') goto yy3;
	if (yych <= '9') goto yy15;
yy3:
#line 241 "src/system/Time.re"
	{
                        continue;
                 }
#line 242 "<stdout>"
yy4:
	yyaccept = 0;
	yych = *(marker = ++cursor);
	if (yych <= '/') goto yy3;
	if (yych <= '9') goto yy6;
	goto yy3;
yy5:
	yych = *++cursor;
	goto yy3;
yy6:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych <= '9') goto yy8;
yy7:
	cursor = marker;
	if (yyaccept <= 1) {
		if (yyaccept <= 0) {
			goto yy3;
		} else {
			goto yy9;
		}
	} else {
		if (yyaccept <= 2) {
			goto yy23;
		} else {
			goto yy31;
		}
	}
yy8:
	yyaccept = 1;
	yych = *(marker = ++cursor);
	if (yych == '\n') goto yy9;
	if (yych <= '/') goto yy10;
	if (yych <= '9') goto yy11;
	goto yy10;
yy9:
#line 228 "src/system/Time.re"
	{ // Timezone: +-HH:MM, +-HH or +-HHMM is offset from UTC in seconds
                        if (has_time) { // Only set timezone if time has been seen
                                tm.TM_GMTOFF = a2i(token + 1, 2) * 3600;
                                if (token[3] >= '0' && token[3] <= '9')
                                        tm.TM_GMTOFF += a2i(token + 3, 2) * 60;
                                else if (token[4] >= '0' && token[4] <= '9')
                                        tm.TM_GMTOFF += a2i(token + 4, 2) * 60;
                                if (token[0] == '-')
                                        tm.TM_GMTOFF *= -1;
                        }
                        continue;
                 }
#line 292 "<stdout>"
yy10:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych <= '9') goto yy14;
	goto yy7;
yy11:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yych = *++cursor;
	if (yych <= '/') goto yy9;
	if (yych >= ':') goto yy9;
yy13:
	yych = *++cursor;
	goto yy9;
yy14:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych <= '9') goto yy13;
	goto yy7;
yy15:
	yych = *++cursor;
	if (yych <= '/') goto yy17;
	if (yych >= ':') goto yy17;
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych <= '9') goto yy27;
	goto yy7;
yy17:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yych = *++cursor;
	if (yych <= '/') goto yy20;
	if (yych <= '9') goto yy7;
yy20:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yyaccept = 2;
	yych = *(marker = ++cursor);
	if (yych == '.') goto yy24;
yy23:
#line 212 "src/system/Time.re"
	{ // Time: HH:MM:SS
                        tm.tm_hour = a2i(token, 2);
                        tm.tm_min  = a2i(token + 3, 2);
                        tm.tm_sec  = a2i(token + 6, 2);
                        has_time = true;
                        continue;
                 }
#line 350 "<stdout>"
yy24:
	yych = *++cursor;
	if (yybm[0+yych] & 128) {
		goto yy25;
	}
	goto yy7;
yy25:
	++cursor;
	yych = *cursor;
	if (yybm[0+yych] & 128) {
		goto yy25;
	}
	goto yy23;
yy27:
	yych = *++cursor;
	if (yych <= '/') goto yy28;
	if (yych <= '9') goto yy29;
yy28:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych <= '9') goto yy38;
	goto yy7;
yy29:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yyaccept = 3;
	yych = *(marker = ++cursor);
	if (yych == '.') goto yy33;
	if (yych <= '/') goto yy31;
	if (yych <= '9') goto yy32;
yy31:
#line 220 "src/system/Time.re"
	{ // Compressed Time: HHMMSS
                        tm.tm_hour = a2i(token, 2);
                        tm.tm_min  = a2i(token + 2, 2);
                        tm.tm_sec  = a2i(token + 4, 2);
                        has_time = true;
                        continue;
                 }
#line 391 "<stdout>"
yy32:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych <= '9') goto yy36;
	goto yy7;
yy33:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
yy34:
	++cursor;
	yych = *cursor;
	if (yych <= '/') goto yy31;
	if (yych <= '9') goto yy34;
	goto yy31;
yy36:
	++cursor;
#line 204 "src/system/Time.re"
	{ // Compressed Date: YYYYMMDD
                        tm.tm_year  = a2i(token, 4);
                        tm.tm_mon   = a2i(token + 4, 2) - 1;
                        tm.tm_mday  = a2i(token + 6, 2);
                        has_date = true;
                        continue;
                 }
#line 417 "<stdout>"
yy38:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yych = *++cursor;
	if (yych <= '/') goto yy40;
	if (yych <= '9') goto yy7;
yy40:
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	yych = *++cursor;
	if (yych <= '/') goto yy7;
	if (yych >= ':') goto yy7;
	++cursor;
#line 196 "src/system/Time.re"
	{ // Date: YYYY-MM-DD
                        tm.tm_year  = a2i(token, 4);
                        tm.tm_mon   = a2i(token + 5, 2) - 1;
                        tm.tm_mday  = a2i(token + 8, 2);
                        has_date = true;
                        continue;
                 }
#line 441 "<stdout>"
}
#line 244 "src/system/Time.re"

        }
	return NULL;
}


char *Time_toString(time_t time, char result[20]) {
        assert(result);
        char x[2];
        struct tm ts = {.tm_isdst = -1};
        gmtime_r(&time, &ts);
        memcpy(result, "YYYY-MM-DD HH:MM:SS\0", 20);
        /*              0    5  8  11 14 17 */
        i2a((ts.tm_year+1900)/100);
        result[0] = x[0];
        result[1] = x[1];
        i2a((ts.tm_year+1900)%100);
        result[2] = x[0];
        result[3] = x[1];
        i2a(ts.tm_mon + 1); // Months in 01-12
        result[5] = x[0];
        result[6] = x[1];
        i2a(ts.tm_mday);
        result[8] = x[0];
        result[9] = x[1];
        i2a(ts.tm_hour);
        result[11] = x[0];
        result[12] = x[1];
        i2a(ts.tm_min);
        result[14] = x[0];
        result[15] = x[1];
        i2a(ts.tm_sec);
        result[17] = x[0];
        result[18] = x[1];
	return result;
}


time_t Time_now(void) {
	struct timeval t;
	if (gettimeofday(&t, NULL) != 0)
                THROW(AssertException, "%s", System_getLastError());
	return t.tv_sec;
}


long long Time_milli(void) {
	struct timeval t;
	if (gettimeofday(&t, NULL) != 0)
                THROW(AssertException, "%s", System_getLastError());
	return (long long)t.tv_sec * 1000  +  (long long)t.tv_usec / 1000;
}


int Time_usleep(long u) {
        struct timeval t;
        t.tv_sec = u / USEC_PER_SEC;
        t.tv_usec = (suseconds_t)(u % USEC_PER_SEC);
        select(0, 0, 0, 0, &t);
        return true;
}


#ifdef PACKAGE_PROTECTED
#pragma GCC visibility pop
#endif
