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
#ifndef _split_h
#define _split_h

/*
 * splitted is NOT allocated using malloc.
 * if trim is true, trailing separators are removed before splitting.
 */
#define SPLIT_TRIM 1
#define SPLIT_NOTRIM 0
/*
 * String is copied in static array and pointers returned point to the static array that
 * at next call.
 */
void split(char* string, int string_length, char*** splitted, int* count, char separator, int trim);
/*
 * String is altered (null replace the separator) and pointers returned point
 * to the original string.
 */
void split_inplace(char* string, int string_length, char*** splitted, int* count, char separator, int trim);

#endif /* split_h */
