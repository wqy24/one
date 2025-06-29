#include "tokens.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define EXPAND_RATE 32

int pred_ref(char c) { return c != '}'; }
int pred_str(char c) { return c != '`'; }
int pred_num(char c) { return (isdigit(c) || c == '.'); }
int pred_idn(char c) { return (isalnum(c) || c == '_' || c == '-'); }

char *read_value(char first, FILE *input, int (*pred)(char), int rm_quote)
{
  size_t size = EXPAND_RATE * sizeof(char);
  char *buffer = (char *)malloc(size), *res;
  char c;
  size_t count = 0;
  if (!rm_quote) buffer[count++] = first;

  while ((c = getc(input)) != EOF && pred(c)) {
    buffer[count++] = c;
    if (count + 1 >= size) {
      size += EXPAND_RATE * sizeof(char);
      char *tmp = realloc(buffer, size);
      if (tmp == NULL) {
        fputs("Out of memory\n", stderr);
        free(buffer);
        return NULL;
      }
      buffer = tmp;
    }
  }

  if (rm_quote && pred(c)) {
    fputs("EOF while parsing a quoted token\n", stderr);
    free(buffer);
    return NULL;
  }

  buffer[count] = '\0';
  res = realloc(buffer, (count + 1) * sizeof(char));
  if (res == NULL) {
    fputs("Out of memory\n", stderr);
    free(buffer);
    return NULL;
  }

  if (!rm_quote && !pred(c)) {
    fseek(input, -1, SEEK_CUR);
  }

  return res;
}

one_token read_one_token(FILE *input)
{
  char curr;

  switch (curr = getc(input)) {
    case EOF: return (one_token){FEOF, NULL};
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
    case '~': return (one_token){WAVE, NULL};
    case ';':
      while(!feof(input) && getc(input) != '\n') /* do nothing */;
      return read_one_token(input);
    case ',': return (one_token){COMMA, NULL};
    case '{': return (one_token){REF, read_value('{', input, pred_ref, 1)};
    case '`': return (one_token){STR, read_value('`', input, pred_str, 1)};
    default:
      if (isspace(curr)) return read_one_token(input);
      if (isdigit(curr) || curr == '-') return (one_token){NUM, read_value(curr, input, pred_num, 0)};
      if (isalpha(curr) || curr == '_') return (one_token){IDN, read_value(curr, input, pred_idn, 0)};
      fprintf(stderr, "Invalid character %c.\n", curr);
      exit(EXIT_FAILURE);
  }
}
