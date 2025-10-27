/*--------------------------------------------------------------------*/
/* symtablelist.c                                                     */
/* Author: Andy Lau                                                   */
/*--------------------------------------------------------------------*/

/* Linked-list implementation of symbol table that maps string keys to 
void* values. Each key is stored with defensive copy to ensure
ownership by the symbol table. Symbol table maintains a singly linked 
list of bindings, where each binding contains a key-value pair. */

#include "symtable.h"
#include <assert.h>
#include <string.h>

/*--------------------------------------------------------------------*/

/* Each Binding represents a key-value pair in hash table bucket */
struct Binding
{
    /* Pointer to the key string (defensive copy) */
    char *key;
    /* Pointer to the associated value */
    const void *value;
    /* Pointer to the next binding in the linked list */
    struct Binding *next;
};

struct SymTable
{
    /* Head of the linked list of bindings */
    struct Binding *first;
};

/*--------------------------------------------------------------------*/

SymTable_T SymTable_new(void)
{
    /* Allocate memory for a new symbol table */
    SymTable_T oSymTable = malloc(sizeof(struct SymTable));
    
    /* Handle the case if allocation of memory fails */
    if (oSymTable == NULL)
    {
        return NULL;
    }
    
    /* Initialize the empty list */
    oSymTable->first = NULL;
    return oSymTable;
}

/*--------------------------------------------------------------------*/

void SymTable_free(SymTable_T oSymTable)
{
    struct Binding *curr; 
    struct Binding *next; 
    assert(oSymTable != NULL);

    curr = oSymTable->first;

    /* Traverse through all the bindings in list */
    while (curr != NULL)
    {
        /* Save pointer to next before freeing curr */
        next = curr->next; 

        /* Free the copied key string and the binding struct */
        free(curr->key);
        free(curr);

        /* Move to the next binding */
        curr = next;
    }

    /* Free the symbol table itself */
    free(oSymTable);
}

/*--------------------------------------------------------------------*/

size_t SymTable_getLength(SymTable_T oSymTable)
{
    struct Binding *curr;
    size_t count = 0;
    assert(oSymTable != NULL);

    /* Initialize curr to first binding in list */
    curr = oSymTable->first;

    /* Traverse through all the bindings in list */
    while (curr != NULL)
    {
        /* Increment counter for each binding */
        count++; 

        /* Move to the next binding */
        curr = curr->next;
    }
    return count;
}

/*--------------------------------------------------------------------*/

int SymTable_put(SymTable_T oSymTable,
                 const char *pcKey, const void *pvValue)
{

    struct Binding *curr;
    struct Binding *newBinding;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    assert(pvValue != NULL);

    /* Traverse through all bindings to check if key already exists */
    curr = oSymTable->first;
    while (curr != NULL)
    {
        /* Handle condition if binding with pcKey already exists */
        if (strcmp(curr->key, pcKey) == 0)
            return 0; 
        
        /* Otherwise, move on to the next binding */
        curr = curr->next;
    }

    /* Allocate memory for new binding */
    newBinding = malloc(sizeof(struct Binding));
    
    /* Handle condition of insufficient memory for new binding */
    if (newBinding == NULL)
        return 0;

    /* Copy the key string defensively */
    newBinding->key = malloc(strlen(pcKey) + 1);

    /* Handle condition of insufficient memory for defensive copy 
    of key string */
    if (newBinding->key == NULL) {
        free(newBinding);
        return 0; 
    }

    /* Copy string into newly allocated memory */
    strcpy(newBinding->key, pcKey);

    /* Store value pointer */
    newBinding->value = pvValue; 
    /* Insert new binding to head of list */
    newBinding->next = oSymTable->first; 
    /* Update pointer to head of list */
    oSymTable->first = newBinding;
    return 1;
}

/*--------------------------------------------------------------------*/

void *SymTable_replace(SymTable_T oSymTable,
                       const char *pcKey, const void *pvValue)
{

    struct Binding *curr;
    void *oldValue;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    assert(pvValue != NULL);

    /* Initialize curr to first binding in list */
    curr = oSymTable->first;

    /* Traverse through all the bindings in list */
    while (curr != NULL)
    {
        /* Handle condition for if binding with pcKey exists */
        if (strcmp(curr->key, pcKey) == 0)
        {
            /* Save the old value */
            oldValue = curr->value; 
            /* Replace with new value */
            curr->value = pvValue; 
            /* Return the previous old value */
            return oldValue; 
        }

        /* Otherwise, move on to the next binding */
        curr = curr->next;
    }

    /* Handle condition for if no binding with pcKey exists */
    return NULL;
}

/*--------------------------------------------------------------------*/

int SymTable_contains(SymTable_T oSymTable, const char *pcKey)
{
    struct Binding *curr;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Initialize curr to first binding in list */
    curr = oSymTable->first;

    /* Traverse through all the bindings in list */
    while (curr != NULL)
    {
        /* Handle condition for if binding with pcKey exists */
        if (strcmp(curr->key, pcKey) == 0)
            return 1;

        /* Otherwise, move on to the next binding */
        curr = curr->next;
    }
    
    /* Handle condition for if no binding with pcKey exists */
    return 0;
}

/*--------------------------------------------------------------------*/

void *SymTable_get(SymTable_T oSymTable, const char *pcKey)
{
    struct Binding *curr;
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Initialize curr to first binding in list */
    curr = oSymTable->first;

    /* Traverse through all the bindings in list */
    while (curr != NULL)
    {
        /* Handle condition for if binding with pcKey exists */
        if (strcmp(curr->key, pcKey) == 0)
            return curr->value;
        
        /* Otherwise, move on to the next binding */
        curr = curr->next;
    }

    /* Handle condition for if no binding with pcKey exists */
    return NULL;
}

/*--------------------------------------------------------------------*/

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey)
{
    struct Binding *curr;
    void *bindingValue;
    struct Binding *bindingRemoved;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Initialize curr to first binding in list */
    curr = oSymTable->first;

    /* Check if first binding has key pcKey */
    if (strcmp(curr->key, pcKey) == 0)
    {
        bindingValue = curr->value; /* Store value before removal */
        oSymTable->first = curr->next; /* Move head to next binding */
        free(curr->key); /* Free copied key string */
        free(curr); /* Free the Binding struct */
        return bindingValue; 
    }

    /* Check remaining bindings in the list */
    while (curr->next != NULL)
    {
        /* Handle condition for if binding with pcKey exists */
        if (strcmp(curr->next->key, pcKey) == 0)
        {
            bindingRemoved = curr->next;
            
            /* Store value before removal */
            bindingValue = bindingRemoved->value; 
            
            /* Link to next binding after the removed one */
            curr->next = bindingRemoved->next;

            free(bindingRemoved->key); /* Free copied key string */
            free(bindingRemoved); /* Free the Binding struct */
            return bindingValue;
        }

        /* Otherwise, move on to the next binding */
        curr = curr->next;
    }
     /* Handle condition for if no binding with pcKey exists */
    return NULL;
}

/*--------------------------------------------------------------------*/

void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, void *pvValue, 
                  void *pvExtra), const void *pvExtra)
{
    struct Binding *curr;
    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    /* Initialize curr to first binding in list */
    curr = oSymTable->first;

    /* Traverse through all the bindings in list */
    while (curr != NULL)
    {
        /* Apply the function on each key/value */
        pfApply(curr->key, curr->value, (void *)pvExtra);
        /* Move to the next binding */
        curr = curr->next;
    }
}