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
    return WRITE_OK;
}

void rotate( struct BMP bmp ) {


}

void delete_padding_and_fill_image(struct BMP bmp) {
    struct image* temp = malloc(sizeof(struct pixel) * bmp.header.biWidth * bmp.header.biHeight);
    for (size_t i = 0; i < bmp.image->height; ++i) {
        for (size_t j = 0; j < bmp.image->width; ++j) {
            size_t index = 3 * bmp.image->width * i + 3 * j;
            temp->data[i] = (struct pixel) {
                bmp.buffer[index + 2],
                bmp.buffer[index + 1],
                bmp.buffer[index + 0]
            };
        }
        i += bmp.padding;
    }
    bmp.image = temp;
    free(bmp.buffer);
    bmp.padding = 0;
}

void add_padding_and_fill_buffer(struct BMP bmp) {

}
