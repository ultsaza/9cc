#include "9cc.h"

int label_count = 0;

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");

  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

// コードジェネレーター
void gen(Node *node) {
  switch (node->kind) {
  case ND_RETURN:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_IF: 
      int label = label_count++;
      gen(node->lhs->lhs);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      if(node->rhs) {
        printf("  je  .Lelse%d\n", label);
        gen(node->lhs->rhs);
        printf("  jmp .Lend%d\n", label);
        printf(".Lelse%d:\n", label);
        gen(node->rhs);
      } else {
        printf("  je  .Lend%d\n", label);
        gen(node->lhs->rhs);
      }
      printf(".Lend%d:\n", label);
      return;
  case ND_ELSE:
      gen(node->lhs);
      return;
  case ND_WHILE:
      label = label_count++;
      printf(".Lbegin%d:\n", label);
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .Lend%d\n", label);
      gen(node->rhs);
      printf("  jmp .Lbegin%d\n", label);
      printf(".Lend%d:\n", label);
      return;
  case ND_FOR:
      label = label_count++;
      if(node->lhs) {
        gen(node->lhs);
      }
      printf(".Lbegin%d:\n", label);
      if(node->rhs->lhs) {
        gen(node->rhs->lhs);
      }
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je  .Lend%d\n", label);
      gen(node->rhs->rhs->rhs);
      if(node->rhs->rhs->lhs) {
        gen(node->rhs->rhs->lhs);
      }
      printf("  jmp .Lbegin%d\n", label);
      printf(".Lend%d:\n", label);
      return;
  case ND_BLOCK:
      for (int i = 0; i < node->block->size; i++) {
        gen(node->block->data[i]);
        printf("  pop rax\n");
      }
      return;
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  }
  printf("  push rax\n");
}
