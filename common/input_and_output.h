#ifndef __INPUT_AND_OUTPUT_H_
#define __INPUT_AND_OUTPUT_H_

#include <stdio.h>

#include "errors.h"

static const size_t MAX_STRING_LEN  = 100;
static const size_t MAX_COMMAND_LEN = 200;

void SkipSpaces(FILE* fp);
void ClearInput(FILE* fp);

bool ContinueProgramQuestion(error_t* error);
bool AskUserQuestion(const char* question);

char* GetDataFromLine(FILE* fp, error_t* error);
bool DoesLineHaveOtherSymbols(FILE* fp);

const char* GetInputFileName(const int argc, const char* argv[], error_t* error);
FILE* OpenInputFile(const char* file_name, error_t* error);

int SayPhrase(const char *format, ...);

void PrintMenu();

#endif
