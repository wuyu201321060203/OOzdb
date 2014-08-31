#line 1 "src/net/URL.re"

#include "Config.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "URL.h"


/**
 * Implementation of the URL interface. The scanner handle
 * ISO Latin 1 or UTF-8 encoded url's transparently.
 *
 * @file
 */


/* ----------------------------------------------------------- Definitions */


typedef struct param_t {
        char *name;
        char *value;
        struct param_t *next;
} *param_t;

#define T URL_T
struct URL_S {
	int port;
       	char *ref;
	char *path;
	char *host;
	char *user;
        char *qptr;
	char *query;
	char *portStr;
	char *protocol;
	char *password;
	char *toString;
        param_t params;
        char **paramNames;
	uchar_t *data;
	uchar_t *buffer;
	uchar_t *marker, *ctx, *limit, *token;
        /* Keep the above align with zild URL_T */
};

/* Unsafe URL characters: [00-1F, 7F-FF] <>\"#%}{|\\^[] ` */
static const uchar_t urlunsafe[256] = {
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

#define UNKNOWN_PORT -1
#define YYCTYPE       uchar_t
#define YYCURSOR      U->buffer
#define YYLIMIT       U->limit
#define YYMARKER      U->marker
#define YYCTXMARKER   U->ctx
#define YYFILL(n)     ((void)0)
#define YYTOKEN       U->token
#define SET_PROTOCOL(PORT) *(YYCURSOR-3)=0; U->protocol=U->token; U->port=PORT; goto authority


/* ------------------------------------------------------- Private methods */


static int parseURL(T U) {
        param_t param = NULL;
	#line 122 "src/net/URL.re"

proto:
	if (YYCURSOR >= YYLIMIT)
		return false;
	YYTOKEN = YYCURSOR;

#line 121 "<stdout>"
{
	YYCTYPE yych;
	static const unsigned char yybm[] = {
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128,   0,   0,   0,   0,   0,   0,
		  0, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128,   0,   0,   0,   0,   0,
		  0, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128,   0,   0,   0,   0,   0,
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

	if ((YYLIMIT - YYCURSOR) < 13) YYFILL(13);
	yych = *YYCURSOR;
	if (yych <= '@') {
		if (yych <= '\r') {
			if (yych <= 0x08) goto yy9;
			if (yych <= '\n') goto yy2;
			if (yych <= '\f') goto yy9;
		} else {
			if (yych <= ' ') {
				if (yych <= 0x1F) goto yy9;
			} else {
				if (yych <= '/') goto yy9;
				if (yych <= '9') goto yy8;
				goto yy9;
			}
		}
	} else {
		if (yych <= 'm') {
			if (yych <= 'Z') goto yy8;
			if (yych <= '`') goto yy9;
			if (yych <= 'l') goto yy8;
			goto yy4;
		} else {
			if (yych <= 'o') {
				if (yych <= 'n') goto yy8;
				goto yy7;
			} else {
				if (yych <= 'p') goto yy6;
				if (yych <= 'z') goto yy8;
				goto yy9;
			}
		}
	}
yy2:
	++YYCURSOR;
#line 129 "src/net/URL.re"
	{
                        goto proto;
		   }
#line 198 "<stdout>"
yy4:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= 'Z') {
		if (yych <= '/') goto yy5;
		if (yych <= ':') goto yy13;
		if (yych >= 'A') goto yy13;
	} else {
		if (yych <= 'x') {
			if (yych >= 'a') goto yy13;
		} else {
			if (yych <= 'y') goto yy39;
			if (yych <= 'z') goto yy13;
		}
	}
yy5:
#line 149 "src/net/URL.re"
	{
                      	goto proto;
                   }
#line 218 "<stdout>"
yy6:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= 'Z') {
		if (yych <= '/') goto yy5;
		if (yych <= ':') goto yy13;
		if (yych <= '@') goto yy5;
		goto yy13;
	} else {
		if (yych <= 'n') {
			if (yych <= '`') goto yy5;
			goto yy13;
		} else {
			if (yych <= 'o') goto yy26;
			if (yych <= 'z') goto yy13;
			goto yy5;
		}
	}
yy7:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= 'Z') {
		if (yych <= '/') goto yy5;
		if (yych <= ':') goto yy13;
		if (yych <= '@') goto yy5;
		goto yy13;
	} else {
		if (yych <= 'q') {
			if (yych <= '`') goto yy5;
			goto yy13;
		} else {
			if (yych <= 'r') goto yy17;
			if (yych <= 'z') goto yy13;
			goto yy5;
		}
	}
yy8:
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= '@') {
		if (yych <= '/') goto yy5;
		if (yych <= ':') goto yy13;
		goto yy5;
	} else {
		if (yych <= 'Z') goto yy13;
		if (yych <= '`') goto yy5;
		if (yych <= 'z') goto yy13;
		goto yy5;
	}
