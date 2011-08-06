#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

typedef enum {
    LIST,
    LITERAL,
} Nodetype;

struct _Node {
    Nodetype type;
    void     *val; // array or int
};
typedef struct _Node Node;

void  plis_ast_dump(Node *);
void  plis_ast_free(Node *);
int   plis_find_ending_bracket(char *);
Node* plis_parse(char *);
int   plis_sum(Node **);
int   plis_say(Node **);
int   plis_funcall(char *, Node **);
int   plis_eval(Node *);

void plis_ast_dump(Node *n)
{
    if (n->type == LITERAL) {
        printf("'%s', ", (char *)n->val);
    } else {
        printf("LIST: [");
        Node **list = (Node **)n->val;
        Node *cur;
        int i = 0;
        while ((cur = list[i++])) {
            plis_ast_dump(cur);
        }
        printf("]");
    }
}

void plis_ast_free(Node *n)
{
    if (n->type == LITERAL) {
        free(n->val);
    } else {
        Node **list = (Node **)n->val;
        Node *cur;
        int i = 0;
        while ((cur = list[i++])) {
            plis_ast_free(cur);
        }
        free(list);
    }
    free(n);
}

int plis_find_ending_bracket(char *code)
{
    int pos = 0;
    int lparens = 0;
    int rparens = 0;

    do {
        lparens += (code[pos] == '(');
        rparens += (code[pos] == ')');
        pos++;
        if (lparens == rparens) break;
    } while(code[pos]);

    if (lparens != rparens) {
        //syntax error
        return -1;
    }
    return pos;
}

Node * plis_parse(char *code)
{
    Node *ret;

    int pos = plis_find_ending_bracket(code);
    if (pos == -1) {
        return NULL;
    }

    char list[pos + 1];
    strncpy(list, code, pos);

    Node **elems = malloc(sizeof(Node) * 16); //XXX
    int elemsiter = 0;

    char *cursor = list + 1;
    while (cursor - list < pos) {
        if (cursor[0] == '(') {
            int len = plis_find_ending_bracket(cursor);
            Node *n = plis_parse(cursor);
            if (n == NULL) {
                // syntax error
                free(elems);
                return NULL;
            }
            elems[elemsiter++] = n;
            cursor += len;
        } else {
            char *space   = strchr(cursor, ' ');
            char *bracket = strchr(cursor, ')');
            char *delim;

            if (!space) {
                delim = bracket;
            } else if (space < bracket) {
                delim = space;
            } else {
                delim = bracket;
            }
            
            int len = delim - cursor;

            if (len) {
                Node *n = malloc(sizeof(Node));
                n->type = LITERAL;

                char *val = malloc(sizeof(char) * len + 1);
                strncpy(val, cursor, len);
                n->val = val;

                elems[elemsiter++] = n;
            }
            cursor = delim + 1;
        }
    }

    elems[elemsiter] = NULL;

    ret = malloc(sizeof(Node));
    ret->type = LIST;
    ret->val  = elems;

    return ret;
}

int plis_sum(Node **ast)
{
    int ret = 0;
    Node *cur;
    int i = 0;
    while ((cur = ast[i++])) {
        ret += plis_eval(cur);
    }
    return ret;
}

int plis_say(Node **ast)
{
    Node *cur;
    int i = 0;
    while ((cur = ast[i++])) {
        printf("%d\n", plis_eval(cur));
    }
    return 0;
}
int plis_funcall(char *call, Node **ast)
{
    if (strcmp(call, "sum") == 0) {
        return plis_sum(ast);
    } else if (strcmp(call, "say") == 0) {
        return plis_say(ast);
    } else {
        fprintf(stderr, "Unknown builtin '%s'\n", call);
    }
    return -1;
}

int plis_eval(Node *ast)
{
    if (ast->type == LITERAL) {
        return atoi(ast->val);
    } else {
        Node **list = (Node **)ast->val;
        if (list[0] == NULL) return 0;
        if (list[0]->type == LITERAL) {
            return plis_funcall(list[0]->val, list + 1);
        } else {
            fprintf(stderr, "Unable to handle that type of list\n");
            return -1;
        }
    }
}

int main(void)
{
    char *code;
    while ((code = readline("> "))) {
        if (strlen(code) == 0) {
            continue;
        }
        if (!strncmp (code, "q", 1))
            break;
        add_history(code);
        Node *ast = plis_parse(code);
        if (ast) {
            int r = plis_eval(ast);
            printf("Result: %d\n", r);
            plis_ast_free(ast);
        } else {
            fprintf(stderr, "Syntax error\n");
        }
    }
    return 0;
}
