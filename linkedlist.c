/*
 * linkedlist.c
 *
 * Based on the implementation approach described in "The Practice 
 * of Programming" by Kernighan and Pike (Addison-Wesley, 1999).
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "emalloc.h"
#include "linkedlist.h"


node_t *new_node(int pid, char *command) {
    assert( command != NULL );

    node_t *temp = (node_t *)emalloc(sizeof(node_t));

    temp->pid = pid;
    temp->command = strdup(command);
    temp->next = NULL;

    return temp;
}


node_t *add_front(node_t *list, node_t *new) {
    new->next = list;
    return new;
}


node_t *add_end(node_t *list, node_t *new) {
    node_t *curr;

    if (list == NULL) {
        new->next = NULL;
        return new;
    }

    for (curr = list; curr->next != NULL; curr = curr->next);
    curr->next = new;
    new->next = NULL;
    return list;
}


node_t *peek_front(node_t *list) {
    return list;
}


node_t *remove_front(node_t *list) {
    if (list == NULL) {
        return NULL;
    }
    node_t *temp = list->next;
    free(list->command);
    free(list);
    return temp;
}


void apply(node_t *list,
           void (*fn)(node_t *list, void *),
           void *arg)
{
    for ( ; list != NULL; list = list->next) {
        (*fn)(list, arg);
    }
}

node_t *remove_p_node(int pid, node_t *list){
    node_t *curr;
    node_t *prev;
    for (curr = list; curr != NULL; curr = curr->next){
        if (curr->pid != pid){
            prev = curr;
            continue;
        }
        if (curr == list){
            list = curr->next;
            free(curr->command);
            free(curr);
            return list;
        }else{
            prev->next = curr->next;
            free(curr->command);
            free(curr);
            return list;
        }
    }
    return list;
}