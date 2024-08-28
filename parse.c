#include "9cc.h"

// トークナイザー
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

Token *consume_ident() {
  if (token->kind != TK_IDENT)
    return NULL;
  Token *tok = token;
  token = token->next;
  return tok;
}

Token *consume_return() {
  if (token->kind != TK_RETURN)
    return NULL;
  Token *tok = token;
  token = token->next;
  return tok;
}


Token *consume_if() {
  if (token->kind != TK_IF)
    return NULL;
  Token *tok = token;
  token = token->next;
  return tok;
}

Token *consume_else() {
  if (token->kind != TK_ELSE)
    return NULL;
  Token *tok = token;
  token = token->next;
  return tok;
}

Token *consume_while() {
  if (token->kind != TK_WHILE)
    return NULL;
  Token *tok = token;
  token = token->next;
  return tok;
}

void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
    error_at(token->str, "'%s'ではありません", op);
  token = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

bool startswith(char *p, char *q) {
  return memcmp(p, q, strlen(q)) == 0;
}

Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    // 複数文字の演算子
    if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    // 1文字の演算子
    if (strchr("+-*/()<>=;", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    // return
    if (startswith(p, "return") && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    // if
    if (startswith(p, "if") && !is_alnum(p[2])) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }

    // else
    if (startswith(p, "else") && !is_alnum(p[4])) {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }

    // while
    if (startswith(p, "while") && !is_alnum(p[5])) {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }

    // 識別子
    if ('a' <= *p && *p <= 'z') {
        cur = new_token(TK_IDENT, cur, p, 0);
        char *q = p;
        p += strspn(p, "abcdefghijklmnopqrstuvwxyz");
        cur->len = p - q;
        continue;
    }

    // 数値
    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 0);
      char *q = p;
      cur->val = strtol(p, &p, 10); // 数値を読み取った分だけpを進める
      cur->len = p - q;
      continue;
    }

    error_at(token->str, "トークナイズできません");
  }
  
  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

// パーサー
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// 変数を名前で検索する。見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

// program = stmt*
void program() {
  int i = 0;
  while (!at_eof())
    code[i++] = stmt(); // code[i] = stmt()　パース結果の格納
  code[i] = NULL;
}

// stmt = expr ";" | "if" "(" expr ")" stmt ("else" stmt)? | "while" "(" expr ")" stmt | "return" expr ";"
Node *stmt() {
  Node *node;
  if (consume_return()) {
    node = new_node(ND_RETURN, expr(), NULL); // 右辺不要
  } 
  else if (consume_if()) {
    node = new_node(ND_IF, NULL, NULL);
    node->lhs = new_node(ND_IF, node, NULL);
    expect("(");
    node->lhs->lhs = expr();
    expect(")");
    node->lhs->rhs = stmt();
    if (consume_else()) {
      node->rhs = new_node(ND_ELSE, stmt(), NULL);
    }
    return node;
  } 
  else if (consume_while()) {
    node = new_node(ND_WHILE, NULL, NULL);
    expect("(");
    node->lhs = expr();
    expect(")");
    node->rhs = stmt();
    return node;
  }  
  else {
    node = expr();
  }

  if (!consume(";"))
    error_at(token->str, "';'ではないトークンです");
  return node;
}


// expr = assign
Node *expr() {
  return assign();
}

// assign = equality ("=" assign)?
Node *assign() {
  Node *node = equality();
  if (consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if (consume("!="))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LT, node, add());
    else if (consume("<="))
      node = new_node(ND_LE, node, add());
    else if (consume(">"))
      node = new_node(ND_LT, add(), node);
    else if (consume(">="))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

// add = mul ("+" mul | "-" mul)*
Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

// mul = unary ("*" unary | "/" unary)*
Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

// unary = ("+" | "-")? primary
Node *unary() {
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), unary());
  return primary();
}

 // primary = num | ident | "(" expr ")"
Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident(); // freeしたほうが良さそう
  if (tok) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LVAR;
    LVar *lvar = find_lvar(tok);
    if (lvar) {
      node->offset = lvar->offset;
    } else {
      lvar = calloc(1, sizeof(LVar));
      lvar->next = locals;
      lvar->name = tok->str;
      lvar->len = tok->len;
      lvar->offset = (locals ? locals->offset : 0) + 8; // localsは初期化時NULLなので初期化が別途で必要
      node->offset = lvar->offset;
      locals = lvar;
    }
    return node;
  }
  return new_node_num(expect_number());
}