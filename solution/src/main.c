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
    // read in buffer

    struct image img = {0};
    struct bmp_header header = {0};

    enum read_status rs = from_bmp(in, &img, header);
    read_status_print(stdout, rs);

    img = rotate(img);

    enum write_status ws = to_bmp(out, &img, header);
    write_status_print(stdout, ws);

    fclose(in);
    fclose(out);
    return 0;
}
