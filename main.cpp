#include "tree.h"
#include "akinator.h"
#include "input_and_output.h"

static const char* INPUT_FILE = "data.txt";

int main(const int argc, const char* argv[])
{
    OpenLogFile(argv[0]);

    tree_t tree   = {};
    error_t error = {};
    TreeCtor(&tree, &error);

    bool leave_flag = false;

    while (!leave_flag)
    {
        PrintMenu();
        AkinatorMode mode = GetWorkingMode();

        FILE* fp = fopen(INPUT_FILE, "r");
        if (!fp)
        {
            error.code = (int) ERRORS::OPEN_FILE;
            error.data = INPUT_FILE;
        }
        EXIT_IF_ERROR(&error);

        TreePrefixRead(fp, &tree, &error);
        EXIT_IF_TREE_ERROR(&error);

        fclose(fp);

        switch (mode)
        {
            case AkinatorMode::COMPARE:

                CompareMode(&tree, &error);
                EXIT_IF_AKINATOR_ERROR(&error);
                break;

            case AkinatorMode::PRINT_TREE:

                DUMP_TREE(&tree);
                TreePrefixPrint(stdout, &tree);
                break;

            case AkinatorMode::DESCRIBE:

                DescriptionMode(&tree, &error);
                EXIT_IF_AKINATOR_ERROR(&error);
                break;

            case AkinatorMode::GUESS:

                GuessMode(&tree, tree.root, INPUT_FILE, &error);
                EXIT_IF_AKINATOR_ERROR(&error);
                break;

            case AkinatorMode::QUIT:
            // fall through
            default:
                leave_flag = true;
                break;
        }
    }

    printf("Quitting program\n");

    TreeDtor(&tree);
}
