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

//enums
enum{ TVAL_NUM, TVAL_ERR};
enum{ TERR_DIV_ZERO, TERR_INV_OP, TERR_BAD_NUM };

// value struct
typedef struct {
    int type;
    long num;
    int err;
} tval;

tval tval_num(long x){
    tval v;
    v.type = TVAL_NUM;
    v.num = x;
    return v;
}
tval tval_err(int x){
    tval v;
    v.type = TVAL_ERR;
    v.num = x;
    return v;
}

void tval_print(tval v){
    switch (v.type)
    {
    case TVAL_NUM:
        printf("%li", v.num);
        break;
    case TVAL_ERR:
        if(v.err==TERR_DIV_ZERO){
            printf("Error: Division By Zero!");
        }
        if(v.err==TERR_INV_OP){
            printf("Error: Invaild Operation!");
        }
        if (v.err==TERR_BAD_NUM)
        {
            printf("Error: Invaild Number!");
        }
    break;
    }
}
// seperate function for printing line
void tval_println(tval v){ 
    tval_print(v);
    putchar('\n');
}

tval eval_operation(tval x, char* op, tval y){

    if(x.type==TVAL_ERR){ return x; }
    if(y.type==TVAL_ERR){ return y; }

    if(!strcmp(op, "+")){ return tval_num(x.num+y.num); }
    if(!strcmp(op, "-")){ return tval_num(x.num-y.num); }
    if(!strcmp(op, "*")){ return tval_num(x.num*y.num); }
    if(!strcmp(op, "%")){ return tval_num(x.num%y.num); }
    if(!strcmp(op, "/")){ 
        // if(y.num==0){
        //     return tval_err(TERR_DIV_ZERO);
        // }else{
        //     return tval_num(x.num/y.num); }
        return y.num==0 ? tval_err(TERR_DIV_ZERO) : tval_num(x.num/y.num);
        }
    return tval_err(TERR_INV_OP);
}


tval eval(mpc_ast_t* t){
    if(strstr(t->tag, "number")) {
        errno = 0;
        long x = strtol(t->contents, NULL, 10);
        return errno != ERANGE ? tval_num(x) : tval_err(TERR_BAD_NUM);
    }

    char* op = t->children[1]->contents;

    tval x = eval(t->children[2]);

    int j = 3;
    while(strstr(t->children[j]->tag, "expr")){
        x = eval_operation(x, op, eval(t->children[j]));
        j++;
    }
    return x;
}

int main(int argc, char **argv)
{

    /* operator :  \"add\" | \"sub\" | \"mul\" | \"div\" | \"mod\" ;           
    for operators in word */

    // parsers for polish notation
    mpc_parser_t *Expr = mpc_new("expr");
    mpc_parser_t *Thorn = mpc_new("thorn");
    mpc_parser_t *Number = mpc_new("number");
    mpc_parser_t *Operator = mpc_new("operator");

    mpca_lang(MPCA_LANG_DEFAULT,
        "                                                   \
        number   :  /-?[0-9]+/ ;                            \
        operator :  '+' | '-' | '*' | '/' | '%' ;           \
        expr     :  <number> | '(' <operator> <expr>+ ')' ; \
        thorn    : /^/<operator> <expr>+/$/ ;               \
        ",
    Number, Operator, Expr, Thorn);

    // displays basic information
    puts("Thorn version 0.0.4");
    puts("Press Ctrl+C to Exit\n");

    while (1)
    {

        // prompt output and getting input
        char *input = readline("thorn> ");

        add_history(input);

        // printf("No no no %s\n", input);
        mpc_result_t r;
        if(mpc_parse("<stdin>", input, Thorn, &r)){
            // mpc_ast_print(r.output);
            tval result =  eval(r.output);
            tval_println(result);
            mpc_ast_delete(r.output);
        } else {
            mpc_err_print(r.error);
            mpc_err_delete(r.error);
        }
        
        free(input);
    }
    mpc_cleanup(4, Number, Operator, Expr, Thorn);

    return 0;
}
