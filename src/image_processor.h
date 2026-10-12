#ifndef ASP2_DZ3_IMAGE_PROCESSOR_H
#define ASP2_DZ3_IMAGE_PROCESSOR_H

#include <iostream>
#include <fstream>
#include "image.h"
#include "sorting.h"

using namespace std;

class ImageProcessor {
private:
    static Image* loadPixels(const char* filename) {
        ifstream file(filename);
        if (!file.is_open()) return nullptr;

        int rows, cols;
        file >> rows;
        file >> cols;

        const int pixelCount = rows * cols;
        auto* pixels = new Pixel[pixelCount];

        for (int i = 0; i < pixelCount; i++) {
            std::string line;
            file >> line;

            size_t commaPos = line.find(',');
            pixels[i].index = stoi(line.substr(0, commaPos));
            pixels[i].value = line[commaPos + 1];
        }

        auto* image = new Image(pixels, rows, cols);

        file.close();
        return image;
    }

    static bool printImage(const char* filename, const Image* image) {
        ofstream file(filename);
        if (!file.is_open()) return false;

        const int rows = image->rows;
        const int columns = image->cols;

        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < columns; j++) {
                file << image->pixels[columns*i+j].value;
            }
            file << endl;
        }

        file.close();
        return true;
    }

public:
    static void reconstructImage(const char* in_filename, const char* out_filename, SortingAlgorithm* algorithm) {
        Image *image = loadPixels(in_filename);

        if (!image) {
            cout << "Error loading image." << endl;
            return;
        }

        algorithm->sort(image, ASCENDING);
        const bool status = printImage(out_filename, image);

        if (!status) cout << "Error reconstructing image." << endl;
        else cout << "Successfully reconstructed image." << endl;
    }
};

#endif //ASP2_DZ3_IMAGE_PROCESSOR_H