/*--------------------------------------------------------------------*/
/* symtablelist.c                                                     */
/* Author: Andy Lau                                                   */
/*--------------------------------------------------------------------*/

#include "symtable.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

/*--------------------------------------------------------------------*/
struct Node {
    const char *key; 
    const void *value; 
    struct Node *next; 
};

struct List {
    struct Node *first; 
};

struct SymTable {
    struct List list;
};
/*--------------------------------------------------------------------*/
/* Returns a new SymTable object that contains no bindings, or NULL if 
insufficient memory is available */

SymTable_T SymTable_new(void) {
    SymTable_T oSymTable = malloc(sizeof(struct SymTable));
    if (oSymTable == NULL) {
        return NULL;
    }
    oSymTable->list.first = NULL;
    return oSymTable;
}

/*--------------------------------------------------------------------*/
/* Frees all memory occupied by oSymTable */

void SymTable_free(SymTable_T oSymTable) {
    assert (oSymTable != NULL);
    struct Node *curr = oSymTable->list.first;
    while (curr != NULL) {
        struct Node *next = curr->next; /* (*curr).next; */  
        free(curr);
        curr = next;
    }
}

/*--------------------------------------------------------------------*/
/* Returns the numbert of bindings in oSymTable */

size_t SymTable_getLength(SymTable_T oSymTable) {
    assert(oSymTable != NULL);
    size_t count = 0;
    struct Node *curr = oSymTable->list.first;
    while (curr != NULL) {
        count++;
        curr = curr->next;
    }
    return count;
}

/*--------------------------------------------------------------------*/
/* If oSymTable does not contain a binding with key pcKey, then 
SymTable_put must add a new binding to oSymTable consisting of key 
pcKey and value pvValue and return 1 (TRUE). Otherwise, the function
must leave oSymTable unchanged and return 0 (FALSE). If insufficient 
memory is unavailable, then the function must leave oSymTable unchanged 
and return 0 (FALSE) */

int SymTable_put(SymTable_T oSymTable, 
    const char *pcKey, const void *pvValue) {
       
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    assert(pvValue != NULL);

    struct Node *curr = oSymTable->list.first;
    while (curr != NULL) {
        if (strcmp(curr->key, pcKey) == 0) return 0; /* Compares the two strings*/
        curr = curr->next;
    }

    struct Node *newNode = malloc(sizeof(struct Node));
    if (newNode == NULL) return 0; 

    newNode->key = pcKey;
    newNode->value = pvValue; 
    newNode->next = oSymTable->list.first; 
    oSymTable->list.first = newNode;
    return 1; 

}
    

/*--------------------------------------------------------------------*/
/* If oSymTable contains a binding with pcKey, then replaces binding's 
value with pvValue and return the old value. Otherwise, leave the
oSymTable unchanged and return NULL */

void *SymTable_replace(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue) {
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    assert(pvValue != NULL);

    struct Node *curr = oSymTable->list.first;
    while (curr != NULL) {
        if (strcmp(curr->key, pcKey) == 0) {
            void *oldValue = curr->value;
            curr->value = pvValue;
            return oldValue;
            }
        curr = curr->next;
    }
    return NULL;
    }

/*--------------------------------------------------------------------*/
/* Returns 1 (TRUE) if oSymTable contains a binding whose key is pcKey, 
and 0 (FALSE) otherwise */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    struct Node *curr = oSymTable->list.first;
    while (curr != NULL) {
        if (strcmp(curr->key, pcKey) == 0) return 1;
        curr = curr->next;
    }
    return 0;
}

/*--------------------------------------------------------------------*/
/* Return the value of the binding within oSymTable whose key is pcKey, 
or NULL if no such binding exists */

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    struct Node *curr = oSymTable->list.first;
    while (curr != NULL) {
        if (strcmp(curr->key, pcKey) == 0) return curr->value;
        curr = curr->next;
    }
    return NULL;
}

/*--------------------------------------------------------------------*/
/* If oSymTable contains a binding with key pcKey, removes that binding
from oSymTable and returns the binding's value. Otherwise, the function
must not change oSymTable and return NULL. */

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Double check to make sure that I can call other functions I defined */

    struct Node *curr = oSymTable->list.first;

    /* Deals with case if binding with key pcKey is the first node */
    if (strcmp(curr->key, pcKey) == 0) {
        void *bindingValue = curr->value;
        oSymTable->list.first = curr-> next;
        free(curr);
        return bindingValue;
    }

    /* Deals wiith the case of all other nodes in the list */
    while (curr -> next != NULL) {
        if (strcmp(curr->next->key, pcKey) == 0) {
            struct Node *nodeRemoved = curr->next;
            void *bindingValue = nodeRemoved->value;
            curr->next = nodeRemoved->next;
            free(nodeRemoved);
            return bindingValue;
        } 
        curr = curr->next;
    }
    return NULL;
}

/*--------------------------------------------------------------------*/
/* Must apply function *pfApply to each binding in oSymTable, passing
pvExtra as an extra parameter. That is, the function, must call
(*pfApply) (pcKey, pvValue, pvExtra) for each pcKey/pvValue binding in
oSymTable */

void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra) {
        assert(oSymTable != NULL);
        struct Node *curr = oSymTable->list.first;
        while (curr != NULL) {
            pfApply(curr->key, curr->value, (void *) pvExtra); 
            /* Make sure to cast pvExtra to void * since it's const void * in the method */
             curr = curr->next;
        }
    /* Note tht pdApply is a function pointer here, as it's the
    finction we want to call for each binding*/

    /* pvExtra here is one extra pointer of const void*
    that we'll pass into pfApply each time we call it */
        
    }