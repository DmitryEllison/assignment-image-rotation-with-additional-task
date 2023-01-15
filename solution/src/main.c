#include "utilities/source.h"


int main( int argc, char** argv ) {
    (void) argc; (void) argv;

    if (argc != 3) {
        printf("Invalid counts of arguments.\n You inputted %d.", argc);
        return -2;
    }

    struct BMP bmp = null_bmp();

    // read in buffer
    enum read_status rs = from_bmp(fopen(argv[1], "rb"), &bmp);
    read_status_print(rs);

    // extract pixels from puffer to image
    buffer2image(&bmp);
    bmp.image = rotate(bmp.image);
    update_header_and_padding(&bmp);
    // add padding and move pixels to buffer
    image2buffer(&bmp);

    show_header(bmp.header);
    // write
    enum write_status ws = to_bmp(fopen(argv[2], "wb"), &bmp);
    write_status_print(ws);

    return 0;
}

