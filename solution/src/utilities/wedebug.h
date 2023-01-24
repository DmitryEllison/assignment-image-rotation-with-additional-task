void read_status_print(FILE* f, enum read_status rs);

void write_status_print(FILE* f, enum write_status ws);

void static show_header(struct bmp_header const header);

void static show_image(FILE* f, struct image const* img);
