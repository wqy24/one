#ifndef __ONE_PARSE_H__
#define __ONE_PARSE_H__

#include <mpfr.h>
#include <stdio.h>
#include <stdint.h>

typedef struct _one_type {
  enum one_type_base {
    NUM_T, STR_T, AUTO, FN_T, PAIR_T
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
  } com;
} *one_type;

typedef struct _one_AST {
  enum one_AST_type {
    NUM, STR, REF, PARAM, FN, FNCALL, PAIR
  } type;

  union one_AST_value{
    mpfr_t num; char *str; FILE *ref; uint32_t param;

    struct one_AST_fn {
      one_type param_type;
      struct _one_AST *res;

      struct one_AST_cls {
        struct _one_AST *arg;
        struct one_AST_cls *up;
      } *closure;

    } fn;

    struct one_AST_fncall {
      struct _one_AST *fn;
      struct _one_AST *arg;
    } fncall;

    struct one_AST_pair{
      struct _one_AST *left;
      struct _one_AST *right;
    } pair;

  } value;
} *one_AST;

one_type new_one_type(enum one_type_base, union one_type_com);
one_AST new_one_AST(enum one_AST_type, union one_AST_value);
void clean_one_type(one_type);
void clean_one_AST(one_AST);
one_AST parse(FILE *);

#endif
