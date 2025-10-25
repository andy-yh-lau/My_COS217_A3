/*--------------------------------------------------------------------*/
/* symtablehash.c                                                            */
/* Author: Andy Lau                                                   */
/*--------------------------------------------------------------------*/

#include "symtable.h"
#include <assert.h>
#include <stddef.h>

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

SymTable_T SymTable_new(void) {

}

/*--------------------------------------------------------------------*/
/* Frees all memory occupied by oSymTable */

void SymTable_free(SymTable_T oSymTable) {

}

/*--------------------------------------------------------------------*/
/* Returns the numbert of bindings in oSymTable */

size_t SymTable_getLength(SymTable_T oSymTable) {

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

    }

/*--------------------------------------------------------------------*/
/* If oSymTable contains a binding with pcKey, then replaces binding's 
value with pvValue and return the old value. Otherwise, leave the
oSymTable unchanged and return NULL */

void *SymTable_replace(SymTable_T oSymTable,
    const char *pcKey, const void *pvValue) {

    }

/*--------------------------------------------------------------------*/
/* Returns 1 (TRUE) if oSymTable contains a binding whose key is pcKey, 
and 0 (FALSE) otherwise */
int SymTable_contains(SymTable_T oSymTable, const char *pcKey) {

}

/*--------------------------------------------------------------------*/
/* Return the value of the binding within oSymTable whose key is pcKey, 
or NULL if no such binding exists */

void *SymTable_get(SymTable_T oSymTable, const char *pcKey) {

}

/*--------------------------------------------------------------------*/
/* If oSymTable contains a binding with key pcKey, removes that binding
from oSymTable and returns the binding's value. Otherwise, the function
must not change oSymTable and return NULL. */

void *SymTable_remove(SymTable_T oSymTable, const char *pcKey) {

}

/*--------------------------------------------------------------------*/
/* Must apply function *pfApply to each binding in oSymTable, passing
pvExtra as an extra parameter. That is, the function, must call
(*pfApply) (pcKey, pvValue, pvExtra) for each pcKey/pvValue binding in
oSymTable */

void SymTable_map(SymTable_T oSymTable,
    void (*pfApply)(const char *pcKey, void *pvValue, void *pvExtra),
    const void *pvExtra) {
        
    }