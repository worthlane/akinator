#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "tree.h"
#include "graphs.h"

static void DestructNodes(Node* root);

static void NodesPrefixPrint(FILE* fp, const Node* node);
static void NodesPostfixPrint(FILE* fp, const Node* node);
static void NodesInfixPrint(FILE* fp, const Node* node);

static Node* NodesPrefixRead(FILE* fp, error_t* error);

static inline void SkipSpaces(FILE* fp);
static inline void ParseClosingBracket(FILE* fp, char* read);
static Node* ReadNewNode(FILE* fp, error_t* error);

static void TextTreeDump(FILE* fp, const tree_t* tree);

// ======== GRAPHS =========

static void DrawTreeGraph(const tree_t* tree);

static inline void DrawNodes(FILE* dotf, const Node* node, const int rank);

// =========================

static const char*  NIL               = "nil";
static const size_t MAX_LISP_WORD_LEN = 10;

//-----------------------------------------------------------------------------------------------------

Node* NodeCtor(node_t data, Node* left, Node* right, error_t* error)
{
    assert(error);

    Node* node = (Node*) calloc(1, sizeof(Node));
    if (node == nullptr)
    {
        error->code = (int) TreeErrors::ALLOCATE_MEMORY;
        error->data = "NODE";
        return nullptr;
    }

    node->data  = data;
    node->left  = left;
    node->right = right;

    return node;
}

//-----------------------------------------------------------------------------------------------------

void NodeDtor(Node* node)
{
    assert(node);

    free(node);
}

//-----------------------------------------------------------------------------------------------------

TreeErrors TreeCtor(tree_t* tree, error_t* error)
{
    Node* root = NodeCtor(ROOT_DATA, nullptr, nullptr, error);
    RETURN_IF_TREE_ERROR((TreeErrors) error->code);

    tree->size = 0;
    tree->root = root;

    return TreeErrors::NONE;
}

//-----------------------------------------------------------------------------------------------------

void TreeDtor(tree_t* tree)
{
    DestructNodes(tree->root);

    tree->size = 0;
    tree->root = nullptr;
}

//-----------------------------------------------------------------------------------------------------

static void DestructNodes(Node* root)
{
    if (root->left != nullptr)
        DestructNodes(root->left);

    if (root->right != nullptr)
        DestructNodes(root->right);

    NodeDtor(root);
}

//-----------------------------------------------------------------------------------------------------

int PrintTreeError(FILE* fp, const void* err, const char* func, const char* file, const int line)
{
    assert(err);

    LOG_START(func, file, line);

    const struct ErrorInfo* error = (const struct ErrorInfo*) err;

    switch ((TreeErrors) error->code)
    {
        case (TreeErrors::NONE):
            LOG_END();
            return (int) error->code;

        case (TreeErrors::ALLOCATE_MEMORY):
            fprintf(fp, "CAN NOT ALLOCATE MEMORY FOR %s<br>\n", (const char*) error->data);
            LOG_END();
            return (int) error->code;

        case (TreeErrors::EMPTY_TREE):
            fprintf(fp, "TREE IS EMPTY<br>\n");
            LOG_END();
            return (int) error->code;

        case (TreeErrors::INVALID_SYNTAX):
            fprintf(fp, "UNKNOWN INPUT<br>\n");
            LOG_END();
            return (int) error->code;

        case (TreeErrors::UNKNOWN):
        // fall through
        default:
            fprintf(fp, "UNKNOWN ERROR WITH TREE<br>\n");
            LOG_END();
            return (int) TreeErrors::UNKNOWN;
    }
}

//-----------------------------------------------------------------------------------------------------

void TreePrefixPrint(FILE* fp, const tree_t* tree)
{
    assert(tree);

    NodesPrefixPrint(fp, tree->root);
    fprintf(fp, "\n");
}

//-----------------------------------------------------------------------------------------------------

void TreePostfixPrint(FILE* fp, const tree_t* tree)
{
    assert(tree);

    NodesPostfixPrint(fp, tree->root);
    fprintf(fp, "\n");
}

//-----------------------------------------------------------------------------------------------------

void TreeInfixPrint(FILE* fp, const tree_t* tree)
{
    assert(tree);

    NodesInfixPrint(fp, tree->root);
    fprintf(fp, "\n");
}

//-----------------------------------------------------------------------------------------------------

static void NodesPrefixPrint(FILE* fp, const Node* node)
{
    if (!node) { fprintf(fp, " %s ", NIL); return; }

    fprintf(fp, "(" PRINT_NODE, node->data);

    NodesPrefixPrint(fp, node->left);
    NodesPrefixPrint(fp, node->right);

    fprintf(fp, ")");
}

//-----------------------------------------------------------------------------------------------------

