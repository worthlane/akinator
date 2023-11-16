#include <ctype.h>
#include <assert.h>
#include <stdarg.h>

#include "input_and_output.h"
#include "fast_input_and_output.h"
#include "colorlib.h"

static void ReadLine(FILE* fp, char* buf);

//-----------------------------------------------------------------------------------------------------

void SkipSpaces(Storage* info)
{
    int ch = 0;
    ch = Bufgetc(info);

    while (isspace(ch))
        ch = Bufgetc(info);

    Bufungetc(info);
}

//-----------------------------------------------------------------------------------------------------

int Bufgetc(Storage* info)
{
    int ch = info->buf[(info->ptr)++];

    return ch;
}

//-----------------------------------------------------------------------------------------------------

void Bufungetc(Storage* info)
{
    info->ptr--;
}

//------------------------------------------------------------------------------------------------------------------

void ClearInput(FILE* fp)
{
    int ch = 0;
    while ((ch = fgetc(fp)) != '\n' && ch != EOF) {}
}

//------------------------------------------------------------------------------------------------------------------

bool ContinueProgramQuestion(error_t* error)
{
    assert(error);

    SayPhrase("Do you want to continue? (1 - Yes): ");

    int ans = false;
    scanf("%d", &ans);
    ClearInput(stdin);

    if (ans != 1)
    {
        PrintRedText(stdout, "Bye Bye", nullptr);
        error->code = (int) ERRORS::USER_QUIT;
    }

    return (ans == 1) ? true : false;
}

//------------------------------------------------------------------------------------------------------------------

bool AskUserQuestion(const char* question)
{
    assert(question);

    PrintGreenText(stdout, "%s (1 - Yes): ", question);

    int ans = false;
    scanf("%d", &ans);
    ClearInput(stdin);

    return (ans == 1) ? true : false;
}

//-----------------------------------------------------------------------------------------------------

char* GetDataFromLine(FILE* fp, error_t* error)
{
    assert(error);

    char* line = (char*) calloc(MAX_STRING_LEN, sizeof(char));
    if (line == nullptr)
    {
        error->code = (int) ERRORS::ALLOCATE_MEMORY;
        return nullptr;
    }

    ReadLine(fp, line);

    return line;
}

//-----------------------------------------------------------------------------------------------------

static void ReadLine(FILE* fp, char* buf)
{
    assert(buf);

    for (size_t i = 0; i < MAX_STRING_LEN; i++)
    {
        char ch = getc(fp);

        if (ch == EOF)
            break;

        if (ch == '\n' || ch == '\0')
        {
            buf[i] = 0;
            break;
        }
        else
            buf[i] = ch;
    }
}

//-----------------------------------------------------------------------------------------------------

void PrintMenu()
{
    PrintCyanText(stdout, "CHOOSE PROGRAM MODE:\n"
                          "[G]UESS              [C]OMPARE\n"
                          "[D]ESCRIBE           [P]RINT TREE\n"
                          "                     [Q]UIT\n", nullptr);
}

//-----------------------------------------------------------------------------------------------------

bool DoesLineHaveOtherSymbols(FILE* fp)
{
    int ch = 0;

    while ((ch = getc(fp)) != '\n' && ch != EOF)
    {
        if (!isspace(ch))
            {
                ClearInput(fp);
                return true;
            }
    }

    return false;
}

//-----------------------------------------------------------------------------------------------------

FILE* OpenInputFile(const char* file_name, error_t* error)
{
    assert(file_name);
    assert(error);

    FILE* fp = fopen(file_name, "r");
    if (!fp)
    {
        error->code = (int) ERRORS::OPEN_FILE;
        error->data = file_name;
    }

    return fp;
}

//-----------------------------------------------------------------------------------------------------

const char* GetInputFileName(const int argc, const char* argv[], error_t* error)
{
    assert(error);
    assert(argv);

    const char* file_name = nullptr;

    if (argc > 1)
        file_name = argv[1];
    else
    {
        PrintGreenText(stdout, "Enter input file name: \n", nullptr);
        file_name = GetDataFromLine(stdin, error);
    }

    if (file_name != nullptr)
        PrintGreenText(stdout, "INPUT FILE NAME: \"%s\"\n", file_name);

    return file_name;
}

//-----------------------------------------------------------------------------------------------------

int SayPhrase(const char *format, ...)
{
    va_list arg;
    int done;

    char buf[MAX_STRING_LEN] = {};

    va_start (arg, format);
    done = vsnprintf(buf, MAX_STRING_LEN, format, arg);
    va_end (arg);

    PrintCyanText(stdout, "%s", buf);

    char cmd[MAX_COMMAND_LEN] = {};
    snprintf(cmd, MAX_COMMAND_LEN, "say %s", buf);
    system(cmd);

    return done;
}

