#include "tree.h"
#include "akinator.h"
#include "input_and_output.h"
#include "colorlib.h"

static const char* INPUT_FILE = "data.txt";

int main(const int argc, const char* argv[])
{
    OpenLogFile(argv[0]);

    tree_t tree   = {};
    error_t error = {};
    TreeCtor(&tree, &error);

    const char* data_file = GetInputFileName(argc, argv, &error);
    EXIT_IF_ERROR(&error);

    bool leave_flag = false;

    while (!leave_flag)
    {
        FILE* fp = OpenInputFile(data_file, &error);
        EXIT_IF_ERROR(&error);

        TreePrefixRead(fp, &tree, &error);
        EXIT_IF_TREE_ERROR(&error);

        fclose(fp);

        PrintMenu();
        AkinatorMode mode = GetWorkingMode();

        switch (mode)
        {
            case AkinatorMode::COMPARE:
            {
                CompareMode(&tree, &error);
                EXIT_IF_AKINATOR_ERROR(&error);
                break;
            }

            case AkinatorMode::PRINT_TREE:
            {
                DUMP_TREE(&tree);
                TreePrefixPrint(stdout, &tree);
                break;
            }

            case AkinatorMode::DESCRIBE:
            {
                DescriptionMode(&tree, &error);
                EXIT_IF_AKINATOR_ERROR(&error);
                break;
            }

            case AkinatorMode::GUESS:
            {
                GuessMode(&tree, tree.root, INPUT_FILE, &error);
                EXIT_IF_AKINATOR_ERROR(&error);
                break;
            }

            case AkinatorMode::QUIT:
            // fall through
            default:
            {
                leave_flag = true;
                break;
            }
        }
    }

    PrintRedText(stdout, "Quitting program\n", nullptr);

    TreeDtor(&tree);
}

