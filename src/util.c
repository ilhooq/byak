#include <stdlib.h>
#include <assert.h>
#include "util.h"

int charToInt(char d)
{
	char str[2];
	str[0] = d;
	str[1] = '\0';
	return (int) strtol(str, NULL, 10);
}


List * list_init()
{
	List *list = malloc(sizeof(List));
	list->first = NULL;
	return list;
}

void list_prepend(List *list, void *data)
{
	assert(list != NULL);
	ListItem *item = malloc(sizeof(ListItem));
	if (item == NULL) {
		exit(EXIT_FAILURE);
	}
	item->data = data;
	/* Insert Element */
	item->next = list->first;
	list->first = item;
}

void list_append(List *list, void *data)
{
	assert(list != NULL);
	if (!list->first) {
		list_prepend(list, data);
		assert(list->first->next == NULL);
		return;
	}

	ListItem *item = malloc(sizeof(ListItem));
	ListItem *current = list->first;
	ListItem *last = NULL;

	if (item == NULL) {
		exit(EXIT_FAILURE);
	}
	item->data = data;
	item->next = NULL;

	while (current != NULL){
		if (!current->next) {
			last = current;
		}
		current = current->next;
	}
	last->next = item;
}

int list_len(List *list) {
	int count = 0;
	ListItem *item = list->first;
	while (item != NULL){
		count++;
		item = item->next;
	}
	return count;
}

void list_destroy(List *list)
{
	if (list == NULL) {
		exit(EXIT_FAILURE);
	}
	while (list->first != NULL){
		ListItem *first = list->first;
		list->first = list->first->next;
		free(first->data);
		free(first);
	}
	free(list);
}
