/*
 * Sample run
 *
 * bash$ ./t_url < t_url.input > 1
 * url_cannonicalize_1: illegal char / in context SPEC_NETLOC
 * bash$ ./t_url < 1 > 2
 * bash$ ./t_url < 2 > 3
 * bash$ diff 2 3
 * bash$ 
 *
 * t_url2.input contains tests about convertion to absolute
 * URL from relative URLs and base.
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <string.h>
#include <stdio.h>

#include "url.h"

#define DUMMY "http://www.dumy.fr/"

#define BUFFER_SIZE 1024

char buffer[BUFFER_SIZE];

main(argc, argv)
int argc;
char** argv;
{
  url_t* url = url_alloc(DUMMY, strlen(DUMMY));
  url_t* base_url = url_alloc(DUMMY, strlen(DUMMY));

  while(fgets(buffer, BUFFER_SIZE, stdin)) {
    int length;
    if((buffer[0] == '#' && buffer[1] == '\n') ||
       (buffer[0] == '#' && buffer[1] == ' '))
      continue;
    /* - 1 to suppress the trailing newline. */
    fprintf(stderr, "#\n# %s#\n", buffer);
    buffer[strlen(buffer) - 1] = '\0';
    {
      char* base;
      base = strchr(buffer, ' ');
      if(base) {
	*base++ = '\0';
      }
      if(url_realloc(url, buffer, strlen(buffer)) == URL_CANNONICAL) {
	fprintf(stderr, "url: %s\n", url_url(url));
	fprintf(stderr, "furl: %s\n", url_furl(url));
	if(argv[1])
	  url_dump(url);
	if(base && (url->info & URL_INFO_RELATIVE)) {
	  if(url_realloc(base_url, base, strlen(base)) == URL_CANNONICAL) {
	    url_t* absolute = url_abs_1(base_url, url);
	    fprintf(stderr, "url base: %s\n", url_url(base_url));
	    if(absolute) {
	      fprintf(stderr, "url absolute: %s\n", url_url(absolute));
	      if(argv[1])
		url_dump(absolute);
	    }
	  }
	}
      }
    }
  }
  url_free(url);
/*  printf("%s\n", url_furl(url));*/
}
