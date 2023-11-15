#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "tree.h"
#include "graphs.h"
#include "input_and_output.h"

static void DestructNodes(Node* root);

static void NodesPrefixPrint(FILE* fp, const Node* node);
static void NodesPostfixPrint(FILE* fp, const Node* node);
static void NodesInfixPrint(FILE* fp, const Node* node);

static Node* NodesPrefixRead(FILE* fp, error_t* error);

static inline void DeleteClosingBracketFromWord(FILE* fp, char* read);
static TreeErrors CheckQuotatationMark(FILE* fp, error_t* error);
static TreeErrors ReadTextInQuotes(FILE* fp, char* data, error_t* error);

static Node* ReadNewNode(FILE* fp, error_t* error);

static void TextTreeDump(FILE* fp, const tree_t* tree);
static TreeErrors VerifyNodes(const Node* node, error_t* error);

// ======== GRAPHS =========

static void DrawTreeGraph(const tree_t* tree);

static inline void DrawNodes(FILE* dotf, const Node* node, const int rank);

// =========================

static const char* NIL = "nil";

//-----------------------------------------------------------------------------------------------------

Node* NodeCtor(const node_data_t data, Node* left, Node* right, error_t* error)
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

    tree->root = root;

    return TreeErrors::NONE;
}

//-----------------------------------------------------------------------------------------------------

void TreeDtor(tree_t* tree)
{
    DestructNodes(tree->root);

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

    fprintf(fp, "(" PRINT_NODE "\n", node->data);

    NodesPrefixPrint(fp, node->left);
    NodesPrefixPrint(fp, node->right);

    fprintf(fp, ")\n");
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

    SkipSpaces(fp);
    int ch = getc(fp);

    Node* root = nullptr;

    if (ch == EOF)
        root = NodeCtor("something unknown", 0, 0, error);
    else
    {
        ungetc(ch, fp);
        root = NodesPrefixRead(fp, error);
    }

    tree->root = root;
}

//-----------------------------------------------------------------------------------------------------

static Node* NodesPrefixRead(FILE* fp, error_t* error)
{
    assert(error);

    SkipSpaces(fp);
    char opening_bracket_check = 0;
    opening_bracket_check = getc(fp);
    SkipSpaces(fp);

    if (opening_bracket_check == '(')
    {
        Node* new_node = ReadNewNode(fp, error);

        SkipSpaces(fp);

        char closing_bracket_check = getc(fp);
        if (closing_bracket_check != ')')
        {
            error->code = (int) TreeErrors::INVALID_SYNTAX;
            return nullptr;
        }

        return new_node;
    }
    else
    {
        ungetc(opening_bracket_check, fp);

        char read[MAX_STRING_LEN] = {};
        fscanf(fp, "%s", read);

        DeleteClosingBracketFromWord(fp, read);

        if (strncmp(read, "nil", MAX_STRING_LEN))
            error->code = (int) TreeErrors::INVALID_SYNTAX;
    }

    return nullptr;
}

//-----------------------------------------------------------------------------------------------------

static Node* ReadNewNode(FILE* fp, error_t* error)
{
    Node* node = NodeCtor(0, 0, 0, error);

    node_data_t data = ReadNodeData(fp, error);
    if (error->code != (int) TreeErrors::NONE)
        return nullptr;

    node->data  = data;
    node->left  = NodesPrefixRead(fp, error);
    node->right = NodesPrefixRead(fp, error);

    return node;
}

//-----------------------------------------------------------------------------------------------------

node_data_t ReadNodeData(FILE* fp, error_t* error)
{
    assert(error);

    node_data_t data = (node_data_t) calloc(MAX_STRING_LEN, sizeof(char));
    if (data == nullptr)
    {
        error->code = (int) TreeErrors::ALLOCATE_MEMORY;
        return nullptr;
    }

    ReadTextInQuotes(fp, data, error);
    if (error->code != (int) TreeErrors::NONE)
        return nullptr;

    return data;
}

