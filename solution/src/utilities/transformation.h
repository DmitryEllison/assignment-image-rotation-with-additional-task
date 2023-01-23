uint32_t get_padding(uint32_t biWidth);

void buffer2image(struct BMP* bmp);

void image2buffer(struct BMP* bmp);

void update_header_and_padding(struct BMP* bmp);

