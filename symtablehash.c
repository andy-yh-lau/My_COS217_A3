/*--------------------------------------------------------------------*/
/* symtablehash.c                                                     */
/* Author: Andy Lau                                                   */
/*--------------------------------------------------------------------*/

/* Hash-table-based implementation of symbol table that maps string
keys to void* values. Each key is stored with defensive copy to ensure
ensure ownership by symbol table. Collisions handled via separate
chaining with linked lists within each bucket. When symbol table 
becomes full, resizes and rehahses all bindings into larger bucket 
array. */

#include "symtable.h"
#include <assert.h>
#include <string.h>

/* List of prime number used for hash table sizes. Each number
represents number of buckets for particular binding counts. When 
table grows, moves to the next prime count */
static const size_t BUCKET_COUNTS[] = {509, 1021, 2039, 4093, 8191, 
                                        16381, 32749, 65521};

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
    /* Stores total nubmer of bindings in SymTable */
    size_t bindingsCount;
    /* Stores indx into BUCKET_COUNTS array */
    size_t bucketSizeIndex;
};

/*--------------------------------------------------------------------*/

/* Return a has code for pcKey that is between 0 and uBucketCount-1, 
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

SymTable_T SymTable_new(void)
{
    SymTable_T oSymTable;
    
    oSymTable = malloc(sizeof(struct SymTable));
    if (oSymTable == NULL)
    {
        return NULL;
    }

    oSymTable->bucketSizeIndex = 0;
    oSymTable->bindingsCount = 0;

    /* Allocates array of buckets that are initially set as NULL */
    oSymTable->buckets = calloc(BUCKET_COUNTS
    [oSymTable->bucketSizeIndex], sizeof(struct Binding *));
  
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
    
    /* Free every binding in every binding */
    for (i = 0; i < BUCKET_COUNTS[oSymTable->bucketSizeIndex]; i++)
    {
        curr = oSymTable->buckets[i];
        while (curr != NULL)
        {
            next = curr->next; 

            /* Free the key copy */
            free(((void*)curr->key));
            /* Free the current binding node */ 
            free(curr);   

            curr = next;
        }
    }

    /* Free the bucket array */
    free(oSymTable->buckets); /* Free the bucket array */
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
    
    struct Binding *curr, *newBinding, *next;
    struct Binding **newBuckets;
    size_t bucketIndex, newBucketIndex, newBucketCount, 
    newBucketSizeIndex;
    size_t i;
    char *keyCopy;
    
    assert(oSymTable != NULL);
    assert(pcKey != NULL);
    assert(pvValue != NULL);

    /* Compute the current bucket index for pcKey */
    bucketIndex = SymTable_hash(pcKey, 
    BUCKET_COUNTS[oSymTable->bucketSizeIndex]);

    /* Traverse through all bindings to check if key already exists */
    curr = oSymTable->buckets[bucketIndex];
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
    keyCopy = malloc(strlen(pcKey) + 1);
    
    /* Handle condition of insufficient memory for defensive copy 
    of key string */
    if (keyCopy == NULL) {
        free(newBinding);
        return 0; 
    }

    /* Copy string into newly allocated memory */
    strcpy(keyCopy, pcKey);

    /* Resize the symbol table if the load factor (number of bindings 
    divided by the number of buckets) exceeds 1 but not at max size */
    if (oSymTable->bucketSizeIndex < 7 && oSymTable->bindingsCount + 1 
    > BUCKET_COUNTS[(oSymTable->bucketSizeIndex)])
    {
        newBucketSizeIndex = oSymTable->bucketSizeIndex + 1;
        newBucketCount = BUCKET_COUNTS[newBucketSizeIndex];

        /* Allocate memory for larger array of buckets  */
        newBuckets = calloc(newBucketCount, sizeof(struct Binding *));
        if (newBuckets == NULL)
        {
            free(newBinding);
            free(keyCopy);
            return 0;
        }

        /* Rehash all existing bindings into the symbol table */
        for (i = 0; i < BUCKET_COUNTS[oSymTable->bucketSizeIndex]; i++)
        {
            curr = oSymTable->buckets[i];
            while (curr != NULL)
            {
                next = curr->next; 
                newBucketIndex = SymTable_hash(curr->key, 
                newBucketCount);
                
                /* Insert binding at the front of new bucket */
                curr->next = newBuckets[newBucketIndex]; 
                newBuckets[newBucketIndex] = curr;  
                
                /* Sets current node to next node in bucket */
                curr = next;                            
            }
        }

        /* Replace old buckets in oSymTable with new larger, array */
        free(oSymTable->buckets);
        oSymTable->buckets = newBuckets;
        oSymTable->bucketSizeIndex = newBucketSizeIndex;
    }

    /* Compute bucket index again in case symbol table was resized */
    bucketIndex = SymTable_hash(pcKey, 
    BUCKET_COUNTS[oSymTable->bucketSizeIndex]);

    /* Insert new binding at the front of the bucket chain */
    newBinding->key = keyCopy;
    newBinding->value = pvValue;
    newBinding->next = oSymTable->buckets[bucketIndex];
    oSymTable->buckets[bucketIndex] = newBinding;

    /* Increases number of bindings by 1 */
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
    assert(pvValue != NULL);

    bucketIndex = SymTable_hash(pcKey, 
    BUCKET_COUNTS[oSymTable->bucketSizeIndex]);

    curr = oSymTable->buckets[bucketIndex];

    while (curr != NULL)
    {
        if (strcmp(curr->key, pcKey) == 0)
        {
            void *oldValue = (void*)curr->value;
            curr->value = pvValue;
            return oldValue;
        }
        curr = curr->next;
    }
    return NULL;
}

