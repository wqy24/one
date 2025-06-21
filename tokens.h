#ifndef __ONE_TOKENS_H__
#define __ONE_TOKENS_H__

#include <stdio.h>

typedef struct token {
  enum {
    FEOF,
    STR,
    NUM,
    REF,
    IDN,
    L_BRK,
    R_BRK,
    L_PRN,
    R_PRN,
    PIPE,
    CARET,
    COLON,
    COMMA,
    LT, GT,
    ET /* & */, AT /* @ */,
    QUESTION
  } type;
  char *value;
} one_token;

one_token read_one_token(FILE *input); /* malloc warning: "value" in the token */

#endif
