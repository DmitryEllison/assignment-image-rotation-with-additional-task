#include <inttypes.h>
#include <malloc.h>
#include <stdint.h>
#include <stdio.h>

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
    READ_INVALID_SIGNATURE = 1,
    READ_INVALID_BITS = 2,
    READ_INVALID_HEADER = 3
};


enum  write_status  {
    WRITE_OK = 0,
    WRITE_HEADER_ERROR = 1,
    WRITE_BUFFER_ERROR = 2
};

struct image {
    uint64_t width, height;
    struct pixel* data;
};

enum read_status from_bmp(FILE *in, struct image *img);

enum write_status to_bmp(FILE *out, struct image *img);

struct image rotate( struct image const img );
