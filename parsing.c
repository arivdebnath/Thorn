#include <stdio.h>
#include <stdlib.h>
#include "mpc.h"

// for windows >> substitute readline && add_history function
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

// substitute readline funtion
char *readline(char *prompt)
{
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char *buffer_str = malloc(strlen(buffer) + 1);
    strcpy(buffer_str, buffer);
    buffer_str[strlen(buffer_str) - 1] = '\0';
    return buffer_str;
}

// substitute add_history function
void add_history(char *sub_input){};

#else

#include <editline/readline.h>
#include <editline/history.h>

#endif

int number_of_nodes(mpc_ast_t* t){
    if(t->children_num == 0) {return 1;}
    if(t-> children_num >= 1) {
        int total = 1;
        for(int i=0; i<t->children_num; i++){
            total += number_of_nodes(t->children[i]);
        }
        return total;
    }
    return 0;
}

// struct declarations 
struct tval;
struct tenv;
typedef struct tval tval;
typedef struct tenv tenv;

//enums
enum{ TVAL_NUM, TVAL_ERR, TVAL_SYM, TVAL_FUNC, TVAL_SYEXPR, TVAL_QEXPR};
enum{ TERR_DIV_ZERO, TERR_INV_OP, TERR_BAD_NUM };

// function pointer
typedef tval* ( *tbuiltin )(tval*, tenv*);

// value struct
typedef struct tval{
    int type;
    long num;
    // Error and Symbol/operator
    char* err;
    char* sym;
    tbuiltin func;
    // Count and pointer to a list of tval*
    int count;
    tval** cell;
}tval;

struct tenv{
    int count;
    char** syms;
    tval** vals;    
};

tval* tval_copy(tval* v){
    tval* x = malloc(sizeof(tval));
    x->type = v->type;
    
    switch(v->type){
        case TVAL_FUNC:
            x->func = v->func;
            break;
        case TVAL_NUM:
            x->num = v->num;
            break;

        case TVAL_ERR:
            x->err = malloc(strlen(v->err)+1);
            strcpy(x->err, v->err);
            break;

        case TVAL_SYM:
            x->sym = malloc(strlen(v->sym)+1);
            strcpy(x->sym, v->sym);
            break;

        case TVAL_SYEXPR:
        case TVAL_QEXPR:
            x->count = v->count;
            x->cell = malloc(sizeof(tval*)*v->count);
            for(int i=0; i<v->count; i++){
                x->cell[i] = tval_copy(v->cell[i]);
            }
            break;
    }
    return x;
}

tval* tval_num(long x){
    tval* v = malloc(sizeof(tval));
    v->type = TVAL_NUM;
    v->num = x;
    return v;
}

tval* tval_err(char* m){
    tval* v = malloc(sizeof(tval));
    v->type = TVAL_ERR;
    v->err = malloc(strlen(m)+1);
    strcpy(v->err, m);
    return v;
}

tval* tval_sym(char* s){
    tval* v = malloc(sizeof(tval));
    v->type = TVAL_SYM;
    v->sym = malloc(strlen(s)+1);
    strcpy(v->sym, s);
    return v;
}

tval* tval_func(tbuiltin funct){
    tval* v = malloc(sizeof(tval));
    v->type = TVAL_FUNC;
    v->func = funct;
    return v; 
}

