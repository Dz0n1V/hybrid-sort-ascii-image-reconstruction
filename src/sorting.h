#ifndef ASP2_DZ3_SORTING_H
#define ASP2_DZ3_SORTING_H

#include "image.h"

enum SortingDirection {
    ASCENDING,
    DESCENDING
};

class SortingAlgorithm {
public:
    virtual ~SortingAlgorithm() = default;
    virtual void sort(Image* image, SortingDirection direction) = 0;
};

class ExampleSort final : public SortingAlgorithm {
public:
    void sort(Image* image, SortingDirection direction) override {
        const int count = image->getElementCount();

        for (int i = 0; i < count; i++) {
            for (int j = i + 1; j < count; j++) {
                const int el1 = image->getElement(i);
                const int el2 = image->getElement(j);

                if (direction == ASCENDING && el1 > el2 || direction == DESCENDING && el1 < el2)
                    image->swapElements(i, j);
            }
        }
    }
};

#endif //ASP2_DZ3_SORTING_H