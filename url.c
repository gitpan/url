/*
 *   Copyright (C) 1995, 1996, 1997, 1998
 *      Civil Engineering in Cyberspace
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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#include <stdio.h>

#ifdef HAVE_CTYPE_H
#include <ctype.h>
#endif /* HAVE_CTYPE_H */

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif /* HAVE_STDLIB_H */

#if STDC_HEADERS
# include <string.h>
#else
# ifndef HAVE_STRCHR
#  define strchr index
#  define strrchr rindex
# endif
char *strchr (), *strrchr ();
# ifndef HAVE_MEMCPY
#  define memcpy(d, s, n) bcopy ((s), (d), (n))
#  define memmove(d, s, n) bcopy ((s), (d), (n))
# endif
#endif

#include "strlower.h"
#include "mpath.h"
#include "salloc.h"
#include "url.h"

static void url_clear(url_t* object);
static void url_string(url_t* object, char** stringp, int* string_sizep, int flags);

static int verbose = 0;

static url_t* url_object = 0;

url_t* url2object(char* url, int url_length)
{
  if(url_object) {
    int cannonical = url_realloc(url_object, url, url_length);
    return cannonical == URL_CANNONICAL ? url_object : 0;
  } else
    return url_object = url_alloc(url, url_length);
}

url_t* url_alloc(char* url, int url_length)
{
  url_t* object = (url_t*)smalloc(sizeof(url_t));

  memset(object, '\0', sizeof(url_t));
  object->pool = (char*)smalloc(url_length + 1);
  memcpy(object->pool, url, url_length);
  object->pool[url_length] = '\0';
  object->pool_size = url_length + 1;

  url_parse(object);
  if(url_cannonicalize(object) != URL_CANNONICAL) {
    url_free(object);
    return 0;
  } else {
    return object;
  }
}

static void url_clear(url_t* object)
{
  object->scheme = 0;
  object->host = 0;
  object->port = 0;
  object->path = 0;
  object->params = 0;
  object->query = 0;
  object->frag = 0;
  object->user = 0;
  object->passwd = 0;
  object->info = 0;
}

int url_realloc(url_t* object, char* url, int url_length)
{
  url_clear(object);

  if(object->pool_size < url_length + 1) {
    object->pool = (char*)srealloc(object->pool, url_length + 1);
    object->pool_size = url_length + 1;
  }
  memcpy(object->pool, url, url_length);
  object->pool[url_length] = '\0';
  url_parse(object);
  return url_cannonicalize(object);
}

char* url_robots(url_t* object)
{
  if((object->info & URL_INFO_ROBOTS) == 0) {
    url_string(object, &object->robots, &object->robots_size, URL_STRING_ROBOTS_STYLE);
    object->info |= URL_INFO_ROBOTS;
  }
  return object->robots;
}

char* url_url(url_t* object)
{
  if((object->info & URL_INFO_URL) == 0) {
    url_string(object, &object->url, &object->url_size, URL_STRING_URL_STYLE);
    object->info |= URL_INFO_URL;
  }
  return object->url;
}

char* url_furl(url_t* object)
{
  if((object->info & URL_INFO_FURL) == 0) {
    url_string(object, &object->furl, &object->furl_size, URL_STRING_FURL_STYLE);
    object->info |= URL_INFO_FURL;
  }
  return object->furl;
}

char* url_furl_string(char* url, int url_length, int flag)
{
  static char* path = 0;
  static int path_size = 0;
  url_t* url_object = url2object(url, url_length);
  char* furl = url_furl(url_object);
  int furl_length;
  char* wlroot = 0;

  if(!furl) return 0;

  furl_length = strlen(furl);

  if(flag & URL_FURL_REAL_PATH) {
    wlroot = getenv("WLROOT");
  }
  if(wlroot == 0)
    wlroot = "";
  
  static_alloc(&path, &path_size, strlen(wlroot) + furl_length + 32);
  
  sprintf(path, "%s%s%s.store", wlroot, (wlroot ? "/" : ""), furl);

  return path;
}

