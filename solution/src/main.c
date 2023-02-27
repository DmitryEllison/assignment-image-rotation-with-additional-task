#include "utilities/math_work.h"
#include "utilities/source.h"
#include "utilities/transformation.h"
#include "utilities/wedebug.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/// To change args in Clion you need edit configuration
int main( int argc, char** argv ) {
    (void) argc; (void) argv;

    if (argc != 3) {
        printf("Invalid counts of arguments.\n You inputted %d.", argc);
        return -2;
    }

    FILE* in = fopen(argv[1], "rb");
    FILE* out = fopen(argv[2], "wb");

    struct image img = {0};
    // TODO create separate free-function
    struct pixel* old_pointer = img.data;

    enum read_status rs = from_bmp(in, &img);
    if ((size_t)rs >= 1)
    {
        read_status_print(stderr, rs);
        return rs;
    } else
        read_status_print(stdout, rs);

    // TODO: -----------------------------------------------------------------------------------------------------------
    //  1. rotate the image by 0.5 * \pi

    //img = rotate(img);

    // TODO: -----------------------------------------------------------------------------------------------------------
    //  2. blur the image by 3x3 and 5x5 kernel

    struct kernel kernel3 = {
            .height = 3,
            .width = 3,
            .kernel = (double[]){ 1./29, 4./29, 1./29,
                                  4./29, 9./29, 4./29,
                                  1./29, 4./29, 1./29}
    };

    struct kernel kernel5 = {
            .height = 5,
            .width = 5,
            .kernel = (double[]){ 1./181, 4./181, 9./181, 4./181, 1./181,
                                  4./181, 9./181, 16./181, 9./181, 4./181,
                                  9./181, 16./181, 25./181, 16./181, 9./181,
                                  4./181, 9./181, 16./181, 9./181, 4./181,
                                  1./181, 4./181, 9./181, 4./181, 1./181}
    };

    img = convolution(img, kernel3);

    // TODO: -----------------------------------------------------------------------------------------------------------
    //  3. rotate the image by \alpha radian
    //  by this method we can change image in any way we want

    struct kernel kernel_sep = {
            .height = 3,
            .width = 3,
            .kernel = (double[]){ 0.15, 0, -0.15,
                                  0.15, 0, -0.15,
                                  0.15, 0, -0.15,}
    };

    double alpha = M_PI  / 2.;
    struct kernel kernel_alpha = {
            .height = 2,
            .width = 2,
            .kernel = (double[]){ cos(alpha), -sin(alpha),
                                  sin(alpha), cos(alpha)}
    };

    struct kernel kernel_mirror = {
            .height = 2,
            .width = 2,
            .kernel = (double[]){ -1, 0,
                                  0, 1}
    };

    // ---- WORK ----

    //img = matrix_transformation(img, kernel_alpha);
    //free(old_pointer);
    //old_pointer = img.data;
    //img = convolution(img, kernel5);

    // ---- END ----

    enum write_status ws = to_bmp(out, &img);
    if ((size_t)ws >= 1)
    {
        write_status_print(stderr, ws);
        return ws;
    } else
        write_status_print(stdout, ws);

    // ---- FREE MEMORY ----
    free(old_pointer);
    free(img.data);
    fclose(in);
    fclose(out);
    return 0;
}
