/*--------------------------------------------------------------------*/
/* symtablehash.c                                                     */
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
    SymTable_T oSymTable;
    
    oSymTable= malloc(sizeof(struct SymTable));
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
    size_t i;
    struct Binding *curr;
    struct Binding *next;
    assert(oSymTable != NULL);
    
    for (i = 0; i < oSymTable->bucketCount; i++)
    {
        curr = oSymTable->buckets[i];
        while (curr != NULL)
        {
            next = curr->next; /* (*curr).next; */
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
    size_t i;
    struct Binding *curr;
    size_t bindingsCount = 0;
    
    assert(oSymTable != NULL);

    for (i = 0; i < oSymTable->bucketCount; i++)
    {
        curr = oSymTable->buckets[i];
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

    /* Def change up the variable names later on */
    struct Binding *curr;
    struct Binding *newBinding;
    size_t bucketIndex;
    size_t bucketIndexNew;
    size_t newIndex;
    size_t newBucketCount;

    struct Binding **newBuckets;
    size_t newBucketIndex;
    struct Binding *next;

    size_t i;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    assert(pvValue != NULL);

    bucketIndex = SymTable_hash(pcKey, oSymTable->bucketCount);

    /* Case for when oSymTble contains a binding with key pcKey */
    curr = oSymTable->buckets[bucketIndex];
    while (curr != NULL)
    {
        if (strcmp(curr->key, pcKey) == 0)
            return 0; /* Compares the two strings*/
        curr = curr->next;
    }

    /* Addresses the case for when insufficient memory is unavailable */
    newBinding = malloc(sizeof(struct Binding));
    if (newBinding == NULL)
        return 0;

    if (oSymTable->sizeIndex < 7 && SymTable_getLength(oSymTable) + 1 >= BUCKET_COUNTS[(oSymTable->sizeIndex) + 1])
    {
        newIndex = oSymTable->sizeIndex + 1;
        newBucketCount = BUCKET_COUNTS[newIndex];

        /* Case of inefficient memory, as the allocation failed */
        newBuckets = calloc(newBucketCount, sizeof(struct Binding *));
        if (newBuckets == NULL)
        {
            free(newBinding);
            return 0;
        }

        /* Case for rehashing the existing bindings
         */
        for (i = 0; i < oSymTable->bucketCount; i++)
        {
            curr = oSymTable->buckets[i];
            while (curr != NULL)
            {
                next = curr->next; /* This saves the next node*/
                newBucketIndex = SymTable_hash(curr->key, newBucketCount);
                curr->next = newBuckets[newBucketIndex]; /* Next of curr node is tghe statrt of the bucket at the new hash table */
                newBuckets[newBucketIndex] = curr;       /* start of the bucket at in the new hash table is now the current node */
                curr = next;                             /* Sets the current node to the next node for iteration purposes */
            }
        }

        free(oSymTable->buckets);
        oSymTable->buckets = newBuckets;
        oSymTable->bucketCount = newBucketCount;
        oSymTable->sizeIndex = newIndex;
    }

    /* Computes the bucket index for the new key*/
    bucketIndex = SymTable_hash(pcKey, oSymTable->bucketCount);
    newBinding->key = pcKey;
    newBinding->value = pvValue;
    newBinding->next = oSymTable->buckets[bucketIndex];
    oSymTable->buckets[bucketIndex] = newBinding;

    /* Successful insertion*/
    return 1;
}

/*--------------------------------------------------------------------*/
/* If oSymTable contains a binding with pcKey, then replaces binding's
value with pvValue and return the old value. Otherwise, leave the
oSymTable unchanged and return NULL */

void *SymTable_replace(SymTable_T oSymTable,
                       const char *pcKey, const void *pvValue)
{
    size_t bucketIndex;
    struct Binding *curr;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    assert(pvValue != NULL);

    bucketIndex = SymTable_hash(pcKey, oSymTable->bucketCount);

    curr = oSymTable->buckets[bucketIndex];

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
    return NULL;
}

/*--------------------------------------------------------------------*/
/* Returns 1 (TRUE) if oSymTable contains a binding whose key is pcKey,
and 0 (FALSE) otherwise */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey)
{

    size_t bucketIndex;
    struct Binding *curr;

    bucketIndex = SymTable_hash(pcKey, oSymTable->bucketCount);
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    curr = oSymTable->buckets[bucketIndex];

    while (curr != NULL)
    {
        if (strcmp(curr->key, pcKey) == 0)
            1;
        curr = curr->next;
    }
    return 0;
}

/*--------------------------------------------------------------------*/
/* Return the value of the binding within oSymTable whose key is pcKey,
or NULL if no such binding exists */

void *SymTable_get(SymTable_T oSymTable, const char *pcKey)
{

    size_t bucketIndex;
    struct Binding *curr;

    bucketIndex = SymTable_hash(pcKey, oSymTable->bucketCount);
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    curr = oSymTable->buckets[bucketIndex];

    while (curr != NULL)
    {
        if (strcmp(curr->key, pcKey) == 0)
            return curr->value;
        curr = curr->next;
    }
    return NULL;
}

/*--------------------------------------------------------------------*/
/* If oSymTable contains a binding with key pcKey, removes that binding
from oSymTable and returns the binding's value. Otherwise, the function
must not change oSymTable and return NULL. */

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey)
{

    size_t bucketIndex;
    struct Binding *curr;
    void *bindingValue;
    struct Binding *nodeRemoved;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    bucketIndex = SymTable_hash(pcKey, oSymTable->bucketCount);
    curr = oSymTable->buckets[bucketIndex];

    if (strcmp(curr->key, pcKey) == 0)
    {
        bindingValue = curr->value;
        oSymTable->buckets[bucketIndex] = curr->next;
        free(curr);
        return bindingValue;
    }

    /* Deals with case if binding with key pcKey is the first binding of the bucket */

    /* Deals wiith the case of all other bindings in the bucket */
    while (curr->next != NULL)
    {
        if (strcmp(curr->next->key, pcKey) == 0)
        {
            nodeRemoved = curr->next;
            bindingValue = nodeRemoved->value;
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
                  const void *pvExtra)
{

    size_t i;
    struct Binding *curr;

    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    for (i = 0; i < oSymTable->bucketCount; i++)
    {
        curr = oSymTable->buckets[i];
        while (curr != NULL)
        {
            pfApply(curr->key, curr->value, (void *)pvExtra);
            curr = curr->next;
        }
    }
}