yy9:
	yych = *++YYCURSOR;
	goto yy5;
yy10:
	yych = *++YYCURSOR;
	if (yych == '/') goto yy14;
yy11:
	YYCURSOR = YYMARKER;
	goto yy5;
yy12:
	++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 3) YYFILL(3);
	yych = *YYCURSOR;
yy13:
	if (yybm[0+yych] & 128) {
		goto yy12;
	}
	if (yych == ':') goto yy10;
	goto yy11;
yy14:
	yych = *++YYCURSOR;
	if (yych != '/') goto yy11;
	++YYCURSOR;
#line 145 "src/net/URL.re"
	{
                      	SET_PROTOCOL(UNKNOWN_PORT);
                   }
#line 292 "<stdout>"
yy17:
	yych = *++YYCURSOR;
	if (yych != 'a') goto yy13;
	yych = *++YYCURSOR;
	if (yych != 'c') goto yy13;
	yych = *++YYCURSOR;
	if (yych != 'l') goto yy13;
	yych = *++YYCURSOR;
	if (yych != 'e') goto yy13;
	yych = *++YYCURSOR;
	if (yych != ':') goto yy13;
	yych = *++YYCURSOR;
	if (yych != '/') goto yy11;
	yych = *++YYCURSOR;
	if (yych != '/') goto yy11;
	++YYCURSOR;
#line 141 "src/net/URL.re"
	{
                      	SET_PROTOCOL(ORACLE_DEFAULT_PORT);
                   }
#line 313 "<stdout>"
yy26:
	yych = *++YYCURSOR;
	if (yych != 's') goto yy13;
	yych = *++YYCURSOR;
	if (yych != 't') goto yy13;
	yych = *++YYCURSOR;
	if (yych != 'g') goto yy13;
	yych = *++YYCURSOR;
	if (yych != 'r') goto yy13;
	yych = *++YYCURSOR;
	if (yych != 'e') goto yy13;
	yych = *++YYCURSOR;
	if (yych != 's') goto yy13;
	yych = *++YYCURSOR;
	if (yych != 'q') goto yy13;
	yych = *++YYCURSOR;
	if (yych != 'l') goto yy13;
	yych = *++YYCURSOR;
	if (yych != ':') goto yy13;
	yych = *++YYCURSOR;
	if (yych != '/') goto yy11;
	yych = *++YYCURSOR;
	if (yych != '/') goto yy11;
	++YYCURSOR;
#line 137 "src/net/URL.re"
	{
                      	SET_PROTOCOL(POSTGRESQL_DEFAULT_PORT);
                   }
#line 342 "<stdout>"
yy39:
	yych = *++YYCURSOR;
	if (yych != 's') goto yy13;
	yych = *++YYCURSOR;
	if (yych != 'q') goto yy13;
	yych = *++YYCURSOR;
	if (yych != 'l') goto yy13;
	yych = *++YYCURSOR;
	if (yych != ':') goto yy13;
	yych = *++YYCURSOR;
	if (yych != '/') goto yy11;
	yych = *++YYCURSOR;
	if (yych != '/') goto yy11;
	++YYCURSOR;
#line 133 "src/net/URL.re"
	{
                      	SET_PROTOCOL(MYSQL_DEFAULT_PORT);
                   }
#line 361 "<stdout>"
}
#line 152 "src/net/URL.re"

authority:
	if (YYCURSOR >= YYLIMIT)
		return true;
	YYTOKEN = YYCURSOR;

