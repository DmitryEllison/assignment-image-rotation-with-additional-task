#include  <stdint.h>
#include <stdio.h>

struct bmp_header {
    uint16_t bfType;
    uint32_t bfileSize;
    uint32_t bfReserved;
    uint32_t bOffBits;
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t  biClrImportant;
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
    WRITE_ERROR = -6
};

struct image {
    uint64_t width, height;
    struct pixel* data;
};


struct BMP {
    struct bmp_header header;
    struct image source_image;
    struct image changed_image;
};

enum read_status from_bmp( FILE* in, struct image* img );

enum write_status to_bmp( FILE* out, struct image const* img );

struct image rotate( struct image const source );