char* url_netloc(url_t* object)
{
  static char* netloc = 0;
  static int netloc_size = 0;

  static_alloc(&netloc, &netloc_size, object->pool_size);

  if(object->port) {
    sprintf(netloc, "%s:%s", object->host, object->port);
  } else {
    if(!object->host) {
      fprintf(stderr, "url_netloc: expected host for %s\n", url_url(object));
      return 0;
    } else {
      strcpy(netloc, object->host);
    }
  }

  return netloc;
}

char* url_auth(url_t* object)
{
  static char* auth = 0;
  static int auth_size = 0;

  static_alloc(&auth, &auth_size, object->pool_size);

  if(object->user && object->passwd) {
    sprintf(auth, "%s:%s", object->user, object->passwd);
  } else {
    auth[0] = '\0';
  }

  return auth;
}

char* url_all_path(url_t* object)
{
  static char* path = 0;
  static int path_size = 0;

  static_alloc(&path, &path_size, object->pool_size);

  path[0] = '\0';

  if(!(object->info & URL_INFO_RELATIVE) ||
     !(object->info & URL_INFO_RELATIVE_PATH))
    strcat(path, "/");
  strcat(path, object->path);
  if(object->params) {
    strcat(path, ";");
    strcat(path, object->params);
  }
  if(object->query) {
    strcat(path, "?");
    strcat(path, object->query);
  }

  return path;
}

static void url_string(url_t* object, char** stringp, int* string_sizep, int flags)
{
  char* string;
  /*
   * + 16 is more than enough for separators & all 
   */
  static_alloc(stringp, string_sizep, object->pool_size + 16);

  string = *stringp;

  string[0] = '\0';
  if(object->scheme) {
    strcat(string, object->scheme);
    strcat(string, ":");
  }
  if(object->host || object->user || object->passwd || object->port) {
    strcat(string, (flags & URL_STRING_FURL_STYLE ? "/" : "//"));
    if(object->user) {
      strcat(string, object->user);
      if(object->passwd) {
	strcat(string, ":");
	strcat(string, object->passwd);
      }
      strcat(string, "@");
    }
    if(object->host) strcat(string, object->host);
    if(object->port) {
      strcat(string, ":");
      strcat(string, object->port);
    }
  }
  if(!(object->info & URL_INFO_RELATIVE) ||
     !(object->info & URL_INFO_RELATIVE_PATH))
    strcat(string, "/");
  if(flags & URL_STRING_ROBOTS_STYLE) {
    strcat(string, "robots.txt");
  } else {
    strcat(string, object->path);
    if(object->params) {
      strcat(string, ";");
      strcat(string, object->params);
    }
    if(object->query) {
      strcat(string, "?");
      strcat(string, object->query);
    }
    if(object->frag && (flags & URL_STRING_URL_STYLE) && !(flags & URL_STRING_URL_NOHASH_STYLE)) {
      strcat(string, "#");
      strcat(string, object->frag);
    }
  }
}

void url_free(url_t* object)
{
  if(object->url) free(object->url);
  if(object->furl) free(object->furl);
  if(object->robots) free(object->robots);
  free(object->pool);
  free(object);
}

char* url_port(url_t* object)
{
  if(object->port) return object->port;
  switch(object->info & URL_INFO_SCHEME_MASK) {
  case URL_INFO_HTTP:
    return "80";
  case URL_INFO_FTP:
    return "21";
  case URL_INFO_NEWS:
    return "119";
  default:
    return "0";
  }
}

/*
 * Does not implement the multiple port specification :80,81.
 * I wonder what kind of server support it, anyway.
 */
