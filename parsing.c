#include <stdio.h>
#include <stdlib.h>

// for windows >> substitute readline && add_history function
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

// substitute readline funtion
char* readline(char* prompt){
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* buffer_str = malloc(strlen(buffer)+1);
    strcpy(buffer_str, buffer);
    buffer_str[strlen(buffer_str)-1] = '\0';
    return buffer_str;
}

// substitute add_history function
void add_history(char* sub_input){ };

#else

#include <editline/readline.h>
#include <editline/history.h>

#endif

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
