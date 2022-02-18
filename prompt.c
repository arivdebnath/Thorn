#include <stdio.h>
#include <stdlib.h>

#include <editline/readline.h>
#include <editline/history.h>

int main(int argc, char** argv){

    //displays basic information
    puts("Thorn version 0.0.1");
    puts("Press Ctrl+C to Exit\n");

    while(1){

        // prompt output and getting input
        char* input = readline("thorn> ");

        add_history(input);

        printf("No no no %s\n", input);

        free(input);
    }

    return 0;

}
