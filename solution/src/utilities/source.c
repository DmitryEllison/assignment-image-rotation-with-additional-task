#include "source.h"

uint32_t get_padding(uint32_t biWidth) {
    return 4 - (biWidth * 3) % 4;
}

enum read_status from_bmp( FILE* in, struct BMP* bmp ) {
    // Считали хедер
    if (fread(&(bmp->header), sizeof(struct bmp_header), 1, in) != 1) {
        return READ_INVALID_HEADER;
    }

    // Проверяем сигнатуру
    if (bmp->header.bfType != 0x4D42) {
        return READ_INVALID_SIGNATURE;
    }

    if (fseek(in, bmp->header.bOffBits, SEEK_SET) != 0) {
        return READ_INVALID_BITS;
    }

    bmp->padding = get_padding(bmp->header.biWidth);
    bmp->header.biSizeImage = (3 * bmp->header.biWidth + bmp->padding) * bmp->header.biHeight;
    bmp->buffer = malloc(bmp->header.biSizeImage);

    if (fread(bmp->buffer, bmp->header.biSizeImage, 1, in) != 1) {
        free(bmp->buffer);
        return READ_INVALID_BITS;
    }

    fclose(in);
    return READ_OK;
}

enum write_status to_bmp( FILE* out, struct BMP* bmp ) {
    if (fwrite(&bmp->header, sizeof(struct bmp_header), 1, out) != 1) {
        return WRITE_HEADER_ERROR;
    }

    if (fwrite(bmp->buffer, bmp->header.biSizeImage, 1, out) == 0) {
        free(bmp->buffer);
        return WRITE_BUFFER_ERROR;
    }

    free(bmp->buffer);
    fclose(out);
    return WRITE_OK;
}

struct image rotate(struct image const img) {
    uint64_t width = img.width;
    uint64_t height = img.height;
    struct pixel* temp = malloc(sizeof(struct pixel) * width * height);
    for (uint64_t i = 0; i < height; i++) {
        for (uint64_t j = 0; j < width; j++) {
            temp[j*height + height - (i + 1) ] = img.data[j + i * width];
        }
    }
    free(img.data);
    return (struct image)
            {
                    .width = height,
                    .height = width,
                    .data = temp
            };
}

void buffer2image(struct BMP* bmp) {
    bmp->image.data = malloc(sizeof(struct pixel) * bmp->header.biWidth * bmp->header.biHeight);
    for (size_t i = 0; i < bmp->header.biHeight; ++i) {
        for (size_t j = 0; j < bmp->header.biWidth; ++j) {
            size_t index = 3 * (bmp->header.biWidth * i + j) + i * bmp->padding;
            bmp->image.data[j + i * bmp->header.biWidth] = (struct pixel) {
                bmp->buffer[index + 0],
                bmp->buffer[index + 1],
                bmp->buffer[index + 2]
            };
        }
    }
    bmp->image.width = bmp->header.biWidth;
    bmp->image.height = bmp->header.biHeight;
    free(bmp->buffer);
    bmp->padding = 0;
}

void update_header_and_padding(struct BMP* bmp) {
    bmp->padding = get_padding(bmp->image.width);
    bmp->header.bfileSize = sizeof(struct bmp_header) + (3 * bmp->image.width + bmp->padding) * bmp->image.height;
    bmp->header.biWidth = bmp->image.width;
    bmp->header.biHeight = bmp->image.height;
    bmp->header.biSizeImage = (sizeof(struct pixel) * bmp->header.biWidth + bmp->padding ) * bmp->header.biHeight;
}

void image2buffer(struct BMP* bmp) {
    bmp->buffer = malloc(bmp->header.biSizeImage);
    for (uint32_t i = 0; i < bmp->header.biSizeImage; ++i) {
        bmp->buffer[i] = 0;
    }
    size_t index = 0;
    for (size_t i = 0; i < bmp->image.height; ++i) {
        for (size_t j = 0; j < bmp->image.width; ++j) {
            index = bmp->header.biWidth * i + j;
            bmp->buffer[3 * index + 0 + i * bmp->padding] = bmp->image.data[index].b;
            bmp->buffer[3 * index + 1 + i * bmp->padding] = bmp->image.data[index].g;
            bmp->buffer[3 * index + 2 + i * bmp->padding] = bmp->image.data[index].r;
        }
    }
    free(bmp->image.data);
}

const char* write_out[3] = {"File has been written well.\n",
                            "Something went wrong with writing buffer\n",
                            "Something went wrong with writing header\n"};

void write_status_print(FILE* f, enum write_status ws) {
    fprintf(f, "%s", write_out[(size_t)ws]);
}

const char* read_out[4] = {"File has been read well.\n",
                           "Invalid bits.\n",
                           "Invalid source header.\n",
                           "Invalid signature.\n"};

void read_status_print(FILE* f, enum read_status rs) {
    fprintf(f,read_out[(size_t)rs]);
}

struct bmp_header fill_header(uint32_t biSizeImage, uint32_t width, uint32_t height) {
    struct bmp_header temp = {0};

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

void show_header(struct bmp_header const header) {
    printf("\nHeader: \n");
    printf("\tbiWidth: %d\n", header.biWidth);
    printf("\tbiHeight: %d\n", header.biHeight);
    printf("\tbiSizeImage: %d\n", header.biSizeImage);
    printf("\tPadding: %" PRIu32 "bytes.\n", get_padding(header.biWidth));
    printf("\tbiSize: %d\n", header.biSize);
    printf("\tbfType: %d\n", header.bfType);
    printf("\tbfileSize: %d\n", header.bfileSize);
}
