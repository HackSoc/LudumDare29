#ifndef LIST_H
#define LIST_H

/**
 * Doubly-linked list data type
 */
typedef struct List {
	struct List * next;
	struct List * prev;
} List;

#ifndef offsetof
	#define offsetof(st, m) __builtin_offsetof(st, m)
#endif

/**
 * Convert to and from structures and linked lists
 * T - type, LF - list field name, LP - list pointer
 */
#define fromlist(T,LF,LP) ((T*) ((char*)(LP) - offsetof(T, LF)))

void setnext(List * list, List * next);
void setprev(List * list, List * prev);
List * gethead(List * list);
List * gettail(List * list);
List * drop(List * list);
List * insert(List * list, List * insert);
List * append(List * target, List * source);
unsigned int length(List * list);

#endif
