#include "math_work.h"
#include "source.h"
#include "transformation.h"
#include "wedebug.h"

// ---- READING AND WRITING FILES
enum read_status from_bmp(FILE *in, struct image *img) {
    struct bmp_header header;

    if (fread(&header , sizeof(struct bmp_header), 1, in) != 1) {
        return READ_INVALID_HEADER;
    }

    // Проверяем сигнатуру
    if (header.bfType != 0x4D42) {
        return READ_INVALID_SIGNATURE;
    }


    if (fseek(in, (long) header.bOffBits, SEEK_SET) != 0) {
        return READ_INVALID_BITS;
    }
    size_t padding = get_padding(header.biWidth);
    img->data =  malloc(sizeof(struct pixel) * header.biWidth * header.biHeight);

    for (size_t i = 0; i < header.biHeight; ++i) {
        fread(img->data + (i * header.biWidth), sizeof(struct pixel), header.biWidth,in);
        fseek(in, (long) padding, SEEK_CUR);
    }

    img->height = header.biHeight;
    img->width = header.biWidth;

    show_header(header);
    return READ_OK;
}


enum write_status to_bmp(FILE *out, struct image *img) {
    struct bmp_header header = fill_header(img->width, img->height);

    if (fwrite(&header, sizeof(struct bmp_header), 1, out) != 1) {
        return WRITE_HEADER_ERROR;
    }

    size_t padding = get_padding(header.biWidth);
    size_t index;
    uint8_t zero = 0;

    for (size_t i = 0; i < header.biHeight; ++i) {
        for (size_t j = 0; j < header.biWidth; ++j) {
            index = j + i * header.biWidth;
            if (fwrite(img->data + index, sizeof(struct pixel), 1, out) != 1)
                return WRITE_BUFFER_ERROR;
        }
        for (size_t j = 0; j < padding; ++j) {
            if (fwrite(&zero, 1, 1, out) != 1)
                return WRITE_BUFFER_ERROR;
        }
    }
    show_header(header);

    return WRITE_OK;
}

// ---- WORK WITH HEADER

uint32_t get_padding(uint32_t biWidth) {
    return 4 - (biWidth * 3) % 4;
}

struct bmp_header fill_header(uint32_t width, uint32_t height) {
    struct bmp_header temp = {0};
    uint32_t biSizeImage = (sizeof(struct pixel) * width + get_padding(width) ) * height;

    temp.bfileSize = sizeof(struct bmp_header) + biSizeImage;
    temp.bOffBits = sizeof(struct bmp_header);
    temp.biSizeImage = biSizeImage;
    temp.biWidth = width;
    temp.biHeight = height;

    temp.bfType = 0x4D42;
    temp.biPlanes = 1;
    temp.biBitCount = 24;
    temp.biSize = 40;
    return temp;
}

// ---- WORK WITH IMAGE ----

static uint64_t array_index(uint64_t by_line, uint64_t by_column, uint64_t width) {
    return by_column + by_line * width;
}

struct image rotate(const struct image img) {
    uint64_t width = img.width;
    uint64_t height = img.height;
    struct pixel* temp = malloc(sizeof(struct pixel) * width * height);
    for (uint64_t i = 0; i < height; i++) {
        for (uint64_t j = 0; j < width; j++) {
            temp[ array_index( j + 1,-(i + 1), height) ] = img.data[array_index(i, j, width) ];
        }
    }

    return (struct image) {
            .width = height,
            .height = width,
            .data = temp
    };
}

struct pixel uint16_to_pixel(struct uint16_pixel extend_pixel) {
    return (struct pixel) {
            .b = ( extend_pixel.b > 255) ? 255 : extend_pixel.b,
            .g = ( extend_pixel.g > 255) ? 255 : extend_pixel.g,
            .r = ( extend_pixel.r > 255) ? 255 : extend_pixel.r,
    };
}

struct image convolution(const struct image img, struct kernel const kernel) {
    struct pixel* result = malloc(sizeof(struct pixel) * img.width * img.height);
    struct uint16_pixel temp = {0};
    int64_t x_kernel, y_kernel;