void url_parse(url_t* object)
{
  char* p;

  /*
   *  This parsing code is based on
   *   draft-ietf-uri-relative-url-06.txt Section 2.4
   */
  /* 2.4.1 frag */
  if(p = strrchr(object->pool, '#')) {
    object->frag = p + 1;
    *p = '\0';
  }
  p = object->pool;
  /* 2.4.2 scheme */
  {
    char* start = p;
    char* end;
    while(*start && isspace(*start))
      start++;
    end = start;
    while(*end && ( isalnum(*end) || *end == '+' || *end == '.' || *end == '-'))
      end++;
    if(*end != '\0' && end > start && *end == ':') {
      object->scheme = start;
      *end = '\0';
      p = end + 1;
    }
  }
  /*
   * 2.4.3 netloc
   * Never bother to find the netloc if there is no scheme.
   * It may even lead to errors if done (//foo.bar/dir/file.html for instance)
   */
  if(object->scheme) {
    char* start = p;
    char* end;

    if(start[0] == '/' && start[1] == '/') {
      /*
       * Tolerate /// 
       */
      while(*start && *start == '/')
	start++;
      end = start;
      while(*end && *end != '/')
	end++;
      p = *end ? end + 1 : end;
      *end = '\0';
      /*
       * Decode authentication information.
       */
      {
	char* auth_end;
	if(auth_end = strchr(start, '@')) {
	  char* auth_start = start;
	  *auth_end = '\0';
	  start = auth_end + 1;

	  if(object->passwd = strchr(auth_start, ':')) {
	    *object->passwd++ = '\0';
	  }
	  object->user = auth_start;
	}
      }
      if(end > start) {
	char* tmp;
	*end = '\0';
	object->host = start;
	if(tmp = strrchr(start, ':')) {
	  *tmp = '\0';
	  object->port = tmp + 1;
	}
      }
    }
  }

  if(!object->scheme || !object->host) {
    object->info |= URL_INFO_RELATIVE;
  }

  /* 2.4.4 query */
  object->query = strchr(p, '?');
  if(object->query) {
    *object->query = '\0';
    object->query++;
  }
  /* 2.4.5 query */
  object->params = strchr(p, ';');
  if(object->params) {
    *object->params = '\0';
    object->params++;
  }
  /* Multiple / in path are always mistakes */
  {
    char* read = p;
    char* write = p;
    int slash = 0;
    if((object->info & URL_INFO_RELATIVE) &&
       *p != '/') {
      object->info |= URL_INFO_RELATIVE_PATH;
    }
    while(*read && *read == '/')
      read++;
    while(*read) {
      if(slash && *read == '/') {
	read++;
	slash = 1;
      } else {
	*write++ = *read++;
	slash = 0;
      }
    }
    *write = '\0';
  }
  object->path = p;

  if(object->scheme == 0 &&
     object->host == 0 &&
     object->port == 0 &&
     object->path[0] == '\0' &&
     object->params == 0 &&
     object->query == 0 &&
     object->frag == 0 &&
     object->user == 0 &&
     object->passwd == 0) {
    object->info |= URL_INFO_EMPTY;
  }
}

#define MAX_URL_SIZE 512

/*
 * rfc1808 describes relative URLs.
 * Some modifications have been done to accomodate effective net usage:
 * . http:/french/index.html is a relative URL (rfc1808 says it is an absolute)
 *   provided that the scheme is the same as the scheme of the base url.
 */

static url_t* _relative = 0;

url_t* url_abs(url_t* base, char* relative_string, int relative_length)
{
  if(_relative == 0) {
#define DUMMY "http://www.dummy.org/dir/file"
    _relative = url_alloc(DUMMY, strlen(DUMMY));
#undef DUMMY
  }
  if(url_realloc(_relative, relative_string, relative_length) != URL_CANNONICAL)
    return 0;

  return url_abs_1(base, _relative);
}

static url_t* absolute = 0;

