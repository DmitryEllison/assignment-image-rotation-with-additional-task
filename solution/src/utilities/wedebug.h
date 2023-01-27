void read_status_print(FILE* f, enum read_status rs);

void write_status_print(FILE* f, enum write_status ws);

void show_header(struct bmp_header const header);

void show_image(FILE* f, struct image const* img);

void print(FILE* f, char* str);