    for (uint64_t y = 0; y < img.height; ++y) {
        for (uint64_t x = 0; x < img.width; ++x) {
            temp.b = 0;
            temp.g = 0;
            temp.r = 0;

            for (uint64_t i = 0; i < kernel.height; ++i) {
                for (uint64_t j = 0; j < kernel.width; ++j) {
                    y_kernel = y + (i - (kernel.height / 2));
                    x_kernel = x + (j - (kernel.width / 2));

                    if (x_kernel < 0 || x_kernel >= img.width ||
                        y_kernel < 0 || y_kernel >= img.height)
                        continue;

                    temp.b += img.data[array_index(y_kernel, x_kernel, img.width)].b *
                                kernel.kernel[array_index(i, j, kernel.width)];
                    temp.g += img.data[array_index(y_kernel, x_kernel , img.width)].g *
                                kernel.kernel[array_index(i, j, kernel.width)];
                    temp.r += img.data[array_index(y_kernel, x_kernel , img.width)].r *
                                kernel.kernel[array_index(i, j, kernel.width)];
                }
            }

            result[array_index( y, x, img.width)] = uint16_to_pixel(temp);
        }
    }

    return (struct image) {
        .width = img.width,
        .height = img.height,
        .data = result
    };
}

struct image matrix_transformation(const struct image img, struct kernel kernel) {

    struct kernel inverse_kernel = get_inverse_kernel(kernel);
    double det = get_determine(kernel);
    inverse_kernel.kernel = (double[]){ (double) kernel.kernel[3] / det, (double) -kernel.kernel[1] / det,
                                        (double) -kernel.kernel[2] / det, (double) kernel.kernel[0] / det };


    show_matrix(kernel);

    // Find new w, h via rectangle vertices
    struct borders border = get_borders(kernel, img.width, img.height);
    uint64_t width = border.w_right - border.w_left;
    uint64_t height = border.h_upper - border.h_bottom;

    struct pixel* result = malloc(sizeof(struct pixel) * width * height);

    uint64_t index = 0;
    struct pixel temp;
    for (int64_t y = border.h_bottom; y < border.h_upper; ++y) {
        for(int64_t x = border.w_left; x < border.w_right; ++x) {
            struct point old_xy = multiply_kernel_on_xy(inverse_kernel, x, y);

            if (old_xy.x < 0 || old_xy.x >= img.width ||
                old_xy.y < 0 || old_xy.y >= img.height) {
                temp = (struct pixel) {
                    .g = 255,
                    .b = 255,
                    .r = 255
                };
            } else {
                temp = img.data[array_index(old_xy.y, old_xy.x, img.width) ];
            }
            int64_t real_x = x - border.w_left;
            int64_t real_y = y - border.h_bottom;

            index = array_index(real_y, real_x, width);
            result[index] = temp;

        }
    }


    struct image result_image = (struct image) {
            .width = width,
            .height = height,
            .data = result
    };
    //show_image(stdout, &img);
    //show_image(stdout, &result_image);
    return result_image;
}

// ---- WORK WITH MATRIX ----

int64_t get_max(int num, ...) {
    int64_t max = INT64_MIN, t;
    va_list argptr;

    va_start(argptr, num);
    for(; num; num--) {
        t = va_arg(argptr, int64_t);
        max = t > max ? t : max;
    }

    va_end(argptr);
    return max;
}

int64_t get_min(int num, ...) {
    int64_t min = INT64_MAX, t;
    va_list argptr;

    va_start(argptr, num);
    for(; num; num--) {
        t = va_arg(argptr, int64_t);
        min = t < min ? t : min;
    }

    va_end(argptr);
    return min;
}

double get_determine(struct kernel kernel) {
    if (kernel.width != 2 || kernel.height != 2) {
        print(stderr, "Determine is ZERO\nError suppressed in get_determine()");
        return 0;
    }
    return kernel.kernel[0] * kernel.kernel[3] - kernel.kernel[1] * kernel.kernel[2];
}

