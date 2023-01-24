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
    struct pixel* old_pointer = img.data;

    enum read_status rs = from_bmp(in, &img);
    if ((size_t)rs >= 1)
    {
        read_status_print(stderr, rs);
        return rs;
    } else
        read_status_print(stdout, rs);


    //img = rotate(img);
    struct kernel kernel3 = {
            .height = 3,
            .width = 3,
            .kernel = (double[]){ 0.11, 0.11, 0.11,
                                  0.11, 0.11, 0.11,
                                  0.11, 0.11, 0.11}
    };

    struct kernel kernel5 = {
            .height = 5,
            .width = 5,
            .kernel = (double[]){ 0.04, 0.04, 0.04, 0.04, 0.04,
                                  0.04, 0.04, 0.04, 0.04, 0.04,
                                  0.04, 0.04, 0.04, 0.04, 0.04,
                                  0.04, 0.04, 0.04, 0.04, 0.04,
                                  0.04, 0.04, 0.04, 0.04, 0.04 }
    };

    img = convolution(img, kernel5);


    enum write_status ws = to_bmp(out, &img);
    if ((size_t)ws >= 1)
    {
        write_status_print(stderr, ws);
        return ws;
    } else
        write_status_print(stdout, ws);

    free(old_pointer);
    free(img.data);
    fclose(in);
    fclose(out);
    return 0;
}
