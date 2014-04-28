#include <assert.h>
#include <stddef.h>
#include "list.h"

/**
 * Set the next entry of a list.
 * @param list The list to mutate
 * @param next The new next entry
 */
void setnext(List * list, List * next) {
	assert(list != NULL);

	list->next = next;
}

/**
 * Set the previous entry of a list.
 * @param list The list to mutate
 * @param prev The new prev entry
 */
void setprev(List * list, List * prev) {
	assert(list != NULL);

	list->prev = prev;
}

/**
 * Find the head of a list
 * @param list List to search
 */
List * gethead(List * list) {
	List * head;
	for(head = list; head->prev != NULL; head = head->prev);
	return head;
}

/**
 * Find the tail of a list
 * @param list List to search
 */
List * gettail(List * list) {
	List * tail;
	for(tail = list; tail->next != NULL; tail = tail->next);
	return tail;
}

/**
 * Chop the given item out of the list, updating the prev and
 * next. Returns the head of the list (may not have changed)
 * @param list The list item to remove
 */
List * drop(List * list) {
	assert(list != NULL);

	List * head = list->next;

	if(list->prev != NULL) {
		setnext(list->prev, list->next);
		head = gethead(list->prev);
	}
	if(list->next != NULL) {
		setprev(list->next, list->prev);
	}

	return head;
}

/**
 * Insert an item to a list. Returns the head.
 * @param list The list to mutate
 * @param insert The list item to insert
 */
List * insert(List * list, List * insert) {
	assert(insert != NULL);

	setnext(insert, list);
	setprev(insert, NULL);
	if(list != NULL) {
		setprev(list, insert);
	}

	return insert;
}

/**
 * Appends the source list to the target list. Returns the new head.
 * @param target The list to append to
 * @param source The list to append
 */
List * append(List * target, List * source) {
	if(target == NULL) {
		return source;
	}

	List * tail = gettail(target);
	tail->next = source;
	source->prev = tail;

	return target;
}

/**
 * Get the length of the list
 * @param list The list
 */
unsigned int length(List * list) {
	unsigned int out = 0;
	for(List * item = list; item != NULL; item = item->next) {
		out ++;
	}
	return out;
}
