#include "utilities/source.h"


int main( int argc, char** argv ) {
    (void) argc; (void) argv;

    if (argc != 3) {
        printf("Invalid counts of arguments.\n You inputted %d.", argc);
        return -2;
    }

    struct BMP bmp = {0};

    // read
    enum read_status rs = from_bmp(fopen(argv[1], "rb"), &bmp);
    read_status_print(rs);

    show_header(bmp.header);
    // change
    buffer2image(bmp);
    rotate(bmp.image);
    update_header_and_padding(bmp);
    image2buffer(bmp);

    show_header(bmp.header);
    // write
    enum write_status ws = to_bmp(fopen(argv[2], "wb"), &bmp);
    write_status_print(ws);

    return 0;
}

