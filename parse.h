#ifndef __ONE_PARSE_H__
#define __ONE_PARSE_H__

#include <stdio.h>
#include <stdint.h>

struct _one_AST;

typedef struct _one_type {
  enum one_type_base {
    NUM_T, STR_T, AUTO, GEN, FN_T, PAIR_T, NONE_T
  } base;

  union one_type_com{
    struct one_type_fn {
      struct _one_type *param;
      struct _one_type *res;
    } fn;

    struct one_type_pair {
      struct _one_type *left;
      struct _one_type *right;
    } pair;

    struct _one_AST *gen;
  } com;
} *one_type;

typedef struct _one_AST {
  enum one_AST_type {
    LIT_NUM, LIT_STR, LIT_REF, PARAM, FN, FNCALL, PAIR, ARRAY, GEN_DECL, NONE
  } type;

  union one_AST_value{
    char *num; char *str; char *ref; uint32_t param;

    struct one_AST_fn {
      one_type param_type;
      char *param_name;
      struct _one_AST *res;
      struct _one_AST *up;
    } fn;

    struct one_AST_fncall {
      struct _one_AST *fn;
      struct _one_AST *arg;
    } fncall;

    struct one_AST_pair{
      struct _one_AST *left;
      struct _one_AST *right;
    } pair;

    struct one_array {
      struct _one_AST *curr;
      struct one_array *next;
    } *array;

    struct one_AST_gen {
      char *name;
      struct _one_AST *up;
      struct _one_AST *code;
    } gen_decl;
  } value;
} *one_AST;

one_AST parse(FILE *);
void check_type(one_AST);
void clean_one_AST(one_AST);

#endif