static void NodesInfixPrint(FILE* fp, const Node* node)
{
    if (!node) { fprintf(fp, " %s ", NIL); return; }

    fprintf(fp, "(");

    NodesInfixPrint(fp, node->left);
    fprintf(fp, PRINT_NODE, node->data);
    NodesInfixPrint(fp, node->right);

    fprintf(fp, ")");
}

//-----------------------------------------------------------------------------------------------------

static void NodesPostfixPrint(FILE* fp, const Node* node)
{
    if (!node) { fprintf(fp, " %s ", NIL); return; }

    fprintf(fp, "(");

    NodesPostfixPrint(fp, node->left);
    NodesPostfixPrint(fp, node->right);

    fprintf(fp, PRINT_NODE ")", node->data);
}

//-----------------------------------------------------------------------------------------------------

void TreePrefixRead(FILE* fp, tree_t* tree, error_t* error)
{
    assert(tree);

    tree->root = NodesPrefixRead(fp, error);
}

//-----------------------------------------------------------------------------------------------------

static inline void SkipSpaces(FILE* fp)
{
    char ch = 0;
    ch = getc(fp);

    while (isspace(ch))
        ch = getc(fp);

    ungetc(ch, fp);
}

//-----------------------------------------------------------------------------------------------------

static Node* NodesPrefixRead(FILE* fp, error_t* error)
{
    assert(error);

    SkipSpaces(fp);
    char bracket_check = 0;
    bracket_check = getc(fp);
    SkipSpaces(fp);

    if (bracket_check == '(')
    {
        Node*  new_node = ReadNewNode(fp, error);
        return new_node;
    }
    else
    {
        ungetc(bracket_check, fp);

        char read[MAX_LISP_WORD_LEN] = {};
        fscanf(fp, "%s", read);

        ParseClosingBracket(fp, read);

        if (strncmp(read, "nil", MAX_LISP_WORD_LEN))
            error->code = (int) TreeErrors::INVALID_SYNTAX;
    }

    return nullptr;
}

//-----------------------------------------------------------------------------------------------------

static Node* ReadNewNode(FILE* fp, error_t* error)
{
    Node* node = NodeCtor(0, 0, 0, error);

    int data = 0;
    fscanf(fp, "%d", &data);

    node->data = data;
    node->left  = NodesPrefixRead(fp, error);
    node->right = NodesPrefixRead(fp, error);

    SkipSpaces(fp);

    char bracket_check = getc(fp);
    if (bracket_check != ')')
    {
        error->code = (int) TreeErrors::INVALID_SYNTAX;
        return nullptr;
    }

    return node;
}

//-----------------------------------------------------------------------------------------------------

static inline void ParseClosingBracket(FILE* fp, char* read)
{
    assert(read);

    size_t bracket_pos = strlen(NIL);
    if (read[bracket_pos] == ')')
    {
        read[bracket_pos] = '\0';
        ungetc(')', fp);
    }
}

//-----------------------------------------------------------------------------------------------------

int TreeDump(FILE* fp, const void* nodes, const char* func, const char* file, const int line)
{
    assert(nodes);

    LOG_START_DUMP(func, file, line);

    const tree_t* tree = (const tree_t*) nodes;

    TextTreeDump(fp, tree);
    DrawTreeGraph(tree);

    LOG_END();

    return (int) TreeErrors::NONE;
}

//-----------------------------------------------------------------------------------------------------

static void TextTreeDump(FILE* fp, const tree_t* tree)
{
    assert(tree);

    fprintf(fp, "<pre>");

    fprintf(fp, "<b>DUMPING TREE</b>\n");

    TreePrefixPrint(fp, tree);
    TreePostfixPrint(fp, tree);
    TreeInfixPrint(fp, tree);

    fprintf(fp, "</pre>");
}

//-----------------------------------------------------------------------------------------------------

static void DrawTreeGraph(const tree_t* tree)
{
    assert(tree);

    FILE* dotf = fopen(DOT_FILE, "w");

    StartGraph(dotf);
    DrawNodes(dotf, tree->root, 1);
    EndGraph(dotf);

    fclose(dotf);

    MakeImgFromDot(DOT_FILE);
}

//:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::;::::::::::::::::::::::::::

static inline void DrawNodes(FILE* dotf, const Node* node, const int rank)
{
    if (!node) return;

    fprintf(dotf, "%lld [shape=Mrecord, style=filled, fillcolor=\"lightblue\", color = darkblue, rank = %d, label=\" "
                  "{ node: %p | data: " PRINT_NODE " | { left: %p| right: %p } }\"]\n",
                  node, rank, node, node->data, node->left, node->right);

    DrawNodes(dotf, node->left, rank + 1);
    DrawNodes(dotf, node->right, rank + 1);

    if (node->left != nullptr)
        fprintf(dotf, "%lld->%lld\n", node, node->left);

    if (node->right != nullptr)
        fprintf(dotf, "%lld->%lld\n", node, node->right);

}


