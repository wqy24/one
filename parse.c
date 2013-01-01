#include "parse.h"
#include "quickfail.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

one_type new_one_type(enum one_type_base, union one_type_com);
one_AST new_one_AST(enum one_AST_type, union one_AST_value);
void clean_one_type(one_type);

one_type new_one_type(enum one_type_base base, union one_type_com com)
{
  one_type res = malloc(sizeof(struct _one_type));
  check_mem(res);
  res->base = base;
  res->com = com;
  return res;
}

struct one_array *new_array(struct one_array *prev, one_AST curr)
{
  struct one_array *res = malloc(sizeof(struct one_array));
  if (prev) prev->next = res;
  res->next = NULL;
  res->curr = curr;
  return res;
}

one_AST new_one_AST(enum one_AST_type type, union one_AST_value value)
{
  one_AST res = malloc(sizeof(struct _one_AST));
  check_mem(res);
  res->type = type;
  res->value = value;
  return res;
}

void clean_one_type(one_type type)
{
  switch (type->base) {
    case FN:
      clean_one_type(type->com.fn.param);
      clean_one_type(type->com.fn.res);
      break;
    case PAIR:
      clean_one_type(type->com.pair.left);
      clean_one_type(type->com.pair.right);
      break;
    case GEN:
      clean_one_type(type->com.gen);
      break;
  }

  free(type);
}

void clean_array(struct one_array *arr)
{
  clean_one_AST(arr->curr);
  if (arr->next) clean_array(arr->next);
  free(arr);
}

void clean_one_AST(one_AST ast)
{
  switch (ast->type) {
    case LIT_STR:
      free(ast->value.str);
      break;
    case LIT_REF:
      fclose(ast->value.ref);
      break;
    case FN:
      clean_one_type(ast->value.fn.param_type);
      clean_one_AST(ast->value.fn.res);
      clean_one_AST(ast->value.fn.up);
      break;
    case FNCALL:
      clean_one_AST(ast->value.fncall.fn);
      clean_one_AST(ast->value.fncall.arg);
      break;
    case PAIR:
      clean_one_AST(ast->value.pair.left);
      clean_one_AST(ast->value.pair.right);
      break;
    case ARRAY:
      clean_array(ast->value.array);
      break;
  }

  free(ast);
}