#line 370 "<stdout>"
{
	YYCTYPE yych;
	unsigned int yyaccept = 0;
	static const unsigned char yybm[] = {
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		 16, 112, 112,  16, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 240, 112, 112,
		248, 248, 248, 248, 248, 248, 248, 248,
		248, 248, 112,  16, 112, 112, 112,  16,
		 64, 240, 240, 240, 240, 240, 240, 240,
		240, 240, 240, 240, 240, 240, 240, 240,
		240, 240, 240, 240, 240, 240, 240, 240,
		240, 240, 240, 112, 112, 112, 112, 112,
		112, 240, 240, 240, 240, 240, 240, 240,
		240, 240, 240, 240, 240, 240, 240, 240,
		240, 240, 240, 240, 240, 240, 240, 240,
		240, 240, 240, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
		112, 112, 112, 112, 112, 112, 112, 112,
	};
	if ((YYLIMIT - YYCURSOR) < 3) YYFILL(3);
	yych = *YYCURSOR;
	if (yych <= '.') {
		if (yych <= '\r') {
			if (yych <= 0x08) goto yy59;
			if (yych <= '\n') goto yy49;
			if (yych <= '\f') goto yy59;
		} else {
			if (yych <= ' ') {
				if (yych <= 0x1F) goto yy59;
				goto yy51;
			} else {
				if (yych == '-') goto yy54;
				goto yy52;
			}
		}
	} else {
		if (yych <= '?') {
			if (yych <= '/') goto yy56;
			if (yych <= '9') goto yy54;
			if (yych <= ':') goto yy58;
			goto yy52;
		} else {
			if (yych <= 'Z') {
				if (yych <= '@') goto yy59;
				goto yy54;
			} else {
				if (yych <= '`') goto yy52;
				if (yych <= 'z') goto yy54;
				goto yy52;
			}
		}
	}
yy49:
	++YYCURSOR;
yy50:
#line 159 "src/net/URL.re"
	{
                        goto authority;
                   }
#line 448 "<stdout>"
yy51:
	yyaccept = 0;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych <= 0x1F) goto yy50;
	goto yy66;
yy52:
	yyaccept = 1;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych >= ' ') goto yy66;
yy53:
#line 197 "src/net/URL.re"
	{
                      	return true;
                   }
#line 463 "<stdout>"
yy54:
	yyaccept = 2;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yybm[0+yych] & 128) {
		goto yy77;
	}
	if (yych <= 0x1F) goto yy55;
	if (yych == '.') goto yy76;
	goto yy66;
yy55:
#line 175 "src/net/URL.re"
	{
                        U->host = Str_ndup(YYTOKEN, (int)(YYCURSOR - YYTOKEN));
                        goto authority;
                   }
#line 479 "<stdout>"
yy56:
	yyaccept = 3;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yybm[0+yych] & 32) {
		goto yy68;
	}
	if (yych <= 0x1F) goto yy57;
	if (yych <= '>') goto yy65;
	if (yych <= '?') goto yy71;
	goto yy70;
yy57:
#line 185 "src/net/URL.re"
	{
                        *YYCURSOR = 0;
                        U->path = URL_unescape(YYTOKEN);
                        return true;
                   }
#line 497 "<stdout>"
yy58:
	yyaccept = 1;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yybm[0+yych] & 8) {
		goto yy60;
	}
	if (yych <= 0x1F) goto yy53;
	goto yy66;
yy59:
	yych = *++YYCURSOR;
	goto yy53;
yy60:
	yyaccept = 4;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yybm[0+yych] & 8) {
		goto yy60;
	}
	if (yych <= 0x1F) goto yy62;
	if (yych == '@') goto yy63;
	goto yy65;
yy62:
#line 180 "src/net/URL.re"
	{
                        U->port = Str_parseInt(YYTOKEN + 1); // read past ':'
                        goto authority;
                   }
#line 526 "<stdout>"
yy63:
	++YYCURSOR;
yy64:
#line 163 "src/net/URL.re"
	{
                        *(YYCURSOR - 1) = 0;
                        U->user = YYTOKEN;
                        char *p = strchr(U->user, ':');
                        if (p) {
                                *(p++) = 0;
                                U->password = URL_unescape(p);
                        }
                        URL_unescape(U->user);
                        goto authority;
                   }
#line 542 "<stdout>"
yy65:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy66:
	if (yybm[0+yych] & 16) {
		goto yy65;
	}
	if (yych >= '@') goto yy63;
