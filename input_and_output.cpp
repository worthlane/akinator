#include <ctype.h>

#include "input_and_output.h"

static void ReadLine(FILE* fp, char* buf);

//-----------------------------------------------------------------------------------------------------

void SkipSpaces(FILE* fp)
{
    char ch = 0;
    ch = getc(fp);

    while (isspace(ch))
        ch = getc(fp);

    ungetc(ch, fp);
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

    printf("Do you want to continue? (1 - Yes): ");

    int ans = false;
    scanf("%d", &ans);
    ClearInput(stdin);

    if (ans != 1)
    {
        printf("Bye Bye");
        error->code = (int) ERRORS::USER_QUIT;
    }

    return (ans == 1) ? true : false;
}

//------------------------------------------------------------------------------------------------------------------

bool AskUserQuestion(const char* question)
{
    assert(question);

    printf("%s (1 - Yes): ", question);

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
