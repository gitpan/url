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

#include <stdio.h>
#include <salloc.h>

char* smalloc(int size)
{
  char* tmp = (char*)malloc(size);
  if(tmp == 0) {
    fprintf(stderr, "cannot malloc %d bytes\n", size);
    exit(1);
  }
  tmp[0] = '\0';
  return tmp;
}

char* srealloc(char* pointer, int size)
{
  char* tmp = (char*)realloc(pointer, size);
  if(tmp == 0) {
    fprintf(stderr, "cannot realloc %d bytes\n", size);
    exit(1);
  }
  return tmp;
}

void static_alloc(char** tmp, int* tmp_size, int new_size)
{
  if(*tmp_size == 0) {
    *tmp_size = 16;
    *tmp = (char*)smalloc(*tmp_size);
  }
  if(*tmp_size < new_size) {
    *tmp_size = new_size + 1;
    *tmp = (char*)srealloc(*tmp, *tmp_size);
  }
}

