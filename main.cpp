#include "tree.h"
#include "akinator.h"

static const char* INPUT_FILE = "data.txt";

int main(const int argc, const char* argv[])
{
    OpenLogFile(argv[0]);

    tree_t tree   = {};
    error_t error = {};

    TreeCtor(&tree, &error);

    FILE* fp = fopen(INPUT_FILE, "r");
    if (!fp)
    {
        error.code = (int) ERRORS::OPEN_FILE;
        error.data = INPUT_FILE;
    }
    EXIT_IF_ERROR(&error);

    TreePrefixRead(fp, &tree, &error);
    EXIT_IF_TREE_ERROR(&error);

    GuessMode(&tree, tree.root, INPUT_FILE, &error);
    EXIT_IF_TREE_ERROR(&error);

    DUMP_TREE(&tree);

    TreeDtor(&tree);
}
