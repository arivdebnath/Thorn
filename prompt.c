#include <stdio.h>

static char input[2048];

int main(int argc, char** argv){

    //displays basic information
    puts("Thorn version 0.0.1");
    puts("Press Ctrl+C to Exit\n");

    while(1){
        fputs("thorn> ", stdout);
        fgets(input, 2048, stdin);

        printf("No no no %s", input);
    }

    return 0;

}
