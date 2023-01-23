#include "source.h"
#include "transformation.h"
#include "wedebug.h"


uint32_t get_padding(uint32_t biWidth) {
    return 4 - (biWidth * 3) % 4;
}

struct bmp_header read_bmp_header(FILE* in) {
    struct bmp_header result = {0};
    if (fread(&result , sizeof(struct bmp_header), 1, in) == 1) {
        return result;
    }
}

// READ BMP FILE AND RETURN IMAGE
enum read_status from_bmp(FILE* in, struct image* img, struct bmp_header header ) {
    // Считали хедер
    if (fread(&(header), sizeof(struct bmp_header), 1, in) != 1) {
        return READ_INVALID_HEADER;
    }

    // Проверяем сигнатуру
    if (header.bfType != 0x4D42) {
        return READ_INVALID_SIGNATURE;
    }

    if (fseek(in, header.bOffBits, SEEK_SET) != 0) {
        return READ_INVALID_BITS;
    }

    size_t padding = get_padding(header.biWidth);
    header.biSizeImage = (3 * header.biWidth + padding) * header.biHeight;
    img->data =  malloc(sizeof(struct pixel) * header.biWidth * header.biHeight);

    for (size_t i = 0; i < header.biHeight; ++i) {
        for (size_t j = 0; j < header.biWidth; ++j) {
            img->data[i + j * header.biWidth] = (struct pixel) {
                    fgetc(in),
                    fgetc(in),
                    fgetc(in)
            };
        }
        for (size_t j = 0; j < padding; ++j)
            fgetc(in);
    }
    if (fgetc(in) != EOF) {
        fclose(in);
        return READ_INVALID_BITS;
    }

    return READ_OK;
}

void update_header(struct bmp_header* header, size_t image_width, size_t image_height) {
    header->bfileSize = sizeof(struct bmp_header) + (3 * image_width + get_padding(image_width)) * image_height;
    header->biWidth = image_width;
    header->biHeight = image_height;
    header->biSizeImage = (sizeof(struct pixel) * image_width + get_padding(image_width) ) * image_height;
}


enum write_status to_bmp(FILE* out, struct image* img, struct bmp_header header ) {
    if (fwrite(&header, sizeof(struct bmp_header), 1, out) != 1) {
        return WRITE_HEADER_ERROR;
    }
    update_header(&header, img->width, img->height);

    size_t padding = get_padding(header.biWidth);
    size_t index = 0;
    uint8_t zero = 0;

    for (size_t i = 0; i < header.biHeight; ++i) {
        for (size_t j = 0; j < header.biWidth; ++j) {
            index = j + i * header.biWidth;
            if (fwrite(&img->data[index].g, 1, 1, out) != 1 &&
                fwrite(&img->data[index].b, 1, 1, out) != 1 &&
                fwrite(&img->data[index].r, 1, 1, out))
                return WRITE_BUFFER_ERROR;
        }
        for (size_t j = 0; j < padding; ++j) {
            if (fwrite(&zero, 1, 1, out) != 1)
                return WRITE_BUFFER_ERROR;
        }
    }
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
    fprintf(f,"%s", read_out[(size_t)rs]);
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
    printf("\tPadding: %" PRIu32 " bytes.\n", get_padding(header.biWidth));
    printf("\tbiSize: %d\n", header.biSize);
    printf("\tbfType: %d\n", header.bfType);
    printf("\tbfileSize: %d\n", header.bfileSize);
}