yy67:
	YYCURSOR = YYMARKER;
	if (yyaccept <= 2) {
		if (yyaccept <= 1) {
			if (yyaccept <= 0) {
				goto yy50;
			} else {
				goto yy53;
			}
		} else {
			goto yy55;
		}
	} else {
		if (yyaccept <= 4) {
			if (yyaccept <= 3) {
				goto yy57;
			} else {
				goto yy62;
			}
		} else {
			goto yy72;
		}
	}
yy68:
	yyaccept = 3;
	YYMARKER = ++YYCURSOR;
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yybm[0+yych] & 32) {
		goto yy68;
	}
	if (yych <= 0x1F) goto yy57;
	if (yych <= '>') goto yy65;
	if (yych <= '?') goto yy71;
yy70:
	yych = *++YYCURSOR;
	if (yych <= '#') {
		if (yych <= ' ') goto yy64;
		if (yych <= '"') goto yy74;
		goto yy64;
	} else {
		if (yych == ';') goto yy64;
		goto yy74;
	}
yy71:
	yyaccept = 5;
	yych = *(YYMARKER = ++YYCURSOR);
	if (yych >= ' ') goto yy66;
yy72:
#line 191 "src/net/URL.re"
	{
                        *(YYCURSOR-1) = 0;
                        U->path = URL_unescape(YYTOKEN);
                        goto query;
                   }
#line 607 "<stdout>"
yy73:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy74:
	if (yybm[0+yych] & 64) {
		goto yy73;
	}
	if (yych <= '>') goto yy57;
	yych = *++YYCURSOR;
	goto yy72;
yy76:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '9') {
		if (yych <= ',') {
			if (yych <= 0x1F) goto yy67;
			goto yy65;
		} else {
			if (yych <= '-') goto yy79;
			if (yych <= '/') goto yy65;
			goto yy79;
		}
	} else {
		if (yych <= 'Z') {
			if (yych <= '?') goto yy65;
			if (yych <= '@') goto yy63;
			goto yy79;
		} else {
			if (yych <= '`') goto yy65;
			if (yych <= 'z') goto yy79;
			goto yy65;
		}
	}
yy77:
	yyaccept = 2;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yybm[0+yych] & 128) {
		goto yy77;
	}
	if (yych <= '.') {
		if (yych <= 0x1F) goto yy55;
		if (yych <= '-') goto yy65;
		goto yy76;
	} else {
		if (yych == '@') goto yy63;
		goto yy65;
	}
yy79:
	yyaccept = 2;
	YYMARKER = ++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '9') {
		if (yych <= '-') {
			if (yych <= 0x1F) goto yy55;
			if (yych <= ',') goto yy65;
			goto yy79;
		} else {
			if (yych <= '.') goto yy76;
			if (yych <= '/') goto yy65;
			goto yy79;
		}
	} else {
		if (yych <= 'Z') {
			if (yych <= '?') goto yy65;
			if (yych <= '@') goto yy63;
			goto yy79;
		} else {
			if (yych <= '`') goto yy65;
			if (yych <= 'z') goto yy79;
			goto yy65;
		}
	}
}
#line 201 "src/net/URL.re"

query:
        if (YYCURSOR >= YYLIMIT)
		return true;
	YYTOKEN =  YYCURSOR;

#line 693 "<stdout>"
{
	YYCTYPE yych;
	static const unsigned char yybm[] = {
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		128, 128, 128,   0, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
		128, 128, 128, 128, 128, 128, 128, 128,
	};
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yych <= 0x1F) goto yy85;
	if (yych == '#') goto yy85;
	goto yy84;
yy83:
#line 208 "src/net/URL.re"
	{
                        *YYCURSOR = 0;
                        U->query = Str_ndup(YYTOKEN, (int)(YYCURSOR - YYTOKEN));
                        YYCURSOR = YYTOKEN; // backtrack to start of query string after terminating it and
                        goto params;
                   }
#line 743 "<stdout>"
yy84:
	yych = *++YYCURSOR;
	goto yy88;
yy85:
	++YYCURSOR;
#line 215 "src/net/URL.re"
	{
                      return true;
                   }
