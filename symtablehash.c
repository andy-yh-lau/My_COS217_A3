/*--------------------------------------------------------------------*/
/* symtablehash.c                                                            */
/* Author: Andy Lau                                                   */
/*--------------------------------------------------------------------*/

/* MAKE SURE TO ALWAYS DECLARE ALL VARIABLES AT THE BEG OF THE LOOPS */

#include "symtable.h"
#include <assert.h>
#include <stddef.h>

/* Sequence of integers for the bucket counts */
static const size_t BUCKET_COUNTS[] = {509, 1021, 2039, 4093, 8191, 16381,
                                       32749, 65521};

/* Array of linked lists for the hash table */
struct Binding
{
    const char *key;
    void *value;
    struct Binding *next;
};

struct SymTable
{
    struct Binding **buckets; /* pointer to array of linked-list heads*/
    size_t bucketCount;
    size_t sizeIndex;
};

/*--------------------------------------------------------------------*/
/* Return a hash code for pcKey that is between 0 and uBucketCount-1,
   inclusive. */

static size_t SymTable_hash(const char *pcKey, size_t uBucketCount)
{
    const size_t HASH_MULTIPLIER = 65599;
    size_t u;
    size_t uHash = 0;

    assert(pcKey != NULL);

    for (u = 0; pcKey[u] != '\0'; u++)
        uHash = uHash * HASH_MULTIPLIER + (size_t)pcKey[u];

    return uHash % uBucketCount;
}

/*--------------------------------------------------------------------*/
/* Returns a new SymTable object that contains no bindings, or NULL if
insufficient memory is available */

SymTable_T SymTable_new(void)
{
    SymTable_T oSymTable = malloc(sizeof(struct SymTable));
    if (oSymTable == NULL)
    {
        return NULL;
    }

    /* Initial assignments for sizeIndex and the bucketCount */
    oSymTable->sizeIndex = 0;
    oSymTable->bucketCount = BUCKET_COUNTS[oSymTable->sizeIndex];

    /* Allocates array of buckets, which are initially set as NULL since we're using calloc*/
    oSymTable->buckets = calloc(oSymTable->bucketCount, sizeof(struct Binding *));
    /* Here, you eseneailly want to allocate enough memory for the array of pointers
    since u don't know how many nodes will go into each bucket. We will deal with that in malloc when
    inserting keys. */
    if (oSymTable->buckets == NULL)
    {
        free(oSymTable);
        return NULL;
    }

    return oSymTable;
}

/*--------------------------------------------------------------------*/
/* Frees all memory occupied by oSymTable */

void SymTable_free(SymTable_T oSymTable)
{
    assert(oSymTable != NULL);
    size_t i;
    for (i = 0; i < oSymTable->bucketCount; i++)
    {
        struct Binding *curr = oSymTable->buckets[i];
        while (curr != NULL)
        {
            struct Binding *next = curr->next; /* (*curr).next; */
            free(curr);
            curr = next;
        }
    }

    free(oSymTable->buckets); /* frees array of bucket pointers*/
    free(oSymTable);          /* this frees the symtable itself*/
}

/*--------------------------------------------------------------------*/
/* Returns the number of bindings in oSymTable */

size_t SymTable_getLength(SymTable_T oSymTable)
{
    size_t bindingsCount = 0;
    size_t i;

    assert(oSymTable != NULL);

    for (i = 0; i < oSymTable->bucketCount; i++)
    {
        struct Binding *curr = oSymTable->buckets[i];
        while (curr != NULL)
        {
            bindingsCount++;
            curr = curr->next;
        }
    }
    return bindingsCount;
}

/*--------------------------------------------------------------------*/
/* If oSymTable does not contain a binding with key pcKey, then
SymTable_put must add a new binding to oSymTable consisting of key
pcKey and value pvValue and return 1 (TRUE). Otherwise, the function
must leave oSymTable unchanged and return 0 (FALSE). If insufficient
memory is unavailable, then the function must leave oSymTable unchanged
and return 0 (FALSE) */

int SymTable_put(SymTable_T oSymTable,
                 const char *pcKey, const void *pvValue)
{
}

/*--------------------------------------------------------------------*/
/* If oSymTable contains a binding with pcKey, then replaces binding's
value with pvValue and return the old value. Otherwise, leave the
oSymTable unchanged and return NULL */

void *SymTable_replace(SymTable_T oSymTable,
                       const char *pcKey, const void *pvValue)
{
    size_t i;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    assert(pvValue != NULL);

    for (i = 0; i < oSymTable->bucketCount; i++)
    {
        struct Binding *curr = oSymTable->buckets[i];
        while (curr != NULL)
        {
            if (strcmp(curr->key, pcKey) == 0)
            {
                void *oldValue = curr->value;
                curr->value = pvValue;
                return oldValue;
            }
            curr = curr->next;
        }
    }
    return NULL;
}

/*--------------------------------------------------------------------*/
/* Returns 1 (TRUE) if oSymTable contains a binding whose key is pcKey,
and 0 (FALSE) otherwise */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey)
{

    size_t i;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    for (i = 0; i < oSymTable->bucketCount; i++)
    {
        struct Binding *curr = oSymTable->buckets[i];
        while (curr != NULL)
        {
            if (strcmp(curr->key, pcKey) == 0)
                return 1;
        }
    }
    return 0;
}

/*--------------------------------------------------------------------*/
/* Return the value of the binding within oSymTable whose key is pcKey,
or NULL if no such binding exists */

void *SymTable_get(SymTable_T oSymTable, const char *pcKey)
{

    size_t i;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    for (i = 0; i < oSymTable->bucketCount; i++)
    {
        struct Binding *curr = oSymTable->buckets[i];
        while (curr != NULL)
        {
            if (strcmp(curr->key, pcKey) == 0)
                return curr->value;
        }
    }
    return NULL;
}

/*--------------------------------------------------------------------*/
/* If oSymTable contains a binding with key pcKey, removes that binding
from oSymTable and returns the binding's value. Otherwise, the function
must not change oSymTable and return NULL. */

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey)
{

    size_t i;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    for (i = 0; i < oSymTable->bucketCount; i++)
    {
        struct Binding *curr = oSymTable->buckets[i];

        if (strcmp(curr->key, pcKey) == 0)
        {
            void *bindingValue = curr->value;
            oSymTable->buckets[i] = curr->next;
            free(curr);
            return bindingValue;
        }

        /* Deals with case if binding with key pcKey is the first binding of the bucket */

        /* Deals wiith the case of all other bindings in the bucket */
        while (curr->next != NULL)
        {
            if (strcmp(curr->next->key, pcKey) == 0)
            {
                struct Binding *nodeRemoved = curr->next;
                void *bindingValue = nodeRemoved->value;
                curr->next = nodeRemoved->next;
                free(nodeRemoved);
                return bindingValue;
            }
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
                  const void *pvExtra)
{
}