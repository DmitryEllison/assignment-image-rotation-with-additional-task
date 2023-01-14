#include  <stdint.h>
#include <stdio.h>
#include <inttypes.h>

struct __attribute__((packed)) bmp_header {
    uint16_t bfType;
    uint32_t bfileSize; // размер файла
    uint32_t bfReserved;
    uint32_t bOffBits;
    uint32_t biSize; // 40
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage; // в байтах
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
};

struct pixel {
    uint8_t b, g, r;
};

enum read_status  {
    READ_OK = 0,
    READ_INVALID_SIGNATURE = -3,
    READ_INVALID_BITS = -4,
    READ_INVALID_HEADER = -5
};

enum  write_status  {
    WRITE_OK = 0,
    WRITE_ERROR = -666
};

struct image {
    uint64_t width, height;
    struct pixel* data;
};


struct BMP {
    struct bmp_header header;
    struct image *image;
    uint8_t *buffer;
    int32_t padding;
};

enum read_status from_bmp( FILE* in, struct BMP* img );

enum write_status to_bmp( FILE* out, struct BMP const* img );

void rotate( struct BMP bmp);

void show_header(struct bmp_header header);

int32_t get_padding(int32_t biWidth);

void delete_padding_and_fill_image(struct BMP bmp);

void add_padding_and_fill_buffer(struct BMP bmp);
