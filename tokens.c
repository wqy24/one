#include "tokens.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 32

int pred_ref(char c)
{
  return c != '}';
}

int pred_str(char c)
{
  return c != '`';
}

char *read_value(char first, FILE *input, int (*pred)(char), int rm_quote)
{
  char buffer[BUFFER_SIZE], curr;
  int count, plus1 = rm_quote ? 0 : 1;
  char *res = (char *)malloc(plus1 * sizeof(char));

  for (count = 1; !feof(input) && pred(curr = getc(input)); count++) {
    buffer[count - 1] = curr;
    if (count % BUFFER_SIZE == BUFFER_SIZE - 1) {
      res = realloc(res, (plus1 + count) * sizeof(char));
      /* `buffer` is actually full now. Glitch in strncpy is used here. Please ignore the warning of string overflow here. */
      strncpy(res + (plus1 + count - 1 - BUFFER_SIZE), buffer, BUFFER_SIZE);
    }
  }

  if (rm_quote && pred(curr)) {
    fputs("EOF while parsing a token.\n", stderr);
    exit(EXIT_FAILURE);
  }

  res = realloc(res, (plus1 * 2 + count) * sizeof(char));
  strncpy(res + (plus1 + count - 1 - strlen(buffer)), buffer, BUFFER_SIZE);

  if (!rm_quote) {
    res[0] = first;
    if (!pred(curr)) {
      fseek(input, -1, SEEK_CUR);
    }
  }

  return res;
}

one_token read_one_token(FILE *input)
{
  if (feof(input)) return (one_token){FEOF, NULL};

  switch (getc(input)) {
    case '@': return (one_token){AT, NULL};
    case '^': return (one_token){CARET, NULL};
    case '&': return (one_token){ET, NULL};
    case '(': return (one_token){L_PRN, NULL};
    case ')': return (one_token){R_PRN, NULL};
    case '[': return (one_token){L_BRK, NULL};
    case ']': return (one_token){R_BRK, NULL};
    case '|': return (one_token){PIPE, NULL};
    case ':': return (one_token){COLON, NULL};
    case '<': return (one_token){LT, NULL};
    case '>': return (one_token){GT, NULL};
    case '?': return (one_token){QUESTION, NULL};
    case ';':
      while(!feof(input) && getc(input) != '\n') /* do nothing */;
      return read_one_token(input);
    case ',': return (one_token){COMMA, NULL};
    case '{': return (one_token){REF, read_value('{', input, pred_ref, 1)};
    case '`': return (one_token){STR, read_value('`', input, pred_str, 1)};
    default: return (one_token){FEOF, NULL};
  }
}
