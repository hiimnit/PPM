#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdint>
#include <unistd.h>
#include <time.h>
#include <cstdio>

using namespace std;

struct img {
    uint8_t * r;
    uint8_t * g;
    uint8_t * b;
    img(const int size) : current(0) {
        r = new uint8_t[size];
        g = new uint8_t[size];
        b = new uint8_t[size];
    }
    ~img() {
        delete[] r;
        delete[] g;
        delete[] b;
    }
    void add(const uint8_t x, const uint8_t y, const uint8_t z) {
        r[current] = x;
        g[current] = y;
        b[current++] = z;
    }
    int current;
};

img * image_in;
int histogram[5];

// Subinterval:	0 - 50 51 - 101 102 - 152 153 - 203 204 - 255
void add(const int x) {
    int i;
    if (x < 51) i = 0;
    else if (x < 102) i = 1;
    else if (x < 153) i = 2;
    else if (x < 204) i = 3;
    else  i = 4;
    ++(histogram[i]);
}

bool readFile(const char * in_file, int * __restrict__ s1, int * __restrict__ s2) {
    ifstream ifs(in_file, ifstream::in | ifstream::binary);

    FILE * in = fopen(in_file, "r");

    if(in == NULL) {
        cout << "reading err" << endl;
        return false;
    }

    string P6;
    int size;
    char buffer [100];

    fgets(buffer, 100, in); // prvni line "P6"
    fscanf(in, "%d", &*s1); // prvni rozmer
    fscanf(in, "%d", &*s2); // druhy rozmer
    fgets(buffer, 100, in); // 255
    fgets(buffer, 100, in); // 255

    size = *s1 * *s2;
    image_in = new img(size);

    uint8_t * buf = new uint8_t[3 * *s1];

    for (int i = 0; i < *s2; ++i) {
        fread(buf, 1, 3 * *s1, in);
        for (int j = 0; j < 3 * *s1; j += 3) image_in->add(buf[j], buf[j + 1], buf[j + 2]);
    }

    delete[] buf;
    fclose(in);
    return true;
}

void do_magic(const char * out_file, const char * out_hist, const int s1, const int s2) {
    FILE * out = fopen(out_file, "w");
    FILE * out2 = fopen(out_hist, "w");

    if(out == NULL || out2 == NULL) {
        cout << "writing err" << endl;
        return;
    }

    fputs("P6\n", out);
    fprintf(out, "%d\n", s1);
    fprintf(out, "%d\n", s2);
    fputs("255\n", out);

    int size = s1 * s2, r, g, b, x;

    uint8_t * buffer = new uint8_t[3 * s1];

    uint8_t r_l, r_c, r_n;
    uint8_t g_l, g_c, g_n;
    uint8_t b_l, b_c, b_n;

    histogram[0] = 0;
    histogram[1] = 0;
    histogram[2] = 0;
    histogram[3] = 0;
    histogram[4] = 0;

    // tyhle dva for vyhodit, pocitat round() na vypisu
    for (int i = 0, j = 0; i < s1; ++i, j += 3) {
        r = image_in->r[i];
        g = image_in->g[i];
        b = image_in->b[i];

        buffer[j] = r;
        buffer[j + 1] = g;
        buffer[j + 2] = b;

        x = round(0.2126 * r + 0.7152 * g + 0.0722 * b);
        add(x);
    }

    fwrite(buffer, 1, 3 * s1, out);

    // asi zbytecne komplikovany
    for (int i = 1; i < s2 - 1; ++i) {
        r = image_in->r[i * s1];
        g = image_in->g[i * s1];
        b = image_in->b[i * s1];

        buffer[0] = r;
        buffer[1] = g;
        buffer[2] = b;

        x = round(0.2126 * r + 0.7152 * g + 0.0722 * b);
        add(x);

        r_l = r;
        r_c = image_in->r[i * s1 + 1];
        r_n = image_in->r[i * s1 + 2];
        g_l = g;
        g_c = image_in->g[i * s1 + 1];
        g_n = image_in->g[i * s1 + 2];
        b_l = b;
        b_c = image_in->b[i * s1 + 1];
        b_n = image_in->b[i * s1 + 2];

        for (int j = i * s1 + 1, l = 3; j < i * s1 + s1 - 1; ++j, l += 3) {
            r = 5 * r_c - image_in->r[j - s1] - image_in->r[j + s1] - r_l - r_n;
            g = 5 * g_c - image_in->g[j - s1] - image_in->g[j + s1] - g_l - g_n;
            b = 5 * b_c - image_in->b[j - s1] - image_in->b[j + s1] - b_l - b_n;

            r = r > 0 ? r : 0;
            r = r > 255 ? 255 : r;
            g = g > 0 ? g : 0;
            g = g > 255 ? 255 : g;
            b = b > 0 ? b : 0;
            b = b > 255 ? 255 : b;

            buffer[l] = r;
            buffer[l + 1] = g;
            buffer[l + 2] = b;

            x = round(0.2126 * r + 0.7152 * g + 0.0722 * b);
            add(x);

            r_l = r_c;
            r_c = r_n;
            r_n = image_in->r[j + 2];
            g_l = g_c;
            g_c = g_n;
            g_n = image_in->g[j + 2];
            b_l = b_c;
            b_c = b_n;
            b_n = image_in->b[j + 2];
        }

        r = image_in->r[i * s1 + s1 - 1];
        g = image_in->g[i * s1 + s1 - 1];
        b = image_in->b[i * s1 + s1 - 1];

        buffer[3 * s1 - 3] = r;
        buffer[3 * s1 - 2] = g;
        buffer[3 * s1 - 1] = b;

        x = round(0.2126 * r + 0.7152 * g + 0.0722 * b);
        add(x);

        fwrite(buffer, 1, 3 * s1, out);
    }

    for (int i = size - s1, j = 0; i < size; ++i, j += 3) {
        r = image_in->r[i];
        g = image_in->g[i];
        b = image_in->b[i];

        buffer[j] = r;
        buffer[j + 1] = g;
        buffer[j + 2] = b;

        x = round(0.2126 * r + 0.7152 * g + 0.0722 * b);
        add(x);
    }

    fwrite(buffer, 1, 3 * s1, out);

    fprintf(out2, "%d %d %d %d %d ", histogram[0], histogram[1], histogram[2], histogram[3], histogram[4]);

    delete[] buffer;
    fclose(out);
    fclose(out2);
}

int main(int argc, char ** argv) {
    if(argc != 4) {
        cout << "usage : ./a.out in.file out.file hist.file" << endl;
        return 1;
    }

    int s1, s2;
    struct timespec start, stop;

    clock_gettime( CLOCK_REALTIME, &start);

    if(!readFile(argv[1], &s1, &s2)) return 2;
    cout << "PPM image " << s1 << "x" << s2 << endl;
    do_magic(argv[2], argv[3], s1, s2);

    clock_gettime( CLOCK_REALTIME, &stop);
    double accum = ( stop.tv_sec - start.tv_sec )*1000.0 + ( stop.tv_nsec - start.tv_nsec )/ 1000000.0;
    printf( "Time: %.6lf ms\n", accum );

    delete image_in;
    return 0;
}