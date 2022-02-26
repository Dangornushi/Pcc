/*2022.2.25 ~*/

#include "pcc.h"

Token *token;

// Use Err
// printf
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

//次のトークンが記号である場合はトークンを一つ進める
//Trueを返す
bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op)
        return false;
    token = token->next;
    return true;
}

//次のトークンが記号である場合はトークンを進める
//Trueは返さない、それ以外の場合はエラーメッセージ
void expect(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op)
        error_at("%cではありません", &op);
    token = token->next;
}

//次のトークンが数字である場合はトークンを読み進める
//それ以外はエラーメッセージ
int expect_number() {
    if (token->kind != TK_NUM) {
        printf("%d\n", token->kind);
        error_at(token->str, "not number");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

//新しいトークンを作成してcurにつなげる
Token *new_token(TokenKind kind, Token *cur, char *str) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
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
        if (strchr("+-*/()", *p)) {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "invalid token");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

Node *new_node(NodeKind kind) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_num(int val) {
    Node *node = new_node(ND_NUM);
    node->val = val ;
    return node;
}

Node *expr() {
    Node *node = mul();

    for (;;) {
        if (consume('+'))
            node = new_binary(ND_ADD, node, mul());
        else if (consume('-'))
            node = new_binary(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume('*'))
            node = new_binary(ND_MUL, node, unary());
        else if (consume('/'))
            node = new_binary(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary() {
    if (consume('+'))
        return unary();
    if (consume('-'))
        return new_binary(ND_SUB, new_num(0), unary());
    return primary();
}

Node *primary() {
    //次のトークンが(なら( expr )であるはず
    if (consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
    }

    //そうでなけれゔぁ数字であるはず
    return new_num(expect_number());
}

void gen(Node *node) {
    if (node->kind == ND_NUM) {
        printf("  push %d\n", node->val);
        //printf("%D%D%D%D", PUSH, SP, node->val,SP);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    //printf("%D%D%D%d", POP, SP, 0, SP);
    printf("  pop rax\n");
    printf("%D%D%D%d", POP, SP, 1, SP);

    switch (node->kind) {
        case ND_ADD:
            printf("  add rax, rdi\n");
            //printf("%D%D%D%D%D%D",ADD,SP,0,SP,1,SP);
            break;
        case ND_SUB:
            printf("  sub rax, rdi\n");
            //printf("%D%D%D%D%D%D",SUB,SP,0,SP,1,SP);
            break;
        case ND_MUL:
            printf("  imul rax, rdi\n");
            //printf("%D%D%D%D%D%D",MUL,SP,0,SP,1,SP);
            break;
        case ND_DIV:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            //printf("%D%D%D%D%D%D",DIV,SP,0,SP,1,SP);
            break;
        default:
            break;
    }

    printf("  push rax\n");
    //printf("%D%D%D%D",PUSH,SP,0,SP);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が不正です\n");
        return 1;
    }


    user_input = argv[1];
    token = tokenize(user_input);
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("_main:\n");

    gen(node);

    printf("  pop rax\n");
    printf("  ret\n");

    return 0;
}