#line 753 "<stdout>"
yy87:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy88:
	if (yybm[0+yych] & 128) {
		goto yy87;
	}
	goto yy83;
}
#line 219 "src/net/URL.re"

params:
	if (YYCURSOR >= YYLIMIT)
		return true;
	YYTOKEN =  YYCURSOR;

#line 771 "<stdout>"
{
	YYCTYPE yych;
	static const unsigned char yybm[] = {
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		  0,   0,   0,   0,   0,   0,   0,   0,
		 64, 192, 192, 192, 192, 192, 128, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192,  64, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
		192, 192, 192, 192, 192, 192, 192, 192,
	};
	if ((YYLIMIT - YYCURSOR) < 2) YYFILL(2);
	yych = *YYCURSOR;
	if (yych <= ' ') goto yy95;
	if (yych == '=') goto yy93;
	YYCTXMARKER = YYCURSOR + 1;
	++YYCURSOR;
	yych = *YYCURSOR;
	goto yy103;
yy92:
#line 226 "src/net/URL.re"
	{
                /* No parameters in querystring */
                return true;
        }
#line 822 "<stdout>"
yy93:
	++YYCURSOR;
	yych = *YYCURSOR;
	goto yy98;
yy94:
#line 239 "src/net/URL.re"
	{
                *YYTOKEN++ = 0;
                if (*(YYCURSOR - 1) == '&')
                        *(YYCURSOR - 1) = 0;
                if (! param) /* format error */
                        return true;
                param->value = URL_unescape(YYTOKEN);
                goto params;
        }
#line 838 "<stdout>"
yy95:
	++YYCURSOR;
#line 249 "src/net/URL.re"
	{
                return true;
        }
#line 845 "<stdout>"
yy97:
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy98:
	if (yybm[0+yych] & 64) {
		goto yy97;
	}
	if (yych <= '%') goto yy94;
	yych = *++YYCURSOR;
	goto yy94;
yy100:
	++YYCURSOR;
	YYCURSOR = YYCTXMARKER;
#line 231 "src/net/URL.re"
	{
                NEW(param);
                param->name = YYTOKEN;
                param->next = U->params;
                U->params = param;
                goto params;
        }
#line 868 "<stdout>"
yy102:
	YYCTXMARKER = YYCURSOR + 1;
	++YYCURSOR;
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
yy103:
	if (yybm[0+yych] & 128) {
		goto yy102;
	}
	if (yych <= '<') goto yy92;
	goto yy100;
}
#line 252 "src/net/URL.re"

        return false;
}


static inline int x2b(uchar_t *x) {
	register int b;
	b = ((x[0] >= 'A') ? ((x[0] & 0xdf) - 'A')+10 : (x[0] - '0'));
	b *= 16;
	b += (x[1] >= 'A' ? ((x[1] & 0xdf) - 'A')+10 : (x[1] - '0'));
	return b;
}


static inline uchar_t *b2x(uchar_t b, uchar_t *x) {
        static const char b2x_table[] = "0123456789ABCDEF";
        *x++ = '%';
        *x++ = b2x_table[b >> 4];
        *x = b2x_table[b & 0xf];
        return x;
}


static void freeParams(param_t p) {
        for (param_t q = NULL; p; p = q) {
                q = p->next;
                FREE(p);
        }
}


static T ctor(uchar_t *data) {
        T U;
	NEW(U);
	U->data = data;
	YYCURSOR = U->data;
	U->port = UNKNOWN_PORT;
	YYLIMIT = U->data + strlen(U->data);
	if (! parseURL(U))
                URL_free(&U);
	return U;
}


/* -------------------------------------------------------- Public methods */


T URL_new(const char *url) {
        if (STR_UNDEF(url))
                return NULL;
        Exception_init();
        return ctor((uchar_t*)Str_dup(url));
}


T URL_create(const char *url, ...) {
        if (STR_UNDEF(url))
                return NULL;
        Exception_init();
	va_list ap;
        va_start(ap, url);
	T U = ctor((uchar_t*)Str_vcat(url, ap));
  	va_end(ap);
        return U;
}

void URL_free(T *U) {
	assert(U && *U);
        freeParams((*U)->params);
        FREE((*U)->paramNames);
	FREE((*U)->toString);
	FREE((*U)->query);
	FREE((*U)->data);
	FREE((*U)->host);
	FREE(*U);
}


