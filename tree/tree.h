#ifndef __TREE_H_
#define __TREE_H_

#include <stdio.h>

#include "common/errors.h"

typedef char* node_data_t;
#ifdef PRINT_NODE
#undef PRINT_NODE
#endif
#define PRINT_NODE "\"%s\""

static const node_data_t ROOT_DATA = "unknown";

struct Node
{
    node_data_t data;

    Node* left;
    Node* right;
};

struct Tree
{
    Node* root;
};
typedef struct Tree tree_t;

enum class TreeErrors
{
    NONE = 0,

    ALLOCATE_MEMORY,
    EMPTY_TREE,
    INVALID_SYNTAX,
    CYCLED_NODE,
    COMMON_HEIR,

    UNKNOWN
};
int PrintTreeError(FILE* fp, const void* err, const char* func, const char* file, const int line);

#ifdef EXIT_IF_TREE_ERROR
#undef EXIT_IF_TREE_ERROR
#endif
#define EXIT_IF_TREE_ERROR(error)         do                                                            \
                                            {                                                           \
                                                if ((error)->code != (int) TreeErrors::NONE)            \
                                                {                                                       \
                                                    return LogDump(PrintTreeError, error, __func__,     \
                                                                    __FILE__, __LINE__);                \
                                                }                                                       \
                                            } while(0)
#ifdef RETURN_IF_TREE_ERROR
#undef RETURN_IF_TREE_ERROR
#endif
#define RETURN_IF_TREE_ERROR(error)       do                                                            \
                                            {                                                           \
                                                if ((error) != TreeErrors::NONE)                        \
                                                {                                                       \
                                                    return error;                                       \
                                                }                                                       \
                                            } while(0)

Node* NodeCtor(const node_data_t data, Node* left, Node* right, error_t* error);
void  NodeDtor(Node* node);
int   NodeDump(FILE* fp, const void* dumping_node, const char* func, const char* file, const int line);

#ifdef DUMP_NODE
#undef DUMP_NODE
#endif
#define DUMP_NODE(tree)     do                                                              \
                            {                                                               \
                                LogDump(NodeDump, (tree), __func__, __FILE__, __LINE__);    \
                            } while(0)

TreeErrors NodeVerify(const Node* node, error_t* error);

#ifdef CHECK_NODE
#undef CHECK_NODE
#endif
#define CHECK_NODE(node, error)     do                                                              \
                                    {                                                               \
                                        TreeErrors node_err_ = NodeVerify(node, error);             \
                                        if (node_err_ != TreeErrors::NONE)                          \
                                            return node_err_;                                       \
                                    } while(0)

node_data_t ReadNodeData(FILE* fp, error_t* error);

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
#define DUMP_TREE(tree)  do                                                                 \
                            {                                                               \
                                LogDump(TreeDump, (tree), __func__, __FILE__, __LINE__);    \
                            } while(0)

TreeErrors TreeVerify(const tree_t* tree, error_t* error);

#ifdef CHECK_TREE
#undef CHECK_TREE
#endif
#define CHECK_TREE(tree, error)     do                                                              \
                                    {                                                               \
                                        TreeErrors tree_err_ = TreeVerify(tree, error);             \
                                        if (tree_err_ != TreeErrors::NONE)                          \
                                            return tree_err_;                                       \
                                    } while(0)

#endif
