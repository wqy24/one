#include <stdio.h>
#include <stdlib.h>
#include "../tokens.h"

#define with_value(type, token) do { \
  puts(type); \
  puts(token.value); \
  free(token.value); \
} while (0)

#define without_value(type, token) do { \
  puts(type); \
  if (token.value != NULL) { \
    fputs(token.value, stderr); \
    free(token.value); \
    fputs("\nExtra value in "type" token\n", stderr); \
    return EXIT_FAILURE; \
  }\
} while (0)

int main(int argc, char **argv)
{
  if (argc != 2) {
    fputs("Invalid argument number\n", stderr);
    return EXIT_FAILURE;
  }
  one_token curr;
  FILE *input = fopen(argv[1], "r");
  do {
    switch ((curr = read_one_token(input)).type) {
      case STR:
        with_value("STR", curr);
        break;
      case NUM:
        with_value("NUM", curr);
        break;
      case REF:
        with_value("REF", curr);
        break;
      case IDN:
        with_value("IDN", curr);
        break;
      case WAVE:
        without_value("WAVE", curr);
        break;
      case AT:
        without_value("AT", curr);
        break;
      case ET:
        without_value("ET", curr);
        break;
      case QUESTION:
        without_value("QUESTION", curr);
        break;
      case COLON:
        without_value("COLON", curr);
        break;
      case LT:
        without_value("LT", curr);
        break;
      case GT:
        without_value("GT", curr);
        break;
      case L_PRN:
        without_value("L_PRN", curr);
        break;
      case R_PRN:
        without_value("R_PRN", curr);
        break;
      case L_BRK:
        without_value("L_BRK", curr);
        break;
      case R_BRK:
        without_value("R_BRK", curr);
        break;
      case COMMA:
        without_value("COMMA", curr);
        break;
      case PIPE:
        without_value("PIPE", curr);
        break;
      case CARET:
        without_value("CARET", curr);
        break;
      case FEOF:
        without_value("FEOF", curr);
        break;
    }
  } while(curr.type != FEOF);
  return EXIT_SUCCESS;
}
