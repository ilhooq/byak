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
