#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_IDENT,    // 識別子
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークンs
  TK_RETURN,   // return
  TK_IF,       // if
  TK_ELSE,     // else
  TK_WHILE,    // while
  TK_FOR,      // for
} TokenKind;

// トークン型
typedef struct Token Token;
struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LT,        // <
  ND_LE,        // <=
  ND_ASSIGN,    // =
  ND_LVAR,      // ローカル変数
  ND_NUM,       // 整数
  ND_RETURN,    // 
  ND_IF,        // if
  ND_ELSE,      // else
  ND_WHILE,     // while
  ND_FOR,       // for
  ND_BLOCK,     // {}
} NodeKind;

// ブロックに含まれる式を保持するベクタ

typedef struct Node Node;
typedef struct NodeVector NodeVector;
struct NodeVector {
    Node **data;    // データの配列
    int size;     // 現在の要素数
};

// 抽象構文木のノードの型
struct Node {
  NodeKind kind;     // ノードの型
  Node *lhs;         // 左辺
  Node *rhs;         // 右辺
  int val;           // kindがND_NUMの場合のみ使う
  int offset;        // kindがND_LVARの場合のみ使う
  NodeVector *block; // kindがND_BLOCKの場合のみ使う
};



// ローカル変数の型
typedef struct LVar LVar;
struct LVar {
  LVar *next; // 次の変数かNULL
  char *name; // 変数の名前
  int len;    // 名前の長さ
  int offset; // RBPからのオフセット
};


// グローバル変数
extern Token *token;     // 現在着目しているトークン
extern char *user_input; // 入力プログラム
extern Node *code[100];  // コード生成時に使うバッファ
extern LVar *locals;     // ローカル変数(連結リスト)
extern int label_count;  // 分岐のラベル

// プロトタイプ宣言
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
int is_alnum(char c);
bool consume(char *op);
Token *consume_ident();
Token *consume_return();
Token *consume_if();
Token *consume_else();
Token *consume_while();
Token *consume_for();
void expect(char *op);
int expect_number();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize(char *p);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
LVar *find_lvar(Token *tok);

void program();         // program = stmt*
Node *stmt();           // stmt = expr ";" | "{" stmt* "}"　| "if" "(" expr ")" stmt ("else" stmt)? | "while" "(" expr ")" stmt　| "for" "(" expr? ";" expr? ";" expr? ")" stmt | "return" expr ";"
Node *expr();           // expr = assign
Node *assign();         // assign = equality ("=" assign)?
Node *equality();       // equality = relational ("==" relational | "!=" relational)*
Node *relational();     // relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *add();            // add = mul ("+" mul | "-" mul)*
Node *mul();            // mul = unary ("*" unary | "/" unary)*
Node *unary();          // unary = ("+" | "-")? primary
Node *primary();        // primary = num | ident | "(" expr ")"
void gen(Node *node);
