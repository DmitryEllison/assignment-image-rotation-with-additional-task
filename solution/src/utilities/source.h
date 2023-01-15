#include  <stdint.h>
#include <stdio.h>
#include <inttypes.h>


struct __attribute__((packed)) bmp_header {
    uint16_t bfType;
    uint32_t bfileSize; // header + buffer
    uint32_t bfReserved;
    uint32_t bOffBits;
    uint32_t biSize; // 40
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage; // buffer in bytes
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
    WRITE_HEADER_ERROR = -666,
    WRITE_BUFFER_ERROR = -999
};

struct image {
    uint64_t width, height;
    struct pixel* data;
};


struct BMP {
    struct bmp_header header;
    struct image image;
    uint8_t *buffer;
    int32_t padding;
};

enum read_status from_bmp( FILE* in, struct BMP* img );

enum write_status to_bmp( FILE* out, struct BMP const* img );

struct image rotate( struct image const img );

void show_header(struct bmp_header header);

uint32_t get_padding(int32_t biWidth);

void buffer2image(struct BMP* bmp);

void image2buffer(struct BMP* bmp);

void read_status_print(enum read_status rs);

void write_status_print(enum write_status ws);

void update_header_and_padding(struct BMP* bmp);

struct BMP null_bmp();

void show_image(struct image const img);

void show_buffer(struct BMP const bmp);
