#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <functional>
#include <thread>

using namespace std;

struct Color {
    float r, g, b, a;
    string printVal(float f) {
        if (f > 0.75) return "#";
        if (f > 0.5) return ";";
        if (f > 0.25) return ".";
        return " ";
    }
    string print() {
        return printVal(r) + printVal(g) + printVal(b);
    }
};

class Image {
    vector<Color> data;
    int width;
    int height;
    //void Load
public:
    Image(int _width = 0, int _height = 0) :width(_width), height(_height) {
        data.resize(width * height);
    }
    Color& operator()(size_t x, size_t y) {
        return data[x + y * width];
        // 0,0 => 0
        // x:0, y:1 => 0 + 1 * 20 => 20
        // x:1, y:1 => 1 + 1 * 20 => 21
    }
    Color operator()(size_t x, size_t y) const {
        return data[x + y * width];
    }

    int GetWidth() const { return width; }
    int GetHeight() const { return height; }

    void print() {
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                cout << this->operator()(x, y).print();
            }
            cout << endl;
        }
    }
};

// interface
class image_data_provider {
public:
    virtual Image load_image(const std::string& path) const = 0;
    virtual void save_image(const std::string& path, const Image& img) const = 0;
};

class image_data_provider_factory : public image_data_provider {
public:
    Image load_image(const std::string& path) const override {
        ifstream plik(path); // 
        if (!plik) {
            cout << "Brak pliku:" << path << endl;
            return Image();
        }
        string tmp;
        getline(plik, tmp); // P3
        getline(plik, tmp); // # Created by GIMP version 2.10.32 PNM plug-in

        int width, height;

        plik >> width >> height;
        Image obrazek(width, height);

        int colorMaximum;
        plik >> colorMaximum;


        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int red, green, blue;
                plik >> red; // red => 255
                obrazek(x, y).r = (float)red / colorMaximum;
                plik >> green;
                obrazek(x, y).g = (float)green / colorMaximum;
                plik >> blue;
                obrazek(x, y).b = (float)blue / colorMaximum;
            }
        }

        return obrazek;
    }
    void save_image(const std::string& path, const Image& img) const override {
        ofstream zap(path);
        if (!zap.is_open()) {
            std::cout << "nie udao sie otworzyc pliku";
        }
        zap << "P3" << endl;
        zap << "# Created by GIMP version 2.10.32 PNM plug-in" << endl;
        zap << img.GetHeight() << " " << img.GetWidth() << std::endl;
        const int maxColor = 255;
        zap << maxColor << std::endl;

        for (int y = 0; y < img.GetHeight(); y++) {
            for (int x = 0; x < img.GetWidth(); x++) {
                zap << maxColor * img(x, y).r << endl;
                zap << maxColor * img(x, y).g << endl;
                zap << maxColor * img(x, y).b << endl;
            }
        }

        std::cout << "test" << std::endl;
    }
};

image_data_provider_factory fabryka;

void blendRegion(Image& output, const Image& imgA, const Image& imgB,
    std::function<Color(Color, Color)> func, int minX, int minY, int maxX, int maxY) {

    for (auto y = minY; y < maxY; y++) {
        for (auto x = minX; x < maxX; x++) {
            auto colorA = imgA(x, y);
            auto colorB = imgB(x, y);
            output(x, y) = func(colorA, colorB);
        }
    }
}

Color blendMultiply(Color a, Color b) {
    Color res;
    res.r = a.r * b.r;
    res.g = a.g * b.g;
    res.b = a.b * b.b;
    return res;
}

Image blend_images(const Image& imgA, const Image& imgB, int numThreads,
    std::function<Color(Color, Color)>func) {
    auto outWidth = imgA.GetWidth();
    auto outHeight = imgA.GetHeight();

    // TODO: znalezienie obszaru wynikowego

    Image outImg(outWidth, outHeight);

    vector<thread> jobs;
    for (int i = 0; i < numThreads; i++) {
        jobs.push_back(std::thread(
            blendRegion,
            std::ref(outImg),
            std::ref(imgA),
            std::ref(imgB),
            func,
            0, 0, outWidth, outHeight));//TO DO:: poprawi uzaleznic od i
    }
std:cout << "Czekam na laczenie watkow" << endl;
    for (int i = 0; i < numThreads; i++) {
        jobs[i].join();
        std::cout << "Udalo sie polaczyc watki" << endl;
    }
    return outImg;
}

int main()
{
    Image wczytany1 = fabryka.load_image("C:\\Users\\xgrj78\\Desktop\\obrazek1.ppm");
    Image wczytany2 = fabryka.load_image("C:\\Users\\xgrj78\\Desktop\\obrazek2.ppm");
    /* wczytany.print();
      Image obrazek(20, 20);
      obrazek(0, 0).r = 1;
      obrazek(1, 0).r = 1;
      obrazek(2, 0).r = 1;
      obrazek(3, 0).r = 1;
      obrazek(4, 0).r = 1;
      obrazek(2, 3).g = 0.5;
      obrazek(2, 3).b = 1;
      obrazek.print();
    */
    Image res = blend_images(wczytany1, wczytany2, 1, blendMultiply);
    fabryka.save_image("C:\\Users\\xgrj78\\Desktop\\obrazek3.ppm", res);
}