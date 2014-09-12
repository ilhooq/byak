/**
* Byak, a UCI chess engine.
* Copyright (C) 2013  Sylvain Philip
* 
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
 
#ifndef TIME_H
#define TIME_H

#if !defined(_WIN32) && !defined(_WIN64)

/* Linux - Unix */
#include <sys/time.h>
/**
 Get timestamp in ms
**/
static int GetTickCount() 
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec * 1000 + t.tv_usec / 1000;
}

#else

/* windows and Mingw */
#include <windows.h>

#endif

#define GET_TIME() ((int)(GetTickCount()))

#endif