struct kernel get_inverse_kernel(const struct kernel kernel) {

    if (kernel.width != 2 || kernel.height != 2){
        print(stderr, "Kernel has wrong size\nError suppressed in get_inverse_kernel()");
        return (struct kernel) {0};
    }

    return (struct kernel) {
        .height = 2,
        .width = 2,
        .kernel = NULL
    };
}

struct point multiply_kernel_on_xy(struct kernel kernel, int64_t x, int64_t y) {
    if (kernel.width != 2 || kernel.height != 2){
        print(stderr, "Kernel has wrong size\nError suppressed in multiply_kernel_on_xy()");
        return (struct point) { 0 };
    }

    return (struct point) {
            .x = kernel.kernel[0] * x + kernel.kernel[1] * y,
            .y = kernel.kernel[2] * x + kernel.kernel[3] * y,
            .valuable = 1
    };
}

struct borders get_borders(struct kernel kernel, int64_t width, int64_t height) {
    struct point point0  = multiply_kernel_on_xy(kernel, 0, 0);
    struct point point1  = multiply_kernel_on_xy(kernel, width, 0);
    struct point point2  = multiply_kernel_on_xy(kernel, width, height);
    struct point point3  = multiply_kernel_on_xy(kernel, 0, height);

    return (struct borders) {
            .w_left = get_min(4, point0.x, point1.x, point2.x, point3.x),
            .w_right = get_max(4, point0.x, point1.x, point2.x, point3.x),
            .h_bottom = get_min(4, point0.y, point1.y, point2.y, point3.y),
            .h_upper = get_max(4, point0.y, point1.y, point2.y, point3.y)
    };
}

// ---- SHOW DETAILS ----

const char* write_out[3] = {"File has been written well.\n",
                            "Something went wrong with writing buffer\n",
                            "Something went wrong with writing header\n"};

void write_status_print(FILE* f, enum write_status ws) {
    fprintf(f, "%s", write_out[(size_t)ws]);
}

const char* read_out[4] = {"File has been read well.\n",
                           "Due to reading: Invalid signature.\n",
                           "Due to reading: Invalid bits.\n",
                           "Due to reading: Invalid header.\n"};

void read_status_print(FILE* f, enum read_status rs) {
    fprintf(f,"%s", read_out[(size_t)rs]);
}

void print(FILE* f, char* str) {
    fprintf(f,"%s\n", str);
}

void show_matrix(const struct kernel kernel) {
    printf("\n---- KERNEL ----\n");
    printf("  width: %" PRIu64 "  height:%" PRIu64 " \n", kernel.width, kernel.height);
    for(uint64_t y = 0; y < kernel.height; ++y) {
        for(uint64_t x = 0; x < kernel.width; ++x) {
            printf("%10lf", kernel.kernel[array_index(y, x, kernel.width)]);
        }
        printf("\n");
    }
}

void show_image(FILE* f, struct image const* img) {
    printf("\n---- IMAGE ----\n");
    printf("  width: %" PRIu64 "  height:%" PRIu64 " \n", img->width, img->height);
    for (size_t i = 0; i < img->height; ++i) {
        for (size_t j = 0; j < img->width; ++j ) {
            fprintf(f, "[%3d %3d %3d] ",
                    img->data[j + i * img->width].g,
                    img->data[j + i * img->width].g,
                    img->data[j + i * img->width].g);
        }
        printf("\n");
    }
}

void show_header(struct bmp_header const header) {
    printf("\nHeader: \n");
    printf("\tbiWidth: %d\n", header.biWidth);
    printf("\tbiHeight: %d\n", header.biHeight);
    printf("\tbiSizeImage: %d\n", header.biSizeImage);
    printf("\tPadding: %" PRIu32 " bytes.\n", get_padding(header.biWidth));
    printf("\tbiSize: %d\n", header.biSize);
    printf("\tbfType: %d\n", header.bfType);
    printf("\tbfileSize: %d\n", header.bfileSize);
    printf("\tbOffBits: %d\n", header.bOffBits);
}
