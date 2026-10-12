#ifndef ASP2_DZ3_IMAGE_H
#define ASP2_DZ3_IMAGE_H

#include <iostream>

struct Pixel {
    int index;
    char value;
};

class Image {
private:
    Pixel* pixels;
    int rows;
    int cols;

public:
    friend class ImageProcessor;

    Image(Pixel* pixels, const int rows, const int cols) {
        this->rows = rows;
        this->cols = cols;
        this->pixels = pixels;
    }

    ~Image() {
        delete[] pixels;
    }

    [[nodiscard]] int getElementCount() const {
        return rows * cols;
    }

    [[nodiscard]] int getElement(const int i) const {
        if (i >= 0 && i < rows * cols) {
            return pixels[i].index;
        }
        throw std::out_of_range("Index out of range.");
    }

    void swapElements(const int i, const int j) const {
        if (i != j && i >= 0 && j >= 0 && i < rows*cols && j < rows*cols) {
            const Pixel temp = pixels[i];
            pixels[i] = pixels[j];
            pixels[j] = temp;
        }
    }
};

#endif //ASP2_DZ3_IMAGE_H