url_t* url_abs_1(url_t* base, url_t* relative)
{
  static char* path = 0;
  static int path_size = 0;

  int no_relative_path = 0;

  if(relative->info & URL_INFO_EMPTY)
    return base;
  if(!(relative->info & URL_INFO_RELATIVE))
    return relative;

  if(absolute == 0) {
#define DUMMY "http://www.dummy.org/dir/file"
    absolute = url_alloc(DUMMY, strlen(DUMMY));
#undef DUMMY
  }

  if(absolute->pool_size < base->pool_size + relative->pool_size) {
    absolute->pool_size = base->pool_size + relative->pool_size;
    absolute->pool = (char*)srealloc(absolute->pool, absolute->pool_size);
  }
  url_clear(absolute);

  /*
   * Build the new absolute path by merging relative and base.
   */
  {
    static_alloc(&path, &path_size,
		 (relative->path ? strlen(relative->path) : 0) +
		 (base->path ? strlen(base->path) : 0) + 1);
    path[0] = '\0';

    /* 
     * Move the base path to path, striping the last file name.
     */
    {
      char* last_slash;
      if(base->path && (last_slash = strrchr(base->path, '/'))) {
	/* + 1 means that we want to keep the trailing / */
	int length = last_slash - base->path + 1;
	memcpy(path, base->path, length);
	path[length] = '\0';
      }
    }
    
    /*
     * If the relative url path is null or empty, keep the base path if any
     */
    if(relative->path == 0 ||
       relative->path[0] == '\0') {
      if(base->path) strcpy(path, base->path);
      no_relative_path = 1;
    /*
     * If the relative url path is absolute, override base
     */
    } else if(!(relative->info & URL_INFO_RELATIVE_PATH)) {
      strcpy(path, relative->path);
      minimal_path(path, -1);
    /*
     * If the relative url path is relative, append to dirname of base path
     * and reduce . and ..
     */
    } else {
      strcat(path, relative->path);
      minimal_path(path, -1);
    }
  }

  /*
   * Recombine components from base and relative into absolute
   */
  {
    char* tmp;
    char* p = absolute->pool;
    int length;
#define merge(w) \
    if(tmp) { \
      strcpy(p, tmp); \
      absolute->w = p; \
      p += strlen(tmp) + 1; \
    } 
    tmp = base->scheme ? base->scheme : relative->scheme;
    merge(scheme);
    tmp = base->host;
    merge(host);
    tmp = base->port;
    merge(port);
    tmp = path;
    if(path[0] != '\0') {
      merge(path);
    } else {
      absolute->path = p;
      *p++ = '\0';
    }
    if(no_relative_path) {
      tmp = relative->params ? relative->params : base->params;
      merge(params);
      tmp = relative->query ? relative->query : base->query;
      merge(query);
      tmp = relative->frag ? relative->frag : base->frag;
      merge(frag);
    } else {
      tmp = relative->params;
      merge(params);
      tmp = relative->query;
      merge(query);
      tmp = relative->frag;
      merge(frag);
    }
    tmp = relative->user ? relative->user : base->user;
    merge(user);
    tmp = relative->passwd ? relative->passwd : base->passwd;
    merge(passwd);
  }
#undef merge
  absolute->info = URL_INFO_CANNONICAL;
  return absolute;
}


/*
 * Refer to rfc 1738 for constraints imposed to URLs. Some
 * details are derived from actual use of URL which may differ
 * slightly from the specifications.
 */

/*
 * If the SPEC_<part> bit is set, the char may appear unencoded in
 * this part of the URL.
 *
 * If the SPEC_E<part> bit is set, the char must appear encoded in
 * this part of the URL.
 *
 * If both SPEC_<part> and SPEC_E<part> bits are set, the character is
 * left untouched.
 */
#define SPEC_SCHEME	0x0001
#define SPEC_NETLOC	0x0002
#define SPEC_PATH	0x0004
#define SPEC_QUERY	0x0008
#define SPEC_PARAMS	0x0010
#define SPEC_TAG	0x0020
#define SPEC_AUTH	0x0040

#define SPEC_ESCHEME	0x00010000
#define SPEC_ENETLOC	0x00020000
#define SPEC_EPATH	0x00040000
#define SPEC_EQUERY	0x00080000
#define SPEC_EPARAMS	0x00100000
#define SPEC_ETAG	0x00200000
#define SPEC_EAUTH	0x00400000

/* Must be escaped whatever the context is (except scheme and netloc). */
#define SPEC_ESC	SPEC_EPATH|SPEC_EQUERY|SPEC_EPARAMS|SPEC_ETAG|SPEC_EAUTH
/* Allowed unescaped except scheme and netloc. */
#define SPEC_NOR	SPEC_PATH|SPEC_QUERY|SPEC_PARAMS|SPEC_TAG|SPEC_AUTH

/* Can be left unescaped whatever the context is. */
#define SPEC_ALNUM	SPEC_SCHEME|SPEC_NETLOC|SPEC_PATH|SPEC_QUERY|SPEC_PARAMS|SPEC_TAG|SPEC_AUTH

