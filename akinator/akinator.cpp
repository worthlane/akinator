#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "akinator.h"
#include "common/errors.h"
#include "common/colorlib.h"
#include "common/input_and_output.h"
#include "stack/stack.h"

static AkinatorErrors AskUserAboutNode(Node* node, bool* answer, error_t* error);
static AkinatorErrors GuessingLastNodeCase(tree_t* tree, Node* node,
                                            const bool answer, const char* data_file, error_t* error);
static AkinatorErrors AddNewNode(Node* node, const node_data_t guessed_object,
                                             const node_data_t difference, error_t* error);
static AkinatorErrors UpdateAkinatorData(tree_t* tree, Node* node, const char* data_file, error_t* error);
static AkinatorErrors SaveNewTreeInData(const tree_t* tree, const char* data_file, error_t* error);


static char*          GetObjectInTree(const tree_t* tree, Stack_t* stk, error_t* error);
static AkinatorErrors FindObjectInTree(Stack_t* stk, Node* node,
                                const char* object, bool* found_flag, error_t* error);
static AkinatorErrors CompareObjectWithLastNode(Node* node, const char* object,
                                                bool* found_flag, error_t* error);
static AkinatorErrors PrintObjectPropertiesBasedOnStack(const Stack_t* stk, const int start_stk_index,
                                                        Node* node, error_t* error);


static AkinatorErrors CompareObjectsDescription(const Stack_t* stk_1, const Stack_t* stk_2,
                                                const char* object_1, const char* object_2,
                                                Node* node, error_t* error);
static AkinatorErrors WriteSimilarProperties(const Stack_t* stk_1, const Stack_t* stk_2,
                                             int* stk_index, Node** curr_node, error_t* error);


//---------------------------------------------------------------------------------------