/* ------------------------------------------------------------ Properties */


const char *URL_getProtocol(T U) {
	assert(U);
	return U->protocol;
}


const char *URL_getUser(T U) {
	assert(U);
	return U->user;
}


const char *URL_getPassword(T U) {
	assert(U);
	return U->password;
}


const char *URL_getHost(T U) {
	assert(U);
	return U->host;
}


int URL_getPort(T U) {
	assert(U);
	return U->port;
}


const char *URL_getPath(T U) {
	assert(U);
	return U->path;
}


const char *URL_getQueryString(T U) {
	assert(U);
	return U->query;
}


const char **URL_getParameterNames(T U) {
        assert(U);
        if (U->params && (U->paramNames == NULL)) {
                param_t p;
                int i = 0, len = 0;
                for (p = U->params; p; p = p->next) len++;
                U->paramNames = ALLOC((len + 1) * sizeof *(U->paramNames));
                for (p = U->params; p; p = p->next)
                        U->paramNames[i++] = p->name;
                U->paramNames[i] = NULL;
        }
	return (const char **)U->paramNames;
}


const char *URL_getParameter(T U, const char *name) {
	assert(U);
        assert(name);
        for (param_t p = U->params; p; p = p->next) {
                if (Str_isByteEqual(p->name, name))
                        return p->value;
        }
        return NULL;
}


/* ---------------------------------------------------------------- Public */


const char *URL_toString(T U) {
	assert(U);
	if (! U->toString) {
                uchar_t port[11] = {0};
                if (U->port >= 0)
                        snprintf(port, 10, ":%d", U->port);
		U->toString = Str_cat("%s://%s%s%s%s%s%s%s%s%s",
                                      U->protocol,
                                      U->user ? U->user : "",
                                      U->password ? ":" : "",
                                      U->password ? U->password : "",
                                      U->user ? "@" : "",
                                      U->host ? U->host : "",
                                      port,
                                      U->path ? U->path : "",
                                      U->query ? "?" : "",
                                      U->query ? U->query : "");
	}
	return U->toString;
}


/* --------------------------------------------------------- Class methods */


char *URL_unescape(char *url) {
	if (STR_DEF(url)) {
                register int x, y;
                for (x = 0, y = 0; url[y]; x++, y++) {
                        if ((url[x] = url[y]) == '+')
                                url[x] = ' ';
                        else if (url[x] == '%') {
                                if (! (url[x + 1] && url[x + 2]))
                                        break;
                                url[x] = x2b(url + y + 1);
                                y += 2;
                        }
                }
                url[x] = 0;
        }
	return url;
}


char *URL_escape(const char *url) {
        char *escaped = 0;
        if (url) {
                char *p;
                int i, n;
                for (n = i = 0; url[i]; i++)
                        if (urlunsafe[(unsigned char)(url[i])])
                                n += 2;
                p = escaped = ALLOC(i + n + 1);
                for (; *url; url++, p++) {
                        if (urlunsafe[(unsigned char)(*p = *url)])
                                p = b2x(*url, p);
                }
                *p = 0;
        }
        return escaped;
}


char *URL_normalize(char *path) {
	if (path) {
                char c;
                int i,j;
                for (i = j = 0; (c = path[i]); ++i) {
                        if (c == '/') {
                                while (path[i+1] == '/') ++i;
                        } else if (c == '.' && j && path[j-1] == '/') {
                                if (path[i+1] == '.' && (path[i+2] == '/' || path[i+2] == 0)) {
                                        if (j>1)
                                        for (j -= 2; path[j] != '/' && j > 0; --j);
                                        i += 2;
                                } else if (path[i+1] == '/' || path[i+1] == 0) {
                                        ++i;
                                        continue;
                                }
                        }
                        if (! (path[j] = path[i])) break; ++j;
                }
                if (! j) { path[0] = '/'; j = 1; }
                path[j] = 0;
                if (path[0] == '/' && path[1] == '/') {
                        for (i = j = 0; (c = path[i]); ++i) {
                                if (c == '/') {
                                        while (path[i+1] == '/') ++i;
                                }
                                if (! (path[j] = path[i])) break;
                                ++j;
                        }
                        path[j] = 0;
                }
        }
	return path;
}