static int specs[256] = {
/*  00 nul  01 soh   02 stx  03 etx   04 eot  05 enq   06 ack  07 bel   */
  0,        SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
/*  08 bs   09 ht    0a nl   0b vt    0c np   0d cr    0e so   0f si    */
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
/*  10 dle  11 dc1   12 dc2  13 dc3   14 dc4  15 nak   16 syn  17 etb   */
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
/*  18 can  19 em    1a sub  1b esc   1c fs   1d gs    1e rs   1f us    */
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
/*  20 sp   21 !     22 "    23 #     24 $    25 %     26 &    27 '     */
  SPEC_ESC, SPEC_NOR, SPEC_ESC, SPEC_ESC,
  SPEC_NOR,
  SPEC_ESC,
  SPEC_EPATH|SPEC_QUERY|SPEC_EPARAMS|SPEC_ETAG|SPEC_EAUTH,
  SPEC_ESC,
/*  28 (    29 )     2a *    2b +     2c ,    2d -     2e .    2f /     */
  SPEC_NOR,
  SPEC_NOR,
  SPEC_NOR,
  SPEC_SCHEME|SPEC_NETLOC|SPEC_NOR,
  SPEC_NOR,
  SPEC_SCHEME|SPEC_NETLOC|SPEC_NOR,
  SPEC_SCHEME|SPEC_NETLOC|SPEC_NOR,
  SPEC_PATH|SPEC_EPATH|SPEC_EQUERY|SPEC_EPARAMS|SPEC_ETAG|SPEC_EAUTH,
/*  30 0    31 1     32 2    33 3     34 4    35 5     36 6    37 7     */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
/*  38 8    39 9     3a :    3b ;     3c <    3d =     3e >    3f ?     */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ESC, SPEC_ESC,
  SPEC_ESC,
  SPEC_EPATH|SPEC_QUERY|SPEC_EPARAMS|SPEC_ETAG|SPEC_EAUTH,
  SPEC_ESC,
  SPEC_ESC,
/*  40 @    41 A     42 B    43 C     44 D    45 E     46 F    47 G     */
  SPEC_ESC,   SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
/*  48 H    49 I     4a J    4b K     4c L    4d M     4e N    4f O     */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
/*  50 P    51 Q     52 R    53 S     54 T    55 U     56 V    57 W     */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
/*  58 X    59 Y     5a Z    5b [     5c \    5d ]     5e ^    5f _     */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ESC,
  SPEC_ESC,   SPEC_ESC,   SPEC_ESC,   SPEC_NOR,
/*  60 `    61 a     62 b    63 c     64 d    65 e     66 f    67 g     */
  SPEC_ESC,   SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
/*  68 h    69 i     6a j    6b k     6c l    6d m     6e n    6f o     */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
/*  70 p    71 q     72 r    73 s     74 t    75 u     76 v    77 w     */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM,
/*  78 x    79 y     7a z    7b {     7c |    7d }     7e ~    7f del   */
  SPEC_ALNUM, SPEC_ALNUM, SPEC_ALNUM, SPEC_ESC,
  SPEC_ESC,   SPEC_ESC,   SPEC_NOR,   SPEC_ESC,

  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,

  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,

  SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC, SPEC_ESC,
  SPEC_ESC, SPEC_ESC, 
};

