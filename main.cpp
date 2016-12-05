#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdint>
#include <unistd.h>
#include <time.h>

using namespace std;

struct img {
    // uint8_t
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
img * image_out;
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

    if(!ifs.is_open()) {
        cout << "reading err" << endl;
        return false;
    }

    string P6;
    int s255, size;

    ifs >> P6;      // prvni line "P6"
    ifs >> * s1;    // prvni rozmer
    ifs >> * s2;    // druhy rozmer
    ifs >> s255;    // cislo "255"
    ifs.get();      // nacteni newline

    size = *s1 * *s2;
    image_in = new img(size);

    uint8_t r, g, b;

    for (int i = 0; i < size; ++i) {
        r = ifs.get();
        g = ifs.get();
        b = ifs.get();
        image_in->add(r, g, b);
    }

    ifs.close();
    return true;
}

void writeFile(const char * out_file, const char * out_hist, const int s1, const int s2) {
    ofstream ofs(out_file, ofstream::out | ofstream::binary);
    ofstream ofs2(out_hist, ofstream::out);

    if(!ofs.is_open() || !ofs2.is_open()) {
        cout << "writing err" << endl;
        return;
    }

    ofs << "P6" << endl;
    ofs << s1 << endl;
    ofs << s2 << endl;
    ofs << "255" << endl;

    int x;

    uint8_t r, g, b;

    // cela prvni a posledni rada by sla zapsat ze vstupu
    for (int i = 0; i < s2; ++i) {
        // prvni pixel ze vstupniho souboru
        r = image_in->r[i * s1];
        g = image_in->g[i * s1];
        b = image_in->b[i * s1];

        ofs.put(r);
        ofs.put(g);
        ofs.put(b);

        x = round(0.2126 * r + 0.7152 * g + 0.0722 * b);
        add(x);

        for (int j = i * s1 + 1; j < i * s1 + s1 - 1; ++j) {
            r = image_out->r[j];
            g = image_out->g[j];
            b = image_out->b[j];

            ofs.put(r);
            ofs.put(g);
            ofs.put(b);

            x = round(0.2126 * r + 0.7152 * g + 0.0722 * b);
            add(x);
        }
        // posledni pixel ze vstupniho souboru
        r = image_in->r[i * s1 + s1 - 1];
        g = image_in->g[i * s1 + s1 - 1];
        b = image_in->b[i * s1 + s1 - 1];

        ofs.put(r);
        ofs.put(g);
        ofs.put(b);

        x = round(0.2126 * r + 0.7152 * g + 0.0722 * b);
        add(x);
    }

    ofs2 << histogram[0] << ' ' << histogram[1] << ' ' << histogram[2] << ' ' << histogram[3] << ' ' << histogram[4] << ' ';

    ofs2.close();
    ofs.close();
}

void do_magic(const int s1, const int s2) {
    int size = s1 * s2;
    image_out = new img(size);

    int r, g, b;

    histogram[0] = 0;
    histogram[1] = 0;
    histogram[2] = 0;
    histogram[3] = 0;
    histogram[4] = 0;

    // tyhle dva for vyhodit, pocitat round() na vypisu
    for (int i = 0; i < s1; ++i) {
        r = image_in->r[i];
        g = image_in->g[i];
        b = image_in->b[i];

        image_out->add(r, g, b);
    }

    // asi zbytecne komplikovany
    for (int i = s1; i < size - s1; ++i) {
        r = 5 * image_in->r[i] - image_in->r[i - s1] - image_in->r[i + s1] - image_in->r[i - 1] - image_in->r[i + 1];
        r = r > 0 ? r : 0;
        r = r > 255 ? 255 : r;

        g = 5 * image_in->g[i] - image_in->g[i - s1] - image_in->g[i + s1] - image_in->g[i - 1] - image_in->g[i + 1];
        g = g > 0 ? g : 0;
        g = g > 255 ? 255 : g;

        b = 5 * image_in->b[i] - image_in->b[i - s1] - image_in->b[i + s1] - image_in->b[i - 1] - image_in->b[i + 1];
        b = b > 0 ? b : 0;
        b = b > 255 ? 255 : b;

        image_out->add(r, g, b);
    }

    for (int i = size - s1; i < size; ++i) {
        r = image_in->r[i];
        g = image_in->g[i];
        b = image_in->b[i];

        image_out->add(r, g, b);
    }
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
    do_magic(s1, s2);
    writeFile(argv[2], argv[3], s1, s2);

    clock_gettime( CLOCK_REALTIME, &stop);
    double accum = ( stop.tv_sec - start.tv_sec )*1000.0 + ( stop.tv_nsec - start.tv_nsec )/ 1000000.0;
    printf( "Time: %.6lf ms\n", accum );

    delete image_in;
    delete image_out;
    return 0;
}