tval* tval_syexpr(void){
    tval* v = malloc(sizeof(tval));
    v->type = TVAL_SYEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

tval* tval_qexpr(void){
    tval* v = malloc(sizeof(tval));
    v->type = TVAL_QEXPR;
    v->count = 0;
    v->cell = NULL;
    return v;
}

// Function to delete the heap memory acquired from malloc
void tval_del(tval* v) {
    switch(v->type){
        case TVAL_NUM:
            break;
        case TVAL_ERR:
            free(v->err);
            break;
        case TVAL_SYM:
            free(v->sym);
            break;
        case TVAL_FUNC:
            break;
        case TVAL_SYEXPR:
            for(int i=0; i<v->count; i++){
                tval_del(v->cell[i]);
            }
            free(v->cell);
            break;
        case TVAL_QEXPR:
            for(int i = 0; i<v->count; i++){
                tval_del(v->cell[i]);
            }
            free(v->cell);
            break;
    }
    free(v);
}

tval* tval_read_num(mpc_ast_t* t){
    errno = 0;
    long x = strtol(t->contents, NULL, 10);
    return errno != ERANGE ? tval_num(x) : tval_err("Invalid Number");
}

tval* tval_add(tval* v, tval* x){
    v->count += 1;
    v->cell = realloc(v->cell, sizeof(tval*) * v->count);
    v->cell[v->count-1] = x;
    return v;
}

tval* tval_read(mpc_ast_t* t){
    if(strstr(t->tag, "number")) { return tval_read_num(t);}
    if(strstr(t->tag, "symbol")) { return tval_sym(t->contents);}

    tval* x = NULL;
    if(!strcmp(t->tag, ">")) { x = tval_syexpr(); }
    if(strstr(t->tag, "syexpr")) { x = tval_syexpr(); }
    if(strstr(t->tag, "qexpr")) { x = tval_qexpr(); }

    for(int i = 0; i<t->children_num; i++){
        if(!strcmp(t->children[i]->contents, "(")) {continue;}
        if(!strcmp(t->children[i]->contents, ")")) {continue;}
        if(!strcmp(t->children[i]->contents, "{")) {continue;}
        if(!strcmp(t->children[i]->contents, "}")) {continue;}
        if(!strcmp(t->children[i]->tag, "regex")) {continue;}

        x = tval_add(x, tval_read(t->children[i]));
    }
    return x;
}
void tval_print(tval* v);

void tval_syexpr_print(tval* v, char open, char close){
    putchar(open);

    for(int i = 0; i<v->count; i++){
        tval_print(v->cell[i]);
        //for no trailing space after the last element 
        if(i!=v->count-1){
            putchar(' ');
        }
    }
    putchar(close);
}

void tval_print(tval* v){
    switch(v->type){
        case TVAL_NUM: printf("%li", v->num); break;
        case TVAL_SYM: printf("%s", v->sym); break;
        case TVAL_ERR: printf("Error: %s", v->err); break;
        case TVAL_FUNC: printf("<function>"); break;
        case TVAL_SYEXPR: tval_syexpr_print(v, '(', ')'); break;
        case TVAL_QEXPR: tval_syexpr_print(v, '{', '}'); break;
    }
}

// seperate function for printing line
void tval_println(tval* v){ 
    tval_print(v);
    putchar('\n');
}

//Functions for evaluating S-expressions
tval* tval_pop(tval* v, int i){
    //the required item
    tval* x = v->cell[i];

    // deleting the tval* x and moving the tval*s following it one place to the left in the cell array.
    memmove(&v->cell[i], &v->cell[i+1], sizeof(tval*) * v->count-1);

    v->count-- ;
    v->cell = realloc(v->cell, sizeof(tval*) * v->count);
    return x;
}
tval* tval_take(tval* v, int i){
    tval* x = tval_pop(v, i);
    tval_del(v);
    return x;
}

tval* builtin_op(tval* a, char* op){
    for(int i=0; i<a->count; i++){
        if(a->cell[i]->type!=TVAL_NUM){
            tval_del(a);
            return tval_err("Cannot operate on a non-number!");
        }
    }
    tval* x = tval_pop(a, 0);

    if(!strcmp(op,"-") && a->count==0){
        x->num = -x->num;
    }

    while(a->count > 0){
        tval* y = tval_pop(a, 0);

        if(!strcmp(op, "+")){x->num += y->num ;}
        if(!strcmp(op, "-")){x->num -= y->num ;}
        if(!strcmp(op, "*")){x->num *= y->num ;}
        if(!strcmp(op, "%")){x->num %= y->num ;}
        if(!strcmp(op, "^")){x->num = (long) pow(x->num, y->num);}
        if(!strcmp(op, "/")){
            if(y->num==0){
                tval_del(x); 
                tval_del(y);
                x = tval_err("Division By Zero!");
                break;
            }
            x->num /= y->num;
        }

        tval_del(y);
    }
    tval_del(a);
    return x;
}

tval* builtin(tval* a, char* func);
tval* tval_eval(tval* v);

tval* tval_syexpr_eval(tval* v){
    for(int i=0; i<v->count; i++){
        v->cell[i] = tval_eval(v->cell[i]);
    }

    for(int i=0; i<v->count; i++){
        if(v->cell[i]->type == TVAL_ERR){
            return tval_take(v, i);
        }
    }
    
    if(v->count == 0){ return v; }
    if(v->count == 1){ return tval_take(v, 0); }

    tval* f = tval_pop(v, 0);

    if(f->type!=TVAL_SYM){
        tval_del(f);
        tval_del(v);
        return tval_err("Should begin with an operator!");
    }

    tval* result = builtin(v, f->sym);
    tval_del(f);
    return result;

}

tval* tval_eval(tval* v){
    if(v->type == TVAL_SYEXPR) { return tval_syexpr_eval(v); }
    return v;
}
// Macro for error handling
#define TASSERT(arg, cond, err) \
    if(!(cond)){                \
        tval_del(arg);          \
        return tval_err(err);   \
    }

// Functions for  Q-expressions
tval* builtin_head(tval* a){
    TASSERT(a, a->count==1, "Function 'head' passed too many arguments!");

    TASSERT(a, a->cell[0]->type==TVAL_QEXPR, "Function 'head' passed incorrect type!");

    TASSERT(a, a->cell[0]->count!=0, "Function 'head' passed {}!");

    tval* x = tval_take(a, 0);

    while(x->count>1){
        tval_del(tval_pop(x, 1));
    }
    return x;
}

tval* builtin_tail(tval* a){
    TASSERT(a, a->count==1, "Function 'tail' passed too many arguments!");
    
    TASSERT(a, a->cell[0]->type==TVAL_QEXPR, "Function 'tail' passed wrong argument type!");

    TASSERT(a, a->cell[0]->count!=0, "Function 'tail' passed {}!");

    tval* x = tval_take(a, 0);
    tval_del(tval_pop(x, 0));
    return x;
}


tval* builtin_list(tval* a){
    a->type=TVAL_QEXPR;
    return a;
}

tval* builtin_eval(tval* a){
    TASSERT(a, a->count==1, "Function 'eval' passed too many arguments!");

    TASSERT(a, a->cell[0]->type==TVAL_QEXPR, "Function 'eval' passed wrong type!");

    tval* x = tval_take(a, 0);
    x->type = TVAL_SYEXPR;
    return tval_eval(x);

}

tval* tval_join(tval* x, tval* y);

tval* builtin_join(tval* a){
    for(int i=0; i<a->count; i++){
        TASSERT(a, a->cell[i]->type==TVAL_QEXPR, "Function 'join' passed wrong type!");
    }

    tval* x = tval_pop(a, 0);

    while(a->count){
        x = tval_join(x, tval_pop(a, 0));
    }
    return x;
}

tval* tval_join(tval* x, tval* y){
    while(y->count){
        x = tval_add(x, tval_pop(y, 0));
    }
    tval_del(y);
    return x;
}

tval* builtin(tval* a, char* func){
    if(!strcmp("list", func)){ return builtin_list(a); }
    if(!strcmp("join", func)){ return builtin_join(a); }
    if(!strcmp("tail", func)){ return builtin_tail(a); }
    if(!strcmp("head", func)){ return builtin_head(a); }
    if(!strcmp("eval", func)){ return builtin_eval(a); }
    if(strstr("+-*/^%", func)) { return builtin_op(a, func); }

    tval_del(a);
    return tval_err("Unknown operation or function!");
}

int main(int argc, char **argv)
{

    /* operator :  \"add\" | \"sub\" | \"mul\" | \"div\" | \"mod\" ;           
    for operators in word */

    // parsers for polish notation
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Thorn = mpc_new("thorn");
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Symbol = mpc_new("symbol");
    mpc_parser_t *Syexpr = mpc_new("syexpr");
    mpc_parser_t *Qexpr = mpc_new("qexpr");

    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                   \
        number   :  /-?[0-9]+/ ;                            \
        symbol   : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&^]+/;       \
        syexpr   : '(' <expr>* ')' ;                        \
        qexpr    : '{' <expr>* '}' ;                         \
        expr     :  <number> | <symbol> | <syexpr> | <qexpr> ;        \
        thorn    : /^/<expr>* /$/ ;                         \
        ",
    Number, Symbol, Syexpr, Qexpr, Expr, Thorn);

    // displays basic information
    puts("Thorn version 0.0.5");
    puts("Press Ctrl+C to Exit\n");

    while (1)
    {

        // prompt output and getting input
        char *input = readline("thorn> ");

        add_history(input);

        mpc_result_t r;
        if(mpc_parse("<stdin>", input, Thorn, &r)){

            tval* x = tval_eval(tval_read(r.output));
            tval_println(x);
            tval_del(x);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        
        free(input);
    }
    mpc_cleanup(6, Number, Symbol, Syexpr, Qexpr, Expr, Thorn);

    return 0;
}
