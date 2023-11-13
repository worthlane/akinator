#include <assert.h>
#include <string.h>

#include "akinator.h"
#include "errors.h"
#include "colorlib.h"
#include "input_and_output.h"

static AkinatorErrors AskQuestion(Node* node, bool* answer, error_t* error);
static AkinatorErrors HandleTreeEndCase(tree_t* tree, Node* node,
                                        const bool answer, const char* data_file, error_t* error);

static AkinatorErrors UpdateAkinatorData(tree_t* tree, Node* node, const char* data_file, error_t* error);

//---------------------------------------------------------------------------------------

AkinatorErrors GuessMode(tree_t* tree, Node* node, const char* data_file, error_t* error)
{
    assert(tree);
    assert(node);
    assert(data_file);
    assert(error);

    bool answer = false;
    AskQuestion(node, &answer, error);
    RETURN_IF_AKINATOR_ERROR((AkinatorErrors) error->code);

    if (node->left == nullptr || node->right == nullptr)
    {
        HandleTreeEndCase(tree, node, answer, data_file, error);
        return (AkinatorErrors) error->code;
    }

    assert(node->left);
    assert(node->right);

    if (answer == true)
        GuessMode(tree, node->left, data_file, error);
    else
        GuessMode(tree, node->right, data_file, error);

    RETURN_IF_AKINATOR_ERROR((AkinatorErrors) error->code);

    return AkinatorErrors::NONE;
}

//---------------------------------------------------------------------------------------

static AkinatorErrors AskQuestion(Node* node, bool* answer, error_t* error)
{
    assert(node);
    assert(answer);
    assert(error);

    while (true)
    {
        printf("Is it %s?\n", node->data);

        char ans[MAX_STRING_LEN] = {};
        scanf("%s", ans);
        ClearInput(stdin);

        if (!strncasecmp(ans, "yes", MAX_STRING_LEN))       { *answer = true;  break; }
        else if (!strncasecmp(ans, "no", MAX_STRING_LEN))   { *answer = false; break; }
        else
        {
            PrintRedText(stdout, "ERROR: INVALID INPUT \"%s\"\n", ans);
            if (!ContinueProgramQuestion(error))
                return AkinatorErrors::USER_QUIT;

        }
    }

    return AkinatorErrors::NONE;
}

//------------------------------------------------------------------------------------------------------------------

int PrintAkinatorError(FILE* fp, const void* err, const char* func, const char* file, const int line)
{
    assert(err);

    LOG_START(func, file, line);

    const struct ErrorInfo* error = (const struct ErrorInfo*) err;

    switch ((AkinatorErrors) error->code)
    {
        case (AkinatorErrors::NONE):
            LOG_END();
            return (int) error->code;

        case (AkinatorErrors::USER_QUIT):
            fprintf(fp, "USER QUITTED PROGRAM<br>\n");
            LOG_END();
            return (int) error->code;

        case (AkinatorErrors::ALLOCATE_MEMORY):
            fprintf(fp, "CAN NOT ALLOCATE MEMORY<br>\n");
            LOG_END();
            return (int) error->code;

        case (AkinatorErrors::UNKNOWN):
        // fall through
        default:
            fprintf(fp, "UNKNOWN AKINATOR ERROR<br>\n");
            LOG_END();
            return (int) AkinatorErrors::UNKNOWN;
    }
}

//---------------------------------------------------------------------------------------

static AkinatorErrors HandleTreeEndCase(tree_t* tree, Node* node,
                                        const bool answer, const char* data_file, error_t* error)
{
    assert(tree);
    assert(data_file);
    assert(node);
    assert(error);

    if (node->left != nullptr || node->right != nullptr)
    {
        error->code = (int) AkinatorErrors::UNEXPECTED_NODE;
        error->data = node;
        return AkinatorErrors::UNEXPECTED_NODE;
    }

    if (answer == true)
    {
        printf("EZ\n");
        return AkinatorErrors::NONE;
    }
    else
    {
        UpdateAkinatorData(tree, node, data_file, error);
        RETURN_IF_AKINATOR_ERROR((AkinatorErrors) error->code);

        return AkinatorErrors::NONE;
    }

    return AkinatorErrors::NONE;
}

//---------------------------------------------------------------------------------------

static AkinatorErrors UpdateAkinatorData(tree_t* tree, Node* node, const char* data_file, error_t* error)
{
    assert(tree);
    assert(data_file);
    assert(node);
    assert(error);

    printf("What did you guess?\n");

    node_data_t guessed_object = GetDataFromLine(stdin, error);
    if (error->code != (int) ERRORS::NONE)
        return AkinatorErrors::INVALID_SYNTAX;

    printf("What's difference between %s and %s?\n", guessed_object, node->data);

    node_data_t difference = GetDataFromLine(stdin, error);
    if (error->code != (int) ERRORS::NONE)
        return AkinatorErrors::INVALID_SYNTAX;

    Node* positive_ans_node = NodeCtor(guessed_object, 0, 0, error);
    if (error->code != (int) TreeErrors::NONE)  { return AkinatorErrors::TREE_ERROR; }

    Node* negative_ans_node = NodeCtor(node->data, 0, 0, error);
    if (error->code != (int) TreeErrors::NONE)  { return AkinatorErrors::TREE_ERROR; }

    node->data  = difference;
    node->right = negative_ans_node;
    node->left  = positive_ans_node;

    if (AskUserQuestion("Do you want to save edits in data base?"))
    {
        FILE* fp = fopen(data_file, "w");
        if (!fp)
        {
            error->code = (int) AkinatorErrors::DATA_FILE;
            error->data = data_file;
            return AkinatorErrors::DATA_FILE;
        }

        TreePrefixPrint(fp, tree);

        printf("DATA SUCCESFULLY UPDATED\n");
    }

    return AkinatorErrors::NONE;
}
