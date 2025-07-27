#ifndef __ONE_PARSE_H__
#define __ONE_PARSE_H__

#include <mpdecimal.h>
#include <stdio.h>
#include <stdint.h>

typedef struct _one_type {
  enum one_type_base {
    NUM_T, STR_T, AUTO, GEN, FN_T, PAIR_T
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

    struct _one_type *gen;
  } com;
} *one_type;

typedef struct _one_AST {
  enum one_AST_type {
    LIT_NUM, LIT_STR, LIT_REF, PARAM, FN, FNCALL, PAIR, ARRAY, NONE
  } type;

  union one_AST_value{
    mpd_t num; char *str; FILE *ref; uint32_t param;

    struct one_AST_fn {
      one_type param_type;
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

  } value;
} *one_AST;

one_AST parse(FILE *);
void check_type(one_AST);

#endif
