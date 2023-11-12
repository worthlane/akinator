#include <stdlib.h>
#include <time.h>

#include "graphs.h"
#include "logs.h"

static size_t      IMG_CNT        = 1;
static const char* IMG_FOLDER_DIR = "img/";

//---------------------------------------------------------------------------------------

void EndGraph(FILE* dotf)
{
    fprintf(dotf, "}");
}

//---------------------------------------------------------------------------------------

void StartGraph(FILE* dotf)
{
    fprintf(dotf, "digraph structs {\n"
	              "rankdir=TB;\n"
	              "node[color=\"black\",fontsize=14];\n"
                  "nodesep = 0.1;\n"
	              "edge[color=\"darkblue\",fontcolor=\"yellow\",fontsize=12];\n");
}

//---------------------------------------------------------------------------------------

void MakeImgFromDot(const char* dot_file)
{
    char img_name[MAX_IMG_FILE_LEN] = {};
    snprintf(img_name, MAX_IMG_FILE_LEN, "%simg%lu_%lu.png", IMG_FOLDER_DIR, IMG_CNT++, clock());

    char dot_command[MAX_DOT_CMD_LEN] = {};
    snprintf(dot_command, MAX_DOT_CMD_LEN, "dot %s -T png -o %s", dot_file, img_name);
    system(dot_command);

    PrintLog("<img src=\"%s\"><br>", img_name);
}