AkinatorErrors GuessMode(tree_t* tree, Node* node, const char* data_file, error_t* error)
{
    assert(tree);
    assert(node);
    assert(data_file);
    assert(error);

    bool answer = false;
    AskUserAboutNode(node, &answer, error);
    RETURN_IF_AKINATOR_ERROR((AkinatorErrors) error->code);

    if (node->left == nullptr || node->right == nullptr)
    {
        GuessingLastNodeCase(tree, node, answer, data_file, error);
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

static AkinatorErrors AskUserAboutNode(Node* node, bool* answer, error_t* error)
{
    assert(node);
    assert(answer);
    assert(error);

    while (true)
    {
        SayPhrase("Is it %s?\n", node->data);

        char ans[MAX_STRING_LEN] = {};
        scanf("%s", ans);
        ClearInput(stdin);

        if (!strncasecmp(ans, "yes", MAX_STRING_LEN))       { *answer = true;  break; }
        else if (!strncasecmp(ans, "no", MAX_STRING_LEN))   { *answer = false; break; }
        else
        {
            PrintRedText(stdout, "Unknown answer \"%s\"\n", ans);
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

        case (AkinatorErrors::DATA_FILE):
            fprintf(fp, "CAN NOT WORK WITH FILE \"%s\"<br>\n", (const char*) error->data);
            LOG_END();
            return (int) error->code;

        case (AkinatorErrors::UNEXPECTED_NODE):
            fprintf(fp, "UNEXPECTED NODE<br>\n");
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

static AkinatorErrors GuessingLastNodeCase(tree_t* tree, Node* node,
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
        SayPhrase("EZ\n");
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

    SayPhrase("What did you guess?\n");

    node_data_t guessed_object = GetDataFromLine(stdin, error);
    if (error->code != (int) ERRORS::NONE)
        return AkinatorErrors::INVALID_SYNTAX;

    SayPhrase("What is difference between %s and %s?\n", guessed_object, node->data);

    node_data_t difference = GetDataFromLine(stdin, error);
    if (error->code != (int) ERRORS::NONE)
        return AkinatorErrors::INVALID_SYNTAX;

    AddNewNode(node, guessed_object, difference, error);
    RETURN_IF_AKINATOR_ERROR((AkinatorErrors) error->code);

    SaveNewTreeInData(tree, data_file, error);
    RETURN_IF_AKINATOR_ERROR((AkinatorErrors) error->code);

    return AkinatorErrors::NONE;
}

//---------------------------------------------------------------------------------------

static AkinatorErrors AddNewNode(Node* node, const node_data_t guessed_object,
                                             const node_data_t difference, error_t* error)
{
    assert(node);
    assert(guessed_object);
    assert(difference);

    Node* positive_ans_node = NodeCtor(guessed_object, 0, 0, error);
    if (error->code != (int) TreeErrors::NONE)  { return AkinatorErrors::TREE_ERROR; }

    Node* negative_ans_node = NodeCtor(node->data, 0, 0, error);
    if (error->code != (int) TreeErrors::NONE)  { return AkinatorErrors::TREE_ERROR; }

    node->data  = difference;
    node->right = negative_ans_node;
    node->left  = positive_ans_node;

    return AkinatorErrors::NONE;
}

//---------------------------------------------------------------------------------------

static AkinatorErrors SaveNewTreeInData(const tree_t* tree, const char* data_file, error_t* error)
{
    assert(tree);
    assert(data_file);
    assert(error);

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

        PrintGreenText(stdout, "DATA SUCCESFULLY UPDATED\n", nullptr);
        fclose(fp);
    }

    return AkinatorErrors::NONE;
}

//---------------------------------------------------------------------------------------

AkinatorErrors DescriptionMode(tree_t* tree, error_t* error)
{
    assert(tree);
    assert(error);

    Stack_t stk = {};
    StackCtor(&stk);

    SayPhrase("What do you want to describe?\n", nullptr);

    char* object = GetObjectInTree(tree, &stk, error);
    RETURN_IF_AKINATOR_ERROR((AkinatorErrors) error->code);

    if (object != nullptr)
    {
        SayPhrase("%s - ", object);
        PrintObjectPropertiesBasedOnStack(&stk, 0, tree->root, error);
        RETURN_IF_AKINATOR_ERROR((AkinatorErrors) error->code);
    }

    StackDtor(&stk);
    return AkinatorErrors::NONE;
}

//---------------------------------------------------------------------------------------

static AkinatorErrors FindObjectInTree(Stack_t* stk, Node* node,
                                        const char* object, bool* found_flag, error_t* error)
{
    assert(stk);
    assert(node);
    assert(object);
    assert(error);
    assert(found_flag);

    if (node->left == nullptr || node->right == nullptr)
    {
        CompareObjectWithLastNode(node, object, found_flag, error);
        return (AkinatorErrors) error->code;
    }

    if (*found_flag == false)
    {
        StackPush(stk, LEFT_STEP);
        FindObjectInTree(stk, node->left, object, found_flag, error);
        if (*found_flag == false)
            StackPop(stk);
    }

    if (*found_flag == false)
    {
        StackPush(stk, RIGHT_STEP);
        FindObjectInTree(stk, node->right, object, found_flag, error);
        if (*found_flag == false)
            StackPop(stk);
    }

    return AkinatorErrors::NONE;
}

//---------------------------------------------------------------------------------------

static AkinatorErrors CompareObjectWithLastNode(Node* node, const char* object,
                                                bool* found_flag, error_t* error)
{
    if (node->left != nullptr || node->right != nullptr)
    {
        error->code = (int) AkinatorErrors::UNEXPECTED_NODE;
        error->data = node;
        return AkinatorErrors::UNEXPECTED_NODE;
    }

    if (!strncasecmp(object, node->data, MAX_STRING_LEN))
        *found_flag = true;

    return AkinatorErrors::NONE;
}

//---------------------------------------------------------------------------------------

static AkinatorErrors PrintObjectPropertiesBasedOnStack(const Stack_t* stk, const int start_stk_index,
                                                        Node* node, error_t* error)
{
    assert(node);
    assert(stk);

    Node* current_node = node;

    for (size_t i = start_stk_index; i < stk->size; i++)
    {
        elem_t step = stk->data[i];

        if (current_node == nullptr && (step == RIGHT_STEP || step == LEFT_STEP))
        {
            error->code = (int) AkinatorErrors::UNEXPECTED_NODE;
            return AkinatorErrors::UNEXPECTED_NODE;
        }

        if (step == RIGHT_STEP)
        {
            SayPhrase("not %s, ", current_node->data);
            current_node = current_node->right;
        }
        else if (step == LEFT_STEP)
        {
            SayPhrase("%s, ", current_node->data);
            current_node = current_node->left;
        }
        else
        {
            error->code = (int) AkinatorErrors::INVALID_STACK;
            return AkinatorErrors::INVALID_STACK;
        }
    }
    putchar('\n');

    return AkinatorErrors::NONE;
}

//---------------------------------------------------------------------------------------

static char* GetObjectInTree(const tree_t* tree, Stack_t* stk, error_t* error)
{
    assert(error);
    assert(stk);
    assert(tree);

    char* object = GetDataFromLine(stdin, error);
    if (error->code != (int) ERRORS::NONE)
    {
        error->code = (int) AkinatorErrors::INVALID_SYNTAX;
        return nullptr;
    }

    bool found_flag_1 = false;
    FindObjectInTree(stk, tree->root, object, &found_flag_1, error);

    if (found_flag_1 == false)
    {
        PrintRedText(stdout, "Can't find \"%s\" in tree\n", object);
        free(object);
        return nullptr;
    }

    return object;
}

//---------------------------------------------------------------------------------------

AkinatorErrors CompareMode(tree_t* tree, error_t* error)
{
    assert(tree);
    assert(error);

    Stack_t stk_1 = {};
    Stack_t stk_2 = {};
    StackCtor(&stk_1);
    StackCtor(&stk_2);

    SayPhrase("Input first object\n");

    char* object_1 = GetObjectInTree(tree, &stk_1, error);
    RETURN_IF_AKINATOR_ERROR((AkinatorErrors) error->code);

    SayPhrase("Input second object\n");

    char* object_2 = GetObjectInTree(tree, &stk_2, error);
    RETURN_IF_AKINATOR_ERROR((AkinatorErrors) error->code);

    if (object_1 != nullptr && object_2 != nullptr)
    {
        CompareObjectsDescription(&stk_1, &stk_2, object_1, object_2, tree->root, error);
        RETURN_IF_AKINATOR_ERROR((AkinatorErrors) error->code);
    }

    StackDtor(&stk_1);
    StackDtor(&stk_2);
    return AkinatorErrors::NONE;
}

//---------------------------------------------------------------------------------------

static AkinatorErrors CompareObjectsDescription(const Stack_t* stk_1, const Stack_t* stk_2,
                                                const char* object_1, const char* object_2,
                                                Node* node, error_t* error)
{
    assert(stk_1);
    assert(stk_2);
    assert(object_1);
    assert(object_2);
    assert(node);
    assert(error);

    Node* curr_node = node;
    int   stk_index = 0;

    if (stk_1->data[stk_index] == stk_2->data[stk_index])
    {
        SayPhrase("%s and %s are similar in that they both are: ", object_1, object_2);

        WriteSimilarProperties(stk_1, stk_2, &stk_index, &curr_node, error);
        RETURN_IF_AKINATOR_ERROR((AkinatorErrors) error->code);

        SayPhrase("But ");
    }

    SayPhrase("%s is: ", object_1);

    PrintObjectPropertiesBasedOnStack(stk_1, stk_index, curr_node, error);
    RETURN_IF_AKINATOR_ERROR((AkinatorErrors) error->code);

    SayPhrase("And %s is: ", object_2);

    PrintObjectPropertiesBasedOnStack(stk_2, stk_index, curr_node, error);
    RETURN_IF_AKINATOR_ERROR((AkinatorErrors) error->code);

    return AkinatorErrors::NONE;
}

//---------------------------------------------------------------------------------------

static AkinatorErrors WriteSimilarProperties(const Stack_t* stk_1, const Stack_t* stk_2,
                                             int* stk_index, Node** curr_node, error_t* error)
{
    assert(stk_1);
    assert(stk_2);
    assert(stk_index);
    assert(curr_node);
    assert(error);

    while (stk_1->data[*stk_index] == stk_2->data[*stk_index])
    {
        elem_t step = stk_1->data[(*stk_index)++];

        if (*curr_node == nullptr && (step == RIGHT_STEP || step == LEFT_STEP))
        {
            error->code = (int) AkinatorErrors::UNEXPECTED_NODE;
            return AkinatorErrors::UNEXPECTED_NODE;
        }

        if (step == RIGHT_STEP)
        {
            SayPhrase("not %s, ", (*curr_node)->data);
            *curr_node = (*curr_node)->right;
        }
        else if (step == LEFT_STEP)
        {
            SayPhrase("%s, ", (*curr_node)->data);
            (*curr_node) = (*curr_node)->left;
        }
        else
        {
            error->code = (int) AkinatorErrors::INVALID_STACK;
            return AkinatorErrors::INVALID_STACK;
        }
    }
    putchar('\n');

    return AkinatorErrors::NONE;
}

//---------------------------------------------------------------------------------------

AkinatorMode GetWorkingMode()
{
    PrintMenu();

    int mode = 0;

    scanf("%c", &mode);
    mode = toupper(mode);

    bool have_other_symb = DoesLineHaveOtherSymbols(stdin);

    if (have_other_symb == true)
        return AkinatorMode::QUIT;

    switch ((AkinatorMode) mode)
    {
        case AkinatorMode::COMPARE:     return AkinatorMode::COMPARE;
        case AkinatorMode::PRINT_TREE:  return AkinatorMode::PRINT_TREE;
        case AkinatorMode::DESCRIBE:    return AkinatorMode::DESCRIBE;
        case AkinatorMode::GUESS:       return AkinatorMode::GUESS;
        case AkinatorMode::QUIT:
        // fall through
        default:                        return AkinatorMode::QUIT;
    }

}


