/*
 *   Copyright (C) 1997, 1998
 *   	Free Software Foundation, Inc.
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the
 *   Free Software Foundation; either version 2, or (at your option) any
 *   later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */
#ifndef _url_h
#define _url_h

/*
 * Possible values of the info field in the url_t structure.
 */
/*
 * Set if URL is cannonical
 */
#define URL_INFO_CANNONICAL	0x0001
/*
 * Set if the private url field is allocated.
 */
#define URL_INFO_URL		0x0002
/*
 * Set if the private furl field is allocated.
 */
#define URL_INFO_FURL		0x0004
/*
 * Set if the URL is relative.
 */
#define URL_INFO_RELATIVE	0x0008
/*
 * Set if the URL is relative and the path is relative.
 */
#define URL_INFO_RELATIVE_PATH	0x0010
/*
 * Set if the URL is a null string.
 */
#define URL_INFO_EMPTY		0x0020
/*
 * Set if the scheme is HTTP.
 */
#define URL_INFO_HTTP		0x0040
/*
 * Set if the scheme is FTP.
 */
#define URL_INFO_FTP		0x0080
/*
 * Set if the scheme is NEWS.
 */
#define URL_INFO_NEWS		0x0100
/*
 * Set if the scheme is FILE.
 */
#define URL_INFO_FILE		0x0200
/*
 * Set if the URL is a robots.txt URL
 */
#define URL_INFO_ROBOTS		0x0400

#define URL_INFO_SCHEME_MASK	0x03c0

/*
 * Splitted URL information.
 */
typedef struct url {
  /* Public fields */
  int info;		/* Information */
  char* scheme;		/* URL scheme (http, ftp...) */
  char* host;		/* hostname part (www.foo.com) */
  char* port;		/* port part if any (www.foo.com:8080 => 8080) */
  char* path;		/* path portion without params and query */
  char* params;		/* params part (/foo;dir/bar => foo) */
  char* query;		/* query part (/foo?bar=val => bar=val) */
  char* frag;		/* frag part (/foo#part => part) */
  char* user;		/* user part (http://user:pass@www.foo.com => user) */
  char* passwd;		/* user part (http://user:pass@www.foo.com => pass) */

  /* Private fields */
  char* pool;
  int pool_size;
  char* furl;
  int furl_size;
  char* url;
  int url_size;
  char* robots;
  int robots_size;
} url_t;

#define URL_NOT_CANNONICAL	-1
#define URL_CANNONICAL		0

/*
 * Transform URL string in url_t
 */
url_t* url_alloc(char* url, int url_length);
int url_realloc(url_t* object, char* url, int url_length);
/*
 * Release object allocated by url_alloc
 */
void url_free(url_t* object);

/*
 * Put in cannonical form
 */
int url_cannonicalize(url_t* object);
#define url_cannonicalp(o) ((o)->info & URL_INFO_CANNONICAL)

/*
 * Transform relative URL in absolute URL according to base.
 */
url_t* url_abs(url_t* base, char* relative_string, int relative_length);
url_t* url_abs_1(url_t* base, url_t* relative);

/*
 * Access structure fields
 */
char* url_port(url_t* object);
char* url_netloc(url_t* object);
char* url_auth(url_t* object);
char* url_all_path(url_t* object);

/*
 * Copy url_t objects
 */
void url_copy(url_t* to, url_t* from);
/*
 * Parse string pointed by pool and fill fields.
 */
void url_parse(url_t* object);
/*
 * Show fields on stderr.
 */
void url_dump(url_t* object);

/*
 * Convert structure into string.
 */
/*
 * File equivalent of an URL
 */
char* url_furl(url_t* object);
/*
 * URL string
 */
char* url_url(url_t* object);
/*
 * robots.txt URL corresponding to this URL.
 */
char* url_robots(url_t* object);

/*
 * Convinience functions
 */
/*
 * Return static structure instead of malloc'd structure.
 * Must *not* be freed.
 */
url_t* url2object(char* url, int url_length);

/*
 * Possible values of the flag argument of url_furl_string.
/*
 * Prepend the content of the WLROOT environment variable
 * to the FURL.
 */
#define URL_FURL_REAL_PATH	1
/*
 * Do not prepend anything to the FURL.
 */
#define URL_FURL_NOP		0
/*
 * Equivalent to url_furl(url_object(url, strlen(url)))
 */
char* url_furl_string(char* url, int url_length, int flag);

/*
 * Possible values of the flag argument of url_cannonicalize_string.
 */
/*
 * Return a FURL
 */
#define URL_STRING_FURL_STYLE		0x01
/*
 * Return an URL
 */
#define URL_STRING_URL_STYLE		0x02
/*
 * Return the robots.txt URL corresponding to this URL
 */
#define URL_STRING_ROBOTS_STYLE		0x04
/*
 * Omit the frag in the returned string.
 */
#define URL_STRING_URL_NOHASH_STYLE	0x08
/*
 * Cannonicalize the given url and return it in the chosen form.
 */
char* url_cannonicalize_string(char* url, int url_length, int flag);

#endif /* _url_h */
