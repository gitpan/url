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
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <salloc.h>
#include <mpath.h>
#include <split.h>

int minimal_path(char* path, int path_length)
{
  static char* tmp = 0;
  static int tmp_size = 0;

  if(path_length < 0) path_length = strlen(path);

  /*
   * Reduce trivial small paths.
   */
  if((path_length == 1 && path[0] == '.') ||
     (path_length == 2 && path[0] == '.' && path[1] == '/') ||
     (path_length == 2 && path[0] == '.' && path[1] == '.')) {
    path[0] = '\0';
    return 0;
  }
  if((path_length == 1 && path[0] == '/') ||
     (path_length == 2 && path[0] == '/' && path[1] == '.')) {
    path[0] = '/';
    return 1;
  }
  
  /*
   * The code above took care of all the reductions involving 1 or 2
   * characters. If the path is 1 or 2 character long and that we
   * reach this point, no reduction is possible.
   */
  if(path_length < 3) return path_length;

  /*
   * If path does not contain any .. or . or double / do not bother to
   * reduce it.
   */
  if(!strstr(path, "..") &&
     !strstr(path, "/./") &&
     !strstr(path, "//") &&
     !(path[0] == '.' && path[1] == '/') &&
     !(path[path_length - 2] == '/' && path[path_length - 1] == '.'))
    return path_length;

  /*
   * + 1 for trailing '\0', + 1 for trailing /
   */
  static_alloc(&tmp, &tmp_size, path_length + 1 + 1);
  memcpy(tmp, path, path_length);
  tmp[path_length] = '\0';

  /*
   * A / must be append to trailing /. and /.. in order to confirm
   * their status of directory.
   */
  if((tmp[path_length - 1] == '.' &&
      tmp[path_length - 2] == '/') ||
     (tmp[path_length - 1] == '.' &&
      tmp[path_length - 2] == '.' &&
      tmp[path_length - 3] == '/')) {
    tmp[path_length] = '/';
    path_length++;
    tmp[path_length] = '\0';
  }
    
  {
    char** splitted;
    int count;
    int i;
    int index;
    char* p;

    split_inplace(tmp, path_length, &splitted, &count, '/', SPLIT_NOTRIM);

    for(i = 0, index = 0; i < count; i++) {
      /*
       * Ignore /./ and // except for the last /
       */
      if((splitted[i][0] == '\0' && i < (count - 1)) ||
	 (splitted[i][0] == '.' && splitted[i][1] == '\0')) {
	continue;
      } else if(splitted[i][0] == '.' && splitted[i][1] == '.' && splitted[i][2] == '\0') {
	if(index > 0) index--;
      } else {
	splitted[index++] = splitted[i];
      }
    }
    p = path;
    for(i = 0; i < index; i++) {
      int length = strlen(splitted[i]);
      memcpy(p, splitted[i], length);
      p += length;
      if(i < index - 1) *p++ = '/';
    }
    *p = '\0';

    return p - path;
  }
}

