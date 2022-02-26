#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SP 0x00
#define MOV 0x01
#define ADD 0x02
#define SUB 0x03
#define MUL 0x04
#define DIV 0x05
#define PUSH 0x06
#define POP 0x07

//トークンの種類

typedef enum {
    TK_RESERVED,//記号
    TK_NUM,//数
    TK_EOF,//入力終了
    TK_ENTER,// \n
} TokenKind;

typedef struct Token Token;

// Token's
struct Token {
    TokenKind kind;
    Token *next;
    int val;
    char *str;
};

char *user_input;

typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_NUM, // number
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind; //ノードの型
    Node *lhs;     //左辺
    Node *rhs;     //右辺
    int val;
};

Node *primary();
Node *mul();
Node *unary();