static char hex2char[128] = {
/*  00 nul  01 soh   02 stx  03 etx   04 eot  05 enq   06 ack  07 bel   */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  08 bs   09 ht    0a nl   0b vt    0c np   0d cr    0e so   0f si    */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  10 dle  11 dc1   12 dc2  13 dc3   14 dc4  15 nak   16 syn  17 etb   */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  18 can  19 em    1a sub  1b esc   1c fs   1d gs    1e rs   1f us    */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  20 sp   21 !     22 "    23 #     24 $    25 %     26 &    27 '     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  28 (    29 )     2a *    2b +     2c ,    2d -     2e .    2f /     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  30 0    31 1     32 2    33 3     34 4    35 5     36 6    37 7     */
    0,      1,       2,      3,       4,      5,       6,      7,    
/*  38 8    39 9     3a :    3b ;     3c <    3d =     3e >    3f ?     */
    8,      9,       '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  40 @    41 A     42 B    43 C     44 D    45 E     46 F    47 G     */
    '\0',   10,      11,     12,      13,     14,      15,     '\0',    
/*  48 H    49 I     4a J    4b K     4c L    4d M     4e N    4f O     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  50 P    51 Q     52 R    53 S     54 T    55 U     56 V    57 W     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  58 X    59 Y     5a Z    5b [     5c \    5d ]     5e ^    5f _     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  60 `    61 a     62 b    63 c     64 d    65 e     66 f    67 g     */
    '\0',   10,      11,     12,      13,     14,      15,     '\0',    
/*  68 h    69 i     6a j    6b k     6c l    6d m     6e n    6f o     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  70 p    71 q     72 r    73 s     74 t    75 u     76 v    77 w     */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
/*  78 x    79 y     7a z    7b {     7c |    7d }     7e ~    7f del   */
    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    '\0',   '\0',    
};

static char char2hex[16] = "0123456789ABCDEF";

int url_cannonicalize_1(char* from, char* to, int spec, char* spec_string)
{
  char* to_base = to;
  char* from_base = from;

  int esc_spec = spec << 16;
  int mask = spec | esc_spec;

  while(*from) {
    unsigned char c;
    int coded;
    if(*from == '%' && from[1] && from[2]) {
      c = (hex2char[from[1] & 0x7f] << 4) | hex2char[from[2] & 0x7f];
      from += 3;
      coded = 1;
    } else {
      c = *from++;
      coded = 0;
    }
    
    if(specs[c] & mask) {
      /*
       * Code if 1) specs[c] is spec AND esc_spec and the
       *            character was already coded. (must be left untouched)
       *         2) specs[c] say it must be escaped.
       */
      int code = ((specs[c] & mask) == mask) ? coded : (specs[c] & esc_spec);
      if(code) {
	*to++ = '%';
	*to++ = char2hex[(c >> 4) & 0xf];
	*to++ = char2hex[c & 0xf];
      } else {
	*to++ = c;
      }
    } else {
      fprintf(stderr, "url_cannonicalize_1: illegal char %c in context %s\n", c, spec_string);
      return -1;
    }
  }
  *to = '\0';

  return to - to_base + 1;
}

char* url_cannonicalize_string(char* url, int url_length, int flag)
{
  url_t* object = url2object(url, url_length);
  if(!object) return 0;
  url_string(object, &object->url, &object->url_size, flag);
  return object->url;
}

