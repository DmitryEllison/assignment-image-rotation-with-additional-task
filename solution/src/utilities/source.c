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


double get_determine(struct kernel kernel) {
    if (kernel.width != 2 || kernel.height != 2) return 0;
    return kernel.kernel[0] * kernel.kernel[3] - kernel.kernel[1] * kernel.kernel[2];
}

struct kernel get_inverse_kernel(struct kernel kernel) {
    if (kernel.width != 2 || kernel.height != 2)
        return (struct kernel) { 0 };

    double det = get_determine(kernel);
    return (struct kernel) {
            .height = 2,
            .width = 2,
            .kernel = (double[]){ (double) kernel.kernel[3] / det, (double) kernel.kernel[2] / (-det),
                                  (double) kernel.kernel[1] / (-det), (double) kernel.kernel[0] / det}
    };
}

struct point {
    uint64_t x, y;
    int valuable;
};

struct point multiply_kernel_on_xy(struct kernel kernel, uint64_t x, uint64_t y) {
    if (kernel.width != 2 || kernel.height != 2)
        return (struct point) { 0 };

    return (struct point) {
        .x = kernel.kernel[0] * x + kernel.kernel[1] * y,
        .y = kernel.kernel[2] * x + kernel.kernel[3] * y,
        .valuable = 1
    };
}

struct image matrix_transformation(const struct image img, struct kernel kernel) {
    struct pixel* result = malloc(sizeof(struct pixel) * img.width * img.height);
    struct kernel inverse_kernel = get_inverse_kernel(kernel);



    return (struct image) {
            .width = img.width,
            .height = img.height,
            .data = result
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

void show_image(FILE* f, struct image const* img) {
    for (size_t i = 0; i < img->height; ++i) {
        for (size_t j = 0; j < img->width; ++j ) {
            fprintf(f, "[%d %d %d] ",
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
