#ifndef __GRAPHS_H_
#define __GRAPHS_H_

#include <stdio.h>

static const size_t MAX_DOT_CMD_LEN  = 200;
static const size_t MAX_IMG_FILE_LEN = 100;
static const char*  DOT_FILE         = "tmp.dot";

void StartGraph(FILE* dotf);
void EndGraph(FILE* dotf);

void MakeImgFromDot(const char* dot_file);

#endif
