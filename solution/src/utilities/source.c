#include <malloc.h>
#include "source.h"


int32_t get_padding(int32_t biWidth) {
    return 4 - (biWidth * 3) % 4;
}

void show_header(struct bmp_header header) {
    printf("\nHeader: \n");
    printf("\tbiWidth: %d\n", header.biWidth);
    printf("\tbiHeight: %d\n", header.biHeight);
    printf("\tbiSizeImage: %d\n", header.biSizeImage);
    printf("\tPadding: %d bytes.\n", get_padding(header.biWidth));
    printf("\tbiSize: %d\n", header.biSize);
    printf("\tbfType: %d\n", header.bfType);
    printf("\tbfileSize: %d\n", header.bfileSize);
}

enum read_status from_bmp( FILE* in, struct BMP* bmp ) {
    // Считали хедер
    if (fread(&(bmp->header), sizeof(struct bmp_header), 1, in) != 1) {
        return READ_INVALID_HEADER;
    }

    show_header(bmp->header);
    // Проверяем сигнатуру
    if (bmp->header.bfType != 0x4D42) {
        return READ_INVALID_SIGNATURE;
    }

    if (fseek(in, bmp->header.bOffBits, SEEK_SET) != 0) {
        return READ_INVALID_BITS;
    }

    bmp->buffer = malloc(bmp->header.biSizeImage);
    bmp->padding = get_padding(bmp->header.biWidth);
    if (fread(bmp->buffer, bmp->header.biSizeImage, 1, in) != 1) {
        free(bmp->buffer);
        return READ_INVALID_BITS;
    }

    bmp->image->width = bmp->header.biWidth;
    bmp->image->height = bmp->header.biHeight;

    // show_header(bmp->header);
    rewind(in);
    return READ_OK;
}

enum write_status to_bmp( FILE* out, struct BMP const* bmp ) {
    if (fwrite(&bmp->header, sizeof(struct bmp_header), 1, out) != 0) {
        free(bmp->image);
        return WRITE_HEADER_ERROR;
    }
    if (fwrite(bmp->buffer, bmp->header.biSizeImage, 1, out) == 0) {
        free(bmp->image);
        return WRITE_BUFFER_ERROR;
    }

    free(bmp->buffer);
    rewind(out);
    return WRITE_OK;
}

void rotate( struct image* img ) {
    struct pixel* temp = malloc(sizeof(struct pixel) * img->width * img->height);
    for (uint64_t i = 0; i < img->height; i++) {
        for (uint64_t j = 0; j < img->width; j++) {
            temp[j*img->height + (img->height - (i + 1)) ] = img->data[j + i * img->width];
        }
    }
    img->data = temp;
}

void buffer2image(struct BMP bmp) {
    struct pixel* temp = malloc(sizeof(struct pixel) * bmp.header.biWidth * bmp.header.biHeight);
    for (size_t i = 0; i < bmp.image->height; ++i) {
        for (size_t j = 0; j < bmp.image->width; ++j) {
            size_t index = 3 * bmp.image->width * i + 3 * j;
            temp[i] = (struct pixel) {
                bmp.buffer[index + 0],
                bmp.buffer[index + 1],
                bmp.buffer[index + 2]
            };
        }
        i += bmp.padding;
    }
    bmp.image->data = temp;
    free(bmp.buffer);
    bmp.padding = 0;
}

void update_header_and_padding(struct BMP bmp) {
    bmp.padding = get_padding(bmp.image->width);
    bmp.header.biWidth = bmp.image->width;
    bmp.header.biHeight = bmp.image->height;
    bmp.header.biSizeImage = (sizeof(struct pixel) * bmp.header.biWidth + bmp.padding ) * bmp.header.biHeight;
}

void image2buffer(struct BMP bmp) {
    bmp.buffer = malloc(bmp.header.biSizeImage);
    for (size_t i = 0; i < bmp.image->height; ++i) {
        for (size_t j = 0; j < bmp.image->width; ++j) {
            bmp.buffer[i * bmp.image->height + 3 * j] = bmp.image->data->b;
            bmp.buffer[i * bmp.image->height + 3 * j + 1] = bmp.image->data->g;
            bmp.buffer[i * bmp.image->height + 3 * j + 2] = bmp.image->data->r;
        }
        i += bmp.padding;
    }
}

void write_status_print(enum write_status ws) {
    switch (ws) {
        case WRITE_OK: {
            printf("\nFile has been written well.");
            break;
        }
        case WRITE_HEADER_ERROR: {
            printf("\nSomething went wrong with writing header.");
            break;
        }
        case WRITE_BUFFER_ERROR: {
            printf("\nSomething went wrong with writing buffer.");
            break;
        }
        default: {
            printf("Something went wrong with writing changed image in the file");
        }
    }
}

void read_status_print(enum read_status rs) {
    switch (rs) {
        case READ_OK: {
            printf("\nFile has been read well.");
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
            printf("Something went wrong with reading of file");
        }
    }
}

struct BMP null_bmp() {
    return (struct BMP) {
            .image = &(struct image) {
                    .height = 0,
                    .width = 0,
                    .data = NULL
            },
            .header = NULL,
            .buffer = NULL,
            .padding = 0
    };
}

