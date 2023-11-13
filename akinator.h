#ifndef __AKINATOR_H_
#define __AKINATOR_H_

#include "tree.h"

enum class AkinatorErrors
{
    NONE = 0,

    DATA_FILE,
    ALLOCATE_MEMORY,
    USER_QUIT,
    UNEXPECTED_NODE,
    INVALID_SYNTAX,
    TREE_ERROR,

    UNKNOWN
};
int PrintAkinatorError(FILE* fp, const void* err, const char* func, const char* file, const int line);

#ifdef EXIT_IF_AKINATOR_ERROR
#undef EXIT_IF_AKINATOR_ERROR
#endif
#define EXIT_IF_AKINATOR_ERROR(error)       do                                                          \
                                            {                                                           \
                                                if ((error)->code != (int) AkinatorErrors::NONE)         \
                                                {                                                       \
                                                    return LogDump(PrintAkinatorError, error, __func__,  \
                                                                    __FILE__, __LINE__);                \
                                                }                                                       \
                                            } while(0)
#ifdef RETURN_IF_AKINATOR_ERROR
#undef RETURN_IF_AKINATOR_ERROR
#endif
#define RETURN_IF_AKINATOR_ERROR(error)     do                                                          \
                                            {                                                           \
                                                if ((error) != AkinatorErrors::NONE)                     \
                                                {                                                       \
                                                    return error;                                       \
                                                }                                                       \
                                            } while(0)


AkinatorErrors GuessMode(tree_t* tree, Node* node, const char* data_file, error_t* error);

#endif
