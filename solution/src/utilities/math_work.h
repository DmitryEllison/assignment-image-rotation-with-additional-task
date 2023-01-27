#include <inttypes.h>
#include <malloc.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
// actually this looks ugly but . . .

struct point {
    int64_t x, y;
    int valuable;
};

struct borders {
    int64_t w_left, w_right, h_upper, h_bottom;
};

double get_determine(struct kernel kernel);

struct kernel get_inverse_kernel(const struct kernel kernel);

struct point multiply_kernel_on_xy(struct kernel kernel, int64_t x, int64_t y);

struct borders get_borders(struct kernel kernel, int64_t width, int64_t height);

int64_t get_max(int num, ...);

int64_t get_min(int num, ...);

void show_matrix(const struct kernel kernel);

double* get_inverse_matrix();