//-----------------------------------------------------------------------------------------------------

TreeErrors NodeVerify(const Node* node, error_t* error)
{
    assert(node);
    assert(error);

    if (node == node->left || node == node->right)
    {
        error->code = (int) TreeErrors::CYCLED_NODE;
        return TreeErrors::CYCLED_NODE;
    }
    if (node->left == node->right)
    {
        error->code = (int) TreeErrors::COMMON_HEIR;
        return TreeErrors::COMMON_HEIR;
    }

    return TreeErrors::NONE;
}

//-----------------------------------------------------------------------------------------------------

static TreeErrors ReadTextInQuotes(FILE* fp, char* data, error_t* error)
{
    assert(data);
    assert(error);

    CheckQuotatationMark(fp, error);
    RETURN_IF_TREE_ERROR((TreeErrors) error->code);

    bool have_closing_quote_mark = false;

    for (size_t i = 0; i < MAX_STRING_LEN; i++)
    {
        char ch = getc(fp);

        if (ch == EOF)
            break;

        if (ch == '"')
        {
            data[i] = 0;
            have_closing_quote_mark = true;
            break;
        }
        else
            data[i] = ch;
    }

    if (!have_closing_quote_mark)
    {
        error->code = (int) TreeErrors::INVALID_SYNTAX;
        return TreeErrors::INVALID_SYNTAX;
    }

    return TreeErrors::NONE;
}

//-----------------------------------------------------------------------------------------------------

static TreeErrors CheckQuotatationMark(FILE* fp, error_t* error)
{
    assert(error);

    char mark = getc(fp);

    if (mark != '"')
    {
        error->code = (int) TreeErrors::INVALID_SYNTAX;
        return TreeErrors::INVALID_SYNTAX;
    }

    return TreeErrors::NONE;
}

//-----------------------------------------------------------------------------------------------------

static inline void DeleteClosingBracketFromWord(FILE* fp, char* read)
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

int NodeDump(FILE* fp, const void* dumping_node, const char* func, const char* file, const int line)
{
    assert(dumping_node);

    LOG_START_DUMP(func, file, line);

    const Node* node = (const Node*) dumping_node;

    fprintf(fp, "NODE [%p]<br>\n"
                "DATA > %s<br>\n"
                "LEFT > [%p]<br>\n"
                "RIGHT > [%p]<br>\n", node, node->data, node->left, node->right);

    LOG_END();

    return (int) TreeErrors::NONE;
}

//-----------------------------------------------------------------------------------------------------

TreeErrors TreeVerify(const tree_t* tree, error_t* error)
{
    assert(tree);
    assert(error);

    VerifyNodes(tree->root, error);

    return (TreeErrors) error->code;
}

//-----------------------------------------------------------------------------------------------------

static TreeErrors VerifyNodes(const Node* node, error_t* error)
{
    assert(node);
    assert(error);

    if (node->left != nullptr)
    {
        NodeVerify(node->left, error);
        RETURN_IF_TREE_ERROR((TreeErrors) error->code);
    }

    if (node->right != nullptr)
    {
        NodeVerify(node->right, error);
        RETURN_IF_TREE_ERROR((TreeErrors) error->code);
    }

    NodeVerify(node, error);

    return (TreeErrors) error->code;
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
                  "{ node: %p | data: %s | { left: %p| right: %p } }\"]\n",
                  node, rank, node, node->data, node->left, node->right);

    DrawNodes(dotf, node->left, rank + 1);
    DrawNodes(dotf, node->right, rank + 1);

    if (node->left != nullptr)
        fprintf(dotf, "%lld->%lld [fontcolor = black, label = \"yes\"]\n", node, node->left);

    if (node->right != nullptr)
        fprintf(dotf, "%lld->%lld [fontcolor = black, label = \"no\"]\n", node, node->right);

}



