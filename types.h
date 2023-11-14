/*! \file
* \brief Contains all info about typed
*/

#include <math.h>
#include <assert.h>

/// stack element type
typedef long long elem_t;
/// stack type
typedef struct Stack Stack_t;

/// hash type
typedef unsigned int hash_t;
/// hash function type
typedef hash_t (*hash_f) (const void* obj, size_t size);
/// canary type
typedef long long unsigned int canary_t;
/// dump function
typedef int (*dump_f)(FILE*, const void*, const char*, const char*, const int);

#define PRINT_ELEM_T "%lld"

