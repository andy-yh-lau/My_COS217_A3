/*--------------------------------------------------------------------*/
/* symtablehash.c                                                     */
/* Author: Andy Lau                                                   */
/*--------------------------------------------------------------------*/

/* Hash-table-based implementation of symbol table that maps string
keys to void* values. Each key is stored with defensive copy to 
ensure ownership by symbol table. Collisions handled via separate
chaining with linked lists within each bucket. When symbol table
becomes full, resizes and rehashes all bindings into a larger bucket
array if there is sufficient memory */

#include "symtable.h"
#include <assert.h>
#include <string.h>

/* List of prime numbers used for hash table sizes. Each number
represents number of buckets for particular binding counts. When hash
table grows, moves to the next prime count */
static const size_t BUCKET_COUNTS[] = {509, 1021, 2039, 4093, 8191,
                                       16381, 32749, 65521};

/* Number of hash table sizes */
static const size_t BUCKET_COUNTS_LEN =
    sizeof(BUCKET_COUNTS) / sizeof(BUCKET_COUNTS[0]);

/* Each Binding represents a key-value pair in hash table bucket */
struct Binding
{
    /* Pointer to the key string (defensive copy) */
    const char *key;
    /* Pointer to the associated value */
    const void *value;
    /* Pointer to next binding in bucket chain */
    struct Binding *next;
};

