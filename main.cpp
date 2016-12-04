#include <iostream>
#include <fstream>

using namespace std;

struct img {
    unsigned char * r;
    unsigned char * g;
    unsigned char * b;
    img(const int size) : size(size), current(0) {
        r = new unsigned char[size];
        g = new unsigned char[size];
        b = new unsigned char[size];
    }
    ~img() {
        delete[] r;
        delete[] g;
        delete[] b;
    }
    void add(const unsigned char x, const unsigned char y, const unsigned char z) {
        r[current] = x;
        g[current] = y;
        b[current++] = z;
    }
    int size, current;
};

img * image_in;
img * image_out;

void readFile(const char * in_file, int * __restrict__ s1, int * __restrict__ s2) {
    ifstream ifs(in_file, ifstream::in | ifstream::binary);

    if(!ifs.is_open()) return;

    string P6;
    int s255, size;

    ifs >> P6;      // prvni line "P6"
    ifs >> * s1;    // prvni rozmer
    ifs >> * s2;    // druhy rozmer
    ifs >> s255;    // cislo "255"
    ifs.get();      // nacteni newline

    size = *s1 * *s2;
    image_in = new img(size);

    unsigned char r, g, b;

    for (int i = 0; i < size; ++i) {
        r = ifs.get();
        g = ifs.get();
        b = ifs.get();
        image_in->add(r, g, b);
    }

    ifs.close();
}

void writeFile(const char * out_file, const int s1, const int s2) {
    ofstream ofs(out_file, ofstream::out | ofstream::binary);

    if(!ofs.is_open()) return;

    ofs << "P6" << endl;
    ofs << s1 << endl;
    ofs << s2 << endl;
    ofs << "255" << endl;

    int size = s1 * s2;

    // cela prvni a posledni rada by sla zapsat ze vstupu
    for (int i = 0; i < s2; ++i) {
        // prvni pixel ze vstupniho souboru
        ofs.put(image_in->r[i * s1]);
        ofs.put(image_in->g[i * s1]);
        ofs.put(image_in->b[i * s1]);
        for (int j = i * s1 + 1; j < i * s1 + s1 - 1; ++j) {
            ofs.put(image_out->r[j]);
            ofs.put(image_out->g[j]);
            ofs.put(image_out->b[j]);
        }
        // posledni pixel ze vstupniho souboru
        ofs.put(image_in->r[i * s1 + s1 - 1]);
        ofs.put(image_in->g[i * s1 + s1 - 1]);
        ofs.put(image_in->b[i * s1 + s1 - 1]);
    }

    ofs.close();
}

void do_magic(const int s1, const int s2) {
    int size = s1 * s2;
    image_out = new img(size);

    int r, g, b;

    for (int i = 0; i < s1; ++i) image_out->add(image_in->r[i], image_in->g[i], image_in->b[i]);

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

    for (int i = size - s1; i < size; ++i) image_out->add(image_in->r[i], image_in->g[i], image_in->b[i]);
}

int main(int argc, char ** argv) {
    if(argc != 3) {
        cout << "usage : ./a.out in.file out.file" << endl;
        return 1;
    }
    int s1, s2;

    readFile(argv[1], &s1, &s2);
    cout << "PPM image " << s1 << "x" << s2 << endl;
    do_magic(s1, s2);
    writeFile(argv[2], s1, s2);
    return 0;
}