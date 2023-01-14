#include "utilities/source.h"

int main( int argc, char** argv ) {
    (void) argc; (void) argv; // supress 'unused parameters' warning

    if (argc != 3) {
        printf("Invalid counts of arguments.\n You inputted %d.", argc);
        return -2;
    }

    struct BMP bmp = {0};

    switch (from_bmp(fopen(argv[1], "rb"), &bmp)) {
        case READ_OK: {
            printf("\nFile \"%s\" has been read well.", argv[1]);
            break;
        }
        case READ_INVALID_BITS: {
            printf("Invalid bits.");
            break;
        }
        case READ_INVALID_HEADER: {
            printf("Invalid source header.");
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

    delete_padding_and_fill_image(bmp);
    rotate(bmp);
    add_padding_and_fill_buffer(bmp);

    switch (to_bmp(fopen(argv[2], "wb"), &bmp)) {
        case WRITE_OK: {
            printf("\nFile \"%s\" has been written well.", argv[2]);
            break;
        }
        default: {
            printf("Something went wrong with writing changed image in the file \"%s\"", argv[2]);
        }
    }

    return 0;
}