/* SymTable structure represents overall hash table */
struct SymTable
{
    /* Pointer to array of bucket heads */
    struct Binding **buckets;
    /* Stores total number of bindings in SymTable */
    size_t bindingsCount;
    /* Stores index into BUCKET_COUNTS array */
    size_t bucketSizeIndex;
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

/* Expand oSymTable to the next bucket size if adding one more
binding would not make the load factor exceed 1, provided the table
is not already at its maximum size and memory for a new bucket array
is available*/

static void SymTable_tryExpand(SymTable_T oSymTable) {

    struct Binding *curr, *next;
    struct Binding **newBuckets;
    size_t curBucketCount, newBucketCount;
    size_t newBucketSizeIndex, hashSlot;
    size_t i;

    /* Handle case where hash table is at maximum configured size */
    if (oSymTable->bucketSizeIndex >= BUCKET_COUNTS_LEN - 1) return;

    curBucketCount = BUCKET_COUNTS[oSymTable->bucketSizeIndex];

    /* Handle case where adding one more binding does not warrant 
    an expansion */
    if (oSymTable->bindingsCount + 1 <= curBucketCount) return;

    /* Move up one step in prime number bucket sizes sequence */
    newBucketSizeIndex = oSymTable->bucketSizeIndex + 1;
    newBucketCount = BUCKET_COUNTS[newBucketSizeIndex];

    /* Allocate memory for new buckets array */
    newBuckets = calloc(newBucketCount, sizeof(struct Binding *));
    if(newBuckets == NULL) return; 

    /* Rehash all existing bindings into new buckets array */
    for (i = 0; i < curBucketCount; i++) {
        
        /* Initialize curr to first binding in bucket */
        curr = oSymTable->buckets[i];
        
        /* Traverse through all the bindings in hash bucket */
        while (curr != NULL)
        {
            next = curr->next;

            /* Computes new hash key */
            hashSlot = SymTable_hash(curr->key,
                                            newBucketCount);

            /* Insert binding at the front of new bucket */
            curr->next = newBuckets[hashSlot];
            newBuckets[hashSlot] = curr;

            curr = next;
        }
    }

    /* Swaps in the new buckets array and records new size index */
    free(oSymTable->buckets);
    oSymTable->buckets = newBuckets;
    oSymTable->bucketSizeIndex = newBucketSizeIndex;

}

/*--------------------------------------------------------------------*/

SymTable_T SymTable_new(void)
{
    SymTable_T oSymTable;

    /* Allocate memory for a new symbol table */
    oSymTable = malloc(sizeof(struct SymTable));

    /* Handle case if allocation of memory to symTable pointer fails */
    if (oSymTable == NULL)
    {
        return NULL;
    }

    /* Initialize fields for new symbol table */
    oSymTable->bucketSizeIndex = 0;
    oSymTable->bindingsCount = 0;
    oSymTable->buckets = calloc(BUCKET_COUNTS
                                    [oSymTable->bucketSizeIndex],
                                sizeof(struct Binding *));

    /* Handle case where allocation of memory for array of buckets 
    fails */
    if (oSymTable->buckets == NULL)
    {
        free(oSymTable);
        return NULL;
    }

    return oSymTable;
}

/*--------------------------------------------------------------------*/

void SymTable_free(SymTable_T oSymTable)
{
    size_t i;
    struct Binding *curr;
    struct Binding *next;

    assert(oSymTable != NULL);

    /* Free every binding in every bucket */
    for (i = 0; i < BUCKET_COUNTS[oSymTable->bucketSizeIndex]; i++)
    {
        /* Initialize curr to first binding in bucket */
        curr = oSymTable->buckets[i];
        
        /* Traverse through all the bindings in bucket */
        while (curr != NULL)
        {
            /* Save pointer to next before freeing curr */
            next = curr->next;

            /* Free the key copy */
            free(((void *)curr->key));
            /* Free the current binding node */
            free(curr);

            /* Move to the next binding in bucket */
            curr = next;
        }
    }

    /* Free the bucket array */
    free(oSymTable->buckets);
    /* Free the symbol table structure */
    free(oSymTable);
}

/*--------------------------------------------------------------------*/

size_t SymTable_getLength(SymTable_T oSymTable)
{
    assert(oSymTable != NULL);
    return oSymTable->bindingsCount;
}

/*--------------------------------------------------------------------*/

int SymTable_put(SymTable_T oSymTable,
                 const char *pcKey, const void *pvValue)
{

    struct Binding *curr, *newBinding;
    size_t bucketIndex;
    char *keyCopy;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Compute the current bucket index for pcKey */
    bucketIndex = SymTable_hash(pcKey,
    BUCKET_COUNTS[oSymTable->bucketSizeIndex]);

    /* Traverse through all bindings to check if key already exists */
    curr = oSymTable->buckets[bucketIndex];
    while (curr != NULL)
    {
        /* Handle condition where binding with pcKey already exists */
        if (strcmp(curr->key, pcKey) == 0)
            return 0;

        /* Otherwise, move on to the next binding */
        curr = curr->next;
    }

    /* Expand the hash table and rehash all bindings if load factor 
    exceeds 1 and there is sufficient memory */
    SymTable_tryExpand(oSymTable);

    /* Compute bucket index again in case symbol table was resized */
    bucketIndex = SymTable_hash(pcKey,
    BUCKET_COUNTS[oSymTable->bucketSizeIndex]);

    /* Allocate memory for new binding */
    newBinding = malloc(sizeof(struct Binding));

    /* Handle condition of insufficient memory for new binding */
    if (newBinding == NULL)
        return 0;

    /* Copy the key string defensively */
    keyCopy = malloc(strlen(pcKey) + 1);

    /* Handle condition of insufficient memory for defensive copy
    of key string */
    if (keyCopy == NULL)
    {
        free(newBinding);
        return 0;
    }

    /* Copy string into newly allocated memory */
    strcpy(keyCopy, pcKey);

    /* Insert new binding at the front of the bucket chain */
    newBinding->key = keyCopy;
    newBinding->value = pvValue;
    newBinding->next = oSymTable->buckets[bucketIndex];
    oSymTable->buckets[bucketIndex] = newBinding;

    oSymTable->bindingsCount++;

    /* Successful insertion*/
    return 1;
}

/*--------------------------------------------------------------------*/

void *SymTable_replace(SymTable_T oSymTable,
                       const char *pcKey, const void *pvValue)
{
    size_t bucketIndex;
    struct Binding *curr;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Find hash key for pcKey */
    bucketIndex = SymTable_hash(pcKey,
    BUCKET_COUNTS[oSymTable->bucketSizeIndex]);

    /* Initialize curr to first binding in hash bucket */
    curr = oSymTable->buckets[bucketIndex];

    /* Traverse through all the bindings in hash bucket */
    while (curr != NULL)
    {
        /* Handle condition where binding with pcKey exists */
        if (strcmp(curr->key, pcKey) == 0)
        {
            /* Save old value */
            void *oldValue = (void *)curr->value;
            /* Replace with new value */
            curr->value = pvValue;
            /* Return previous old value */
            return oldValue;
        }

        /* Otherwise, move on to the next binding */
        curr = curr->next;
    }

    /* Handle condition where no binding with pcKey exists */
    return NULL;
}

/*--------------------------------------------------------------------*/

int SymTable_contains(SymTable_T oSymTable, const char *pcKey)
{

    size_t bucketIndex;
    struct Binding *curr;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Find hash key for pcKey */
    bucketIndex = SymTable_hash(pcKey,
    BUCKET_COUNTS[oSymTable->bucketSizeIndex]);

    /* Initialize curr to first binding in hash bucket */
    curr = oSymTable->buckets[bucketIndex];

    /* Traverse through all the bindings in hash bucket */
    while (curr != NULL)
    {
        /* Handle condition where binding with pcKey exists */
        if (strcmp(curr->key, pcKey) == 0)
            return 1;

        /* Otherwise, move on to the next binding */
        curr = curr->next;
    }

    /* Handle condition where no binding with pcKey exists */
    return 0;
}

/*--------------------------------------------------------------------*/

void *SymTable_get(SymTable_T oSymTable, const char *pcKey)
{

    size_t bucketIndex;
    struct Binding *curr;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Find hash key for pcKey */
    bucketIndex = SymTable_hash(pcKey,
    BUCKET_COUNTS[oSymTable->bucketSizeIndex]);

    /* Initialize curr to first binding in hash bucket */
    curr = oSymTable->buckets[bucketIndex];

    /* Traverse through all the bindings in hash bucket */
    while (curr != NULL)
    {
        /* Handle condition where binding with pcKey exists */
        if (strcmp(curr->key, pcKey) == 0)
            return (void *)curr->value;
            
        /* Otherwise, move on to the next binding */  
        curr = curr->next;
    }

    /* Handle condition where no binding with pcKey exists */
    return NULL;
}

/*--------------------------------------------------------------------*/

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey)
{

    size_t bucketIndex;
    struct Binding *curr, *nodeRemoved;
    void *bindingValue;

    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    /* Find hash key for pcKey */
    bucketIndex = SymTable_hash(pcKey,
    BUCKET_COUNTS[oSymTable->bucketSizeIndex]);
    curr = oSymTable->buckets[bucketIndex];

    /* Nothing to remove if the corresponding bucket chain is empty */
    if (curr == NULL)
        return NULL;

    /* Check if first binding of the bucket chain matches pcKey */
    if (strcmp(curr->key, pcKey) == 0)
    {
        bindingValue = (void *)curr->value;
        oSymTable->buckets[bucketIndex] = curr->next;
        free((void *)curr->key);
        free(curr);
        oSymTable->bindingsCount--;
        return bindingValue;
    }

    /* Otherwise, search through rest of bindings in bucket chain */
    while (curr->next != NULL)
    {
        /* Handle condition where binding with pcKey exists */
        if (strcmp(curr->next->key, pcKey) == 0)
        {
            nodeRemoved = curr->next;
            bindingValue = (void *)nodeRemoved->value;
            curr->next = nodeRemoved->next;
            free((void *)nodeRemoved->key);
            free((void *)nodeRemoved);
            oSymTable->bindingsCount--;
            return bindingValue;
        }

        /* Otherwise, move on to the next binding */
        curr = curr->next;
    }

    /* Handle condition where no binding with pcKey exists */
    return NULL;
}

/*--------------------------------------------------------------------*/

void SymTable_map(SymTable_T oSymTable,
                  void (*pfApply)(const char *pcKey, void *pvValue,
                                  void *pvExtra),
                  const void *pvExtra)
{

    size_t i;
    struct Binding *curr;

    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    /* Iterate over all buckets and bindings */
    for (i = 0; i < BUCKET_COUNTS[oSymTable->bucketSizeIndex]; i++)
    {
        /* Initialize curr to first binding in bucket */
        curr = oSymTable->buckets[i];

        /* Traverse through all the bindings in bucket */
        while (curr != NULL)
        {
            /* Apply the function on each key/value */
            (void)(*pfApply)(curr->key, (void *)curr->value, 
            (void *)pvExtra);

            /* Move to the next binding */
            curr = curr->next;
        }
    }
}