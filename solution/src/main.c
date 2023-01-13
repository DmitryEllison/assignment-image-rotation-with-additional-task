#include "utilities/source.h"

int main( int argc, char** argv ) {
    (void) argc; (void) argv; // supress 'unused parameters' warning

    if (argc != 3) {
        printf("Invalid counts of arguments.\n You inputted %d.", argc);
        return -2;
    }

    struct BMP bmp = {0};

    switch (from_bmp(fopen(argv[1], "rb"), &bmp.source_image)) {
        case READ_OK: {
            printf("File \"%s\" has been read well.", argv[1]);
            break;
        }
        case READ_INVALID_BITS: {
            printf("Invalid bits.");
            break;
        }
        case READ_INVALID_HEADER: {
            printf("Invalid header.");
            break;
        }
        case READ_INVALID_SIGNATURE:{
            printf("Invalid signature.");
            break;
        }
        default: {
            printf("Something went wrong with reading of file \"%s\"", argv[1]);
        }
    }

    bmp.changed_image = rotate(bmp.source_image);

    switch (to_bmp(fopen(argv[2], "wb"), &bmp.changed_image)) {
        case WRITE_OK: {
            printf("File \"%s\" has been written well.", argv[2]);
            break;
        }
        default: {
            printf("Something went wrong with writing changed image in the file \"%s\"", argv[2]);
        }
    }

    return 0;
}
