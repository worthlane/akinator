#ifndef __TREE_H_
#define __TREE_H_

#include <stdio.h>

#include "errors.h"

typedef int node_t;
#ifdef PRINT_NODE
#undef PRINT_NODE
#endif
#define PRINT_NODE " %d "

static const node_t ROOT_DATA = 10;

struct Node
{
    node_t data;

    Node* left;
    Node* right;
};

struct Tree
{
    Node* root;

    size_t size;
};
typedef struct Tree tree_t;

enum class TreeErrors
{
    NONE = 0,

    ALLOCATE_MEMORY,
    EMPTY_TREE,
    INVALID_SYNTAX,

    UNKNOWN
};
int PrintTreeError(FILE* fp, const void* err, const char* func, const char* file, const int line);

#ifdef EXIT_IF_TREE_ERROR
#undef EXIT_IF_TREE_ERROR
#endif
#define EXIT_IF_TREE_ERROR(error)         do                                                          \
                                            {                                                           \
                                                if ((error)->code != (int) TreeErrors::NONE)         \
                                                {                                                       \
                                                    return LogDump(PrintTreeError, error, __func__,  \
                                                                    __FILE__, __LINE__);                \
                                                }                                                       \
                                            } while(0)
#ifdef RETURN_IF_TREE_ERROR
#undef RETURN_IF_TREE_ERROR
#endif
#define RETURN_IF_TREE_ERROR(error)       do                                                          \
                                            {                                                           \
                                                if ((error) != TreeErrors::NONE)                     \
                                                {                                                       \
                                                    return error;                                       \
                                                }                                                       \
                                            } while(0)

Node* NodeCtor(node_t data, Node* left, Node* right, error_t* error);
void  NodeDtor(Node* node);

TreeErrors TreeCtor(tree_t* tree, error_t* error);
void       TreeDtor(tree_t* tree);
void       TreePrefixPrint(FILE* fp, const tree_t* tree);
void       TreePostfixPrint(FILE* fp, const tree_t* tree);
void       TreeInfixPrint(FILE* fp, const tree_t* tree);
void       TreePrefixRead(FILE* fp, tree_t* tree, error_t* error);
int        TreeDump(FILE* fp, const void* nodes, const char* func, const char* file, const int line);

#ifdef DUMP_TREE
#undef DUMP_TREE
#endif
#define DUMP_TREE(tree)  do                                                              \
                            {                                                               \
                                LogDump(TreeDump, (tree), __func__, __FILE__, __LINE__); \
                            } while(0)

#endif
