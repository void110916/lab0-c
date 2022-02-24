#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/* Notice: sometimes, Cppcheck would find the potential NULL pointer bugs,
 * but some of them cannot occur. You can suppress them by adding the
 * following line.
 *   cppcheck-suppress nullPointer
 */

/*
 * Create empty queue.
 * Return NULL if could not allocate space.
 */
struct list_head *q_new()
{
    struct list_head *q_head = malloc(sizeof(struct list_head));
    if (!q_head)
        return NULL;
    INIT_LIST_HEAD(q_head);
    return q_head;
}

/* Free all storage used by queue */
void q_free(struct list_head *l)
{
    if (!l)
        return;
    while (!list_empty(l)) {
        element_t *e = list_entry(l->next, element_t, list);
        list_del(&e->list);
        free(e->value);
        free(e);
    }
    free(l);
}

/*
 * Attempt to insert element at head of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *e = malloc(sizeof(element_t));
    if (!e) {
        return false;
    }
    e->value = strdup(s);
    if (!e->value) {
        free(e);
        return false;
    }
    list_add(&e->list, head);
    return true;
}

/*
 * Attempt to insert element at tail of queue.
 * Return true if successful.
 * Return false if q is NULL or could not allocate space.
 * Argument s points to the string to be stored.
 * The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *e = malloc(sizeof(element_t));
    if (!e)
        return false;
    e->value = strdup(s);
    if (!e->value) {
        free(e);
        return false;
    }
    list_add_tail(&e->list, head);

    return true;
}

/*
 * Attempt to remove element from head of queue.
 * Return target element.
 * Return NULL if queue is NULL or empty.
 * If sp is non-NULL and an element is removed, copy the removed string to *sp
 * (up to a maximum of bufsize-1 characters, plus a null terminator.)
 *
 * NOTE: "remove" is different from "delete"
 * The space used by the list element and the string should not be freed.
 * The only thing "remove" need to do is unlink it.
 *
 * REF:
 * https://english.stackexchange.com/questions/52508/difference-between-delete-and-remove
 */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    element_t *e = NULL;
    if (!head) {
        return NULL;
    }

    if (!list_empty(head)) {
        e = container_of(head->next, element_t, list);
        list_del(&e->list);
        if (sp) {
            strncpy(sp, e->value, bufsize - 1);
            sp[bufsize - 1] = '\0';
        }
    }
    return e;
}

/*
 * Attempt to remove element from tail of queue.
 * Other attribute is as same as q_remove_head.
 */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    element_t *e = NULL;
    if (!head) {
        return NULL;
    }
    if (!list_empty(head)) {
        e = container_of(head->prev, element_t, list);
        list_del(&e->list);
        if (sp) {
            strncpy(sp, e->value, bufsize - 1);
            sp[bufsize - 1] = '\0';
        }
    }
    return e;
}

/*
 * WARN: This is for external usage, don't modify i t
 * Attempt to release element.
 */
void q_release_element(element_t *e)
{
    free(e->value);
    free(e);
}

/*
 * Return number of elements in queue.
 * Return 0 if q is NULL or empty
 */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int num = 0;
    struct list_head *node = head;
    while (node->next != head) {
        node = node->next;
        num++;
    }
    return num;
}

/*
 * Delete the middle node in list.
 * The middle node of a linked list of size n is the
 * ⌊n / 2⌋th node from the start using 0-based indexing.
 * If there're six element, the third member should be return.
 * Return true if successful.
 * Return false if list is NULL or empty.
 */
bool q_delete_mid(struct list_head *head)
{
    // https://leetcode.com/problems/delete-the-middle-node-of-a-linked-list/

    struct list_head *front, *tail;
    int i = 0;
    if (!head)
        return false;
    if (list_empty(head))
        return false;
    for (front = head->next, tail = head->prev;
         front != tail && front->next != tail;
         front = front->next, tail = tail->prev, i++) {
    }
    list_del(tail);
    q_release_element(list_entry(tail, element_t, list));
    return true;
}

/*
 * Delete all nodes that have duplicate string,
 * leaving only distinct strings from the original list.
 * Return true if successful.
 * Return false if list is NULL.
 *
 * Note: this function always be called after sorting, in other words,
 * list is guaranteed to be sorted in ascending order.
 */
bool q_delete_dup(struct list_head *head)
{
    // https://leetcode.com/problems/remove-duplicates-from-sorted-list-ii/
    if (!head)
        return false;
    element_t *e, *safe, *tmp = NULL;
    // this is list_for_each_entry_safe, but depending safe but entry
    for (e = list_entry((head)->next, element_t, list),
        safe = list_entry(e->list.next, element_t, list);
         &safe->list != (head);
         e = safe, safe = list_entry(safe->list.next, element_t, list)) {
        if (!(strcmp(e->value, safe->value) & 0x7fffffff) ||
            !(strcmp(e->value, safe->value))) {
            list_del(&e->list);
            q_release_element(e);
            tmp = safe;
        } else if (tmp) {
            list_del(&tmp->list);
            q_release_element(tmp);
            tmp = NULL;
        }
    }
    if (tmp) {
        list_del(&tmp->list);
        q_release_element(tmp);
        tmp = NULL;
    }
    return true;
}

/*
 * Attempt to swap every two adjacent nodes.
 */
void q_swap(struct list_head *head)
{
    // https://leetcode.com/problems/swap-nodes-in-pairs/
    struct list_head *next, *nnext;
    for (next = head->next, nnext = next->next;
         (next != head) && (nnext != head);
         next = next->next, nnext = next->next) {
        list_move(next, nnext);
    }
}

/*
 * Reverse elements in queue
 * No effect if q is NULL or empty
 * This function should not allocate or free any list elements
 * (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
 * It should rearrange the existing ones.
 */
void q_reverse(struct list_head *head)
{
    if (!head)
        return;
    struct list_head *iter = head;
    do {
        struct list_head *tmp = iter->next;
        iter->next = iter->prev;
        iter->prev = tmp;
        iter = iter->next;
    } while (iter != head);
}

/*
 * Sort elements of queue in ascending order
 * No effect if q is NULL or empty. In addition, if q has only one
 * element, do nothing.
 */
void q_sort(struct list_head *head)
{
    if (!head)
        return;
    int count = 0;
    // insertion sort
    struct list_head *i, *j, *isafe, *jsafe;
    for (i = (head->next)->next, isafe = i->next; i != head;
         i = isafe, isafe = i->next) {
        for (j = head->next, jsafe = j->next; j != i;
             j = jsafe, jsafe = j->next) {
            char *s1, *s2;
            s1 = list_entry(i, element_t, list)->value;
            s2 = list_entry(j, element_t, list)->value;
            int cmp = strcmp(s1, s2);
            count++;
            if (cmp < 0) {
                list_move(i, j->prev);
                break;
            }
        }
    }
}
