#include "parse.h"
#include "tokens.h"
#include "quickfail.h"
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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
      clean_one_AST(type->com.gen);
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
    case LIT_NUM:
      free(ast->value.num);
      break;
    case LIT_STR:
      free(ast->value.str);
      break;
    case LIT_REF:
      free(ast->value.ref);
      break;
    case FN:
      clean_one_type(ast->value.fn.param_type);
      free(ast->value.fn.param_name);
      clean_one_AST(ast->value.fn.res);
      //clean_one_AST(ast->value.fn.up);
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
    case GEN_DECL:
      free(ast->value.gen_decl.name);
      clean_one_AST(ast->value.gen_decl.code);
      break;
  }

  free(ast);
}

struct token_queue {
  one_token curr;
  FILE *input;
};

one_token pop(struct token_queue queue)
{
  one_token curr = queue.curr;
  queue.curr = read_one_token(queue.input);
  return curr;
}

one_AST up(one_AST context)
{
  switch (context->type) {
    case FN:
      return context->value.fn.up;
    case GEN_DECL:
      return context->value.gen_decl.up;
    default: fail("Invalid AST as a context");
  }
}

int stop_read_type(one_type l, one_token r)
{
  switch (r.type) {
    case ET: case GT: case R_PRN: return 1;
    default: return 0;
  }
}

one_type simple_typefy(one_token, struct token_queue, one_AST);

one_type read_type(struct token_queue queue, one_type left, one_AST context)
{
  one_token op;
  if (stop_read_type(left, queue.curr)) return left;
  switch ((op = pop(queue)).type) {
    case WAVE:
      return read_type(queue, new_one_type(FN_T, (union one_type_com){
        (struct one_type_fn){left, read_type(queue, simple_typefy(pop(queue), queue, context), context)}
      }), context);
    default: fail("Invalid type composer: %d", op.type);
  }
}

one_type simple_typefy(one_token token, struct token_queue queue, one_AST context)
{
  switch (token.type) {
    case IDN:
      if (!strncmp(token.value, "num", 4)) {
        return new_one_type(NUM, (union one_type_com){});
      } else if (!strncmp(token.value, "str", 4)) {
        return new_one_type(STR, (union one_type_com){});
      } else {
        for (one_AST ctx = context; ; ctx = up(ctx)) {
          if (!ctx) fail("Unknown generitic type: %s", token.value);
          else if (ctx->type == GEN_DECL && !strcmp(ctx->value.gen_decl.name, token.value)) {
            free(token.value);
            return new_one_type(GEN, (union one_type_com){.gen = ctx});
          }
        }
      }
    case L_PRN:
      one_token peek_prn = pop(queue);
      if (peek_prn.type == R_PRN) return new_one_type(NONE_T, (union one_type_com){});
      one_type type = read_type(queue, simple_typefy(peek_prn, queue, context), context);
      check(pop(queue).type == R_PRN, "( not matched");
      return type;
    case LT:
      one_type pair_left = read_type(queue, simple_typefy(pop(queue), queue, context), context);
      check(pop(queue).type == ET, "A pair should have `&` between 2 items");
      one_type pair_right = read_type(queue, simple_typefy(pop(queue), queue, context), context);
      check(pop(queue).type == GT, "A pair should end with `>`");
      return new_one_type(PAIR_T, (union one_type_com){.pair = (struct one_type_pair){pair_left, pair_right}});
    default: fail("Invalid token %d", token.type);
  }
}

one_AST simple_ASTfy(one_token, struct token_queue, one_AST);

int stop(one_AST l, one_token r)
{
  switch (r.type) {
    case R_PRN: case R_BRK: case GT:
    case ET: case COMMA: case FEOF:
      return 1;
    case PIPE:
      return (l->type == FNCALL? 1 : 0);
    default: return 0;
  }
}

one_AST parse_recur(struct token_queue queue, one_AST left, one_AST context)
{
  one_token op;
  if (stop(left, queue.curr)) return left;
  switch ((op = pop(queue)).type) {
    case NUM: case STR: case REF:
    case L_PRN: case L_BRK: case LT:
    /*case AT:*/ case IDN: case QUESTION:
      return parse_recur(queue, new_one_AST(FNCALL,
        (union one_AST_value){
          .fncall = (struct one_AST_fncall){left, simple_ASTfy(op, queue, context)}
        }
      ), context);
    case PIPE:
      return parse_recur(queue, new_one_AST(FNCALL,
        (union one_AST_value){
          .fncall = (struct one_AST_fncall){parse_recur(queue, simple_ASTfy(pop(queue), queue, context), context), left}
        }
      ), context);
    default: fail("Invalid token as operator: %d", op.type);
  }
}

