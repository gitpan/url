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

#include <stdlib.h>
#include <split.h>
#include <salloc.h>

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

static char* split_buffer = 0;
static int split_buffer_size = 0;
static char** split_array = 0;
static int split_array_size = 128;
static int split_array_real_size = 0;

void split(char* string, int string_length, char*** splitted, int* count, char separator, int trim)
{
  if(split_buffer_size == 0) {
    split_buffer_size = string_length + 512;
    split_buffer = (char*)smalloc(split_buffer_size);
  }
  if(split_buffer_size < string_length) {
    split_buffer_size = string_length + 512;
    split_buffer = (char*)srealloc(split_buffer, split_buffer_size);
  }
  memcpy(split_buffer, string, string_length);
  split_buffer[string_length] = '\0';

  split_inplace(split_buffer, string_length, splitted, count, separator, trim);
}

void split_inplace(char* split_buffer, int split_buffer_length, char*** splitted, int* count, char separator, int trim)
{
  char* first;
  int index;

  first = split_buffer;
  if(trim) {
    int last = split_buffer_length - 1;
    /*
     * Remove trailing separators.
     */
    while(last >= 0 && split_buffer[last] == separator) {
      split_buffer[last] = '\0';
      last--;
    }
    /*
     * Remove leading separators. 
     */
    while(*first == separator)
      first++;
  }

  static_alloc((char**)&split_array, &split_array_real_size, split_array_size * sizeof(char*));

  index = 0;
  split_array[index++] = first;
  {
    char* p = first;
    while(p = strchr(p, separator)) {
      *p = '\0';
      p++;
      split_array[index++] = p;
      if(index >= split_array_size) {
	split_array_size += 128;
	static_alloc((char**)&split_array, &split_array_real_size, split_array_size * sizeof(char*));
      }
    }
  }
  *count = index;
  *splitted = split_array;
}
