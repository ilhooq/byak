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

#ifndef UTIL_H
#define UTIL_H

/*
 * Save method to convert a char to a int
 */
int charToInt(char d);

/**
 * Linked list definition
 */
struct s_ListItem {
	void *data;
	struct s_ListItem *next;
};

typedef struct s_ListItem ListItem;

typedef struct {
	int sizeOfData;
	ListItem *first;
} List;

List * list_init();
void list_prepend(List *list, void *data);
void list_append(List *list, void *data);
int list_len(List *list);
void list_destroy(List *list);
#endif