one_AST simple_ASTfy(one_token token, struct token_queue queue, one_AST context)
{
  switch (token.type) {
    case NUM:
      return new_one_AST(LIT_NUM, (union one_AST_value){.num = token.value});
    case STR:
      return new_one_AST(LIT_STR, (union one_AST_value){.str = token.value});
    case REF:
      return new_one_AST(LIT_REF, (union one_AST_value){.ref = token.value});
    case L_PRN:
      one_token peek_prn = pop(queue);
      if (peek_prn.type == R_PRN) return new_one_AST(NONE, (union one_AST_value){});
      one_AST tree = parse_recur(queue, simple_ASTfy(peek_prn, queue, context), context);
      check(pop(queue).type == R_PRN, "( not matched");
      return tree;
    case L_BRK:
      one_token peek_brk = pop(queue);
      if (peek_brk.type == R_BRK) return new_one_AST(ARRAY, (union one_AST_value){.array = NULL});

      struct one_array *array_head = new_array(NULL, parse_recur(queue, simple_ASTfy(pop(queue), queue, context), context)),
                       *array_end = array_head;
      while ((peek_brk = pop(queue)).type == COMMA) {
        array_end = new_array(array_end, parse_recur(queue, simple_ASTfy(pop(queue), queue, context), context));
      }
      check(peek_brk.type == R_BRK, "Array doesn't ends with `]`");
      return new_one_AST(ARRAY, (union one_AST_value){.array = array_head});
    case LT:
      one_AST pair_left = parse_recur(queue, simple_ASTfy(pop(queue), queue, context), context);
      check(pop(queue).type == ET, "A pair should have `&` between 2 items");
      one_AST pair_right = parse_recur(queue, simple_ASTfy(pop(queue), queue, context), context);
      check(pop(queue).type == GT, "A pair should end with `>`");
      return new_one_AST(PAIR, (union one_AST_value){.pair = (struct one_AST_pair){pair_left, pair_right}});
#if 0
    case AT:
      one_AST ctx;
      for (ctx = context; ctx->type != FN; ctx = up(ctx));
      return ctx;
#endif
    case IDN:
      one_token next = queue.curr;
      if (next.type == CARET) {
        one_type type;
        type = read_type(queue, simple_typefy(pop(queue), queue, context), context);
        pop(queue);
        check(pop(queue).type == COLON, "Type descriptor of a function should end with `:`");
        one_AST res = new_one_AST(FN,
          (union one_AST_value){
            .fn = (struct one_AST_fn){type, token.value, NULL, context}
          }
        );
        res->value.fn.res = parse_recur(queue, simple_ASTfy(pop(queue), queue, res), res);
        return res;
      } else if (next.type == COLON) {
        pop(queue);
        one_AST res = new_one_AST(FN,
          (union one_AST_value){
            .fn = (struct one_AST_fn){new_one_type(AUTO, (union one_type_com){}), token.value, NULL, context}
          }
        );
        res->value.fn.res = parse_recur(queue, simple_ASTfy(pop(queue), queue, res), res);
        return res;
      } else {
        one_AST ctx; uint32_t p;
        for (ctx = context, p = 0; ; ctx = up(ctx), p++) {
          if (!ctx) fail("param unmatched: %s", token.value);
          else if (ctx->type == FN && !strcmp(token.value, ctx->value.fn.param_name)) {
            free(token.value);
            return new_one_AST(PARAM, (union one_AST_value){.param = p});
          }
        }
      }
    case QUESTION:
      one_token g = pop(queue);
      check(g.type == IDN, "Need an identifier after a `?`");
      check(pop(queue).type == COLON, "Need a `:` after a generitic definition.");
      one_AST res = new_one_AST(GEN_DECL,
        (union one_AST_value){
          .gen_decl = (struct one_AST_gen){token.value, context, NULL}
        }
      );
      res->value.gen_decl.code = parse_recur(queue, simple_ASTfy(pop(queue), queue, res), res);
      return res;
    default:
      fail("Bad token: %d", token.type);
  }
}

one_AST parse(FILE *input)
{
  struct token_queue queue = {(one_token){}, input};
  return parse_recur(queue, simple_ASTfy(pop(queue), queue, NULL), NULL);
}