/*--------------------------------------------------------------------*/

int SymTable_contains(SymTable_T oSymTable, const char *pcKey)
{

    size_t bucketIndex;
    struct Binding *curr;

    bucketIndex = SymTable_hash(pcKey, 
    BUCKET_COUNTS[oSymTable->bucketSizeIndex]);
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    curr = oSymTable->buckets[bucketIndex];

    while (curr != NULL)
    {
        if (strcmp(curr->key, pcKey) == 0)
            return 1;
        curr = curr->next;
    }
    return 0;
}

/*--------------------------------------------------------------------*/

void *SymTable_get(SymTable_T oSymTable, const char *pcKey)
{

    size_t bucketIndex;
    struct Binding *curr;

    bucketIndex = SymTable_hash(pcKey, 
    BUCKET_COUNTS[oSymTable->bucketSizeIndex]);
    assert(oSymTable != NULL);
    assert(pcKey != NULL);

    curr = oSymTable->buckets[bucketIndex];

    while (curr != NULL)
    {
        if (strcmp(curr->key, pcKey) == 0)
            return (void*)curr->value;
        curr = curr->next;
    }
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

    bucketIndex = SymTable_hash(pcKey, 
    BUCKET_COUNTS[oSymTable->bucketSizeIndex]);
    curr = oSymTable->buckets[bucketIndex];

    /* Nothing to remove if the corresponding bucket chain is empty */
    if (curr == NULL) return NULL;

    /* Check if first binding of the bucket chain matches pcKey */
    if (strcmp(curr->key, pcKey) == 0)
    {
        bindingValue = curr->value;
        oSymTable->buckets[bucketIndex] = curr->next;
        free((void*)curr->key);
        free(curr);
        oSymTable->bindingsCount--;
        return bindingValue;
    }

    /* Otherwise, search through rest of bindings in bucket chain */
    while (curr->next != NULL)
    {
        /* Handle condition for if binding with pcKey exists */
        if (strcmp(curr->next->key, pcKey) == 0)
        {
            nodeRemoved = (void)*curr->next;
            bindingValue = nodeRemoved->value;
            curr->next = nodeRemoved->next;
            free(nodeRemoved->key);
            free(nodeRemoved);
            oSymTable->bindingsCount--; 
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

    size_t i;
    struct Binding *curr;

    assert(oSymTable != NULL);
    assert(pfApply != NULL);

    /* Iterate over all buckets and bindings */
    for (i = 0; i < BUCKET_COUNTS[oSymTable->bucketSizeIndex]; i++)
    {
        curr = oSymTable->buckets[i];
        while (curr != NULL)
        {
            (void)(*pfApply)(curr->key, (void*) curr->value, (void*)pvExtra);
            curr = curr->next;
        }
    }
}