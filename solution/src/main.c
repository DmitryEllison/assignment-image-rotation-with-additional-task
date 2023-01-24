#include "utilities/source.h"
#include "utilities/transformation.h"
#include "utilities/wedebug.h"


int main( int argc, char** argv ) {
    (void) argc; (void) argv;

    if (argc != 3) {
        printf("Invalid counts of arguments.\n You inputted %d.", argc);
        return -2;
    }

    FILE* in = fopen(argv[1], "rb");
    FILE* out = fopen(argv[2], "wb");

    struct image img = {0};

    enum read_status rs = from_bmp(in, &img);
    show_image(stderr, &img);
    if ((size_t)rs >= 1)
        read_status_print(stderr, rs);
    else
        read_status_print(stdout, rs);

    img = rotate(img);

    enum write_status ws = to_bmp(out, &img);
    if ((size_t)ws >= 1)
        write_status_print(stderr, ws);
    else
        write_status_print(stdout, ws);

    free(img.data);
    fclose(in);
    fclose(out);
    return 0;
}
