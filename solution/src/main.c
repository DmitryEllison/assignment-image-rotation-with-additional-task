#include <stdio.h>

int main( int argc, char** argv ) {
    (void) argc; (void) argv; // supress 'unused parameters' warning

    if (argc != 3) {
        printf("Аргументов должно быть три штуки.\nУ тебя их %d.", argc);
        return -2;
    }



    return 0;
}