int url_cannonicalize(url_t* object)
{
  static char* tmp = 0;
  static int tmp_size = 0;
  int tmp_length = 0;

  if(object->info & URL_INFO_CANNONICAL) return URL_CANNONICAL;
  /* 
   * multiply by 3 to get the maximum expand length of the URL (all
   * chars converted to %xx sequences.
   */
  static_alloc(&tmp, &tmp_size, object->pool_size * 3);

  {
    int length;
#define normalize(w,s) if(object->w) { length = url_cannonicalize_1(object->w, tmp + tmp_length, (s), #s); object->w = tmp + tmp_length; tmp_length += length; if(length < 0) return URL_NOT_CANNONICAL; }
    normalize(scheme, SPEC_SCHEME);
    normalize(host, SPEC_NETLOC);
    normalize(port, SPEC_NETLOC);
    normalize(path, SPEC_PATH);
    normalize(params, SPEC_PARAMS);
    normalize(query, SPEC_QUERY);
    normalize(frag, SPEC_TAG);
    normalize(user, SPEC_AUTH);
    normalize(passwd, SPEC_AUTH);
#undef normalize
  }
  
  /*
   * Normalize case
   */
  if(object->scheme) strlower(object->scheme, -1);
  if(object->host) strlower(object->host, -1);

  /*
   * Sanity checks for relative URLs
   */
  if(object->info & URL_INFO_RELATIVE) {
#define check(w) if(object->w != 0) { fprintf(stderr, "url_cannonicalize: in %s, " #w " cannot be set in relative url, ignored\n", url_url(object)); object->w = 0; object->info &= ~URL_INFO_URL; }
    check(port);
    check(passwd);
    check(user);
  }
  /*
   * Allow ftp,news,http
   */
  if(object->scheme) {
    if(!strcmp(object->scheme, "http")) {
      object->info |= URL_INFO_HTTP;
    } else if(!strcmp(object->scheme, "ftp")) {
      object->info |= URL_INFO_FTP;
    } else if(!strcmp(object->scheme, "file")) {
      object->info |= URL_INFO_FILE;
    } else if(!strcmp(object->scheme, "news")) {
      object->info |= URL_INFO_NEWS;
    } else {
      if(verbose) fprintf(stderr, "url_cannonicalize: %s is not a known scheme for %s\n", object->scheme, url_url(object));
      return URL_NOT_CANNONICAL;
    }
  }
  /*
   * Cleanup domain name variations
   */
  if(object->host) {
    int length = strlen(object->host);
    /*
     * Kill trailing dots
     */
    while(length >= 1 && object->host[length - 1] == '.') {
      length--;
      object->host[length] = '\0';
    }
    if(length == 0) {
      fprintf(stderr, "url_cannonicalize: %s has null netloc\n", url_url(object));
      return URL_NOT_CANNONICAL;
    }
  }
  /*
   * Reduce path name, if not relative URL.
   */
  if(!(object->info & URL_INFO_RELATIVE)) 
    minimal_path(object->path, -1);

  /*
   * Relocate the normalized object.
   */
  if(object->pool_size < tmp_length) {
    object->pool = (char*)srealloc(object->pool, tmp_length);
    object->pool_size = tmp_length;
  }
  memcpy(object->pool, tmp, tmp_length);
#define reloc(w) if(object->w) object->w = object->pool + (object->w - tmp)
  reloc(scheme);
  reloc(host);
  reloc(port);
  reloc(path);
  reloc(params);
  reloc(query);
  reloc(frag);
  reloc(user);
  reloc(passwd);
#undef reloc

  /*
   * Scheme and domain
   */
  object->info |= URL_INFO_CANNONICAL;
  return URL_CANNONICAL;
}

void url_copy(url_t* to, url_t* from)
{
  if(to->pool_size < from->pool_size) {
    to->pool = (char*)srealloc(to->pool, from->pool_size);
    to->pool_size = from->pool_size;
  }

  {
    int length;
    char* p = to->pool;
#define reloc(w) if(from->w) { length = strlen(from->w); memcpy(p, from->w, length + 1); p += length; } else { to->w = 0; }
    reloc(scheme);
    reloc(host);
    reloc(port);
    reloc(path);
    reloc(params);
    reloc(query);
    reloc(frag);
    reloc(user);
    reloc(passwd);
  }
  to->info = from->info;
  if(to->url) {
    if(to->url_size < from->url_size) {
      to->url = (char*)srealloc(to->url, from->url_size);
      to->url_size = from->url_size;
    }
    strcpy(to->url, from->url);
  }
}

void url_dump(url_t* object)
{
  printf("flags: ");
#define flag(w) if(object->info & w) printf(#w " ")
  flag(URL_INFO_CANNONICAL);
  flag(URL_INFO_URL);
  flag(URL_INFO_FURL);
  flag(URL_INFO_ROBOTS);
  flag(URL_INFO_RELATIVE);
  flag(URL_INFO_RELATIVE_PATH);
  flag(URL_INFO_EMPTY);
#undef flag
  printf("\n");

  if(object->info & URL_INFO_CANNONICAL) printf("cannonical form\n");
  if(object->scheme) printf("scheme: %s\n", object->scheme);
  if(object->host) printf("host: %s\n", object->host);
  if(object->port) printf("port: %s\n", object->port);
  if(object->path) printf("path: %s\n", object->path);
  if(object->params) printf("params: %s\n", object->params);
  if(object->query) printf("query: %s\n", object->query);
  if(object->frag) printf("frag: %s\n", object->frag);
  if(object->user) printf("user: %s\n", object->user);
  if(object->passwd) printf("passwd: %s\n", object->passwd);
}
