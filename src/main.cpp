#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <cmath>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <map>
#include <cstdlib>
#include <ctime>

#include "image.h"
#include "image_processor.h"
#include "sorting.h"
#include "utils.h"

using namespace std;

// Returns true if the two indices are out of order for the given direction
bool compareIndices(int index1, int index2, SortingDirection direction) {
    if (direction == ASCENDING) {
        return index1 > index2;
    } else {
        return index1 < index2;
    }
}


// Hybrid algorithm: Merge Sort + Insertion Sort
// Partitions with k elements or fewer are sorted with insertion sort
class HybridMergeInsertion : public SortingAlgorithm {
private:
    int k;

    void insertionSort(Image* image, int left, int right, SortingDirection direction) {
        for (int i = left + 1; i <= right; i++) {
            for (int j = i; j > left; j--) {
                int currentIndex = image->getElement(j);
                int previousIndex = image->getElement(j - 1);

                if (compareIndices(previousIndex, currentIndex, direction)) {
                    image->swapElements(j, j - 1);
                } else {
                    break;
                }
            }
        }
    }

    void inPlaceMerge(Image* image, int left, int mid, int right, SortingDirection direction) {
        int newStart = mid + 1;

        if (!compareIndices(image->getElement(mid), image->getElement(newStart), direction)) {
            return;
        }

        while (left <= mid && newStart <= right) {
            int leftIndex = image->getElement(left);
            int newStartIndex = image->getElement(newStart);

            if (!compareIndices(leftIndex, newStartIndex, direction)) {
                left++;
            } else {
                int index = newStart;
                while (index != left) {
                    image->swapElements(index, index - 1);
                    index--;
                }
                left++;
                mid++;
                newStart++;
            }
        }
    }

public:
    HybridMergeInsertion(int thresholdK) : k(thresholdK) {}

    void sort(Image* image, SortingDirection direction) override {
        if (!image) return;
        int n = image->getElementCount();
        if (n <= 1) return;

        struct State {
            int left;
            int right;
            bool isMergeStep;
        };

        stack<State> callStack;
        callStack.push({0, n - 1, false});

        while (!callStack.empty()) {
            State current = callStack.top();
            callStack.pop();

            int left = current.left;
            int right = current.right;

            if (current.isMergeStep) {
                int mid = left + (right - left) / 2;
                inPlaceMerge(image, left, mid, right, direction);
            } else {
                if (right - left + 1 <= k) {
                    insertionSort(image, left, right, direction);
                } else {
                    int mid = left + (right - left) / 2;
                    callStack.push({left, right, true});
                    callStack.push({mid + 1, right, false});
                    callStack.push({left, mid, false});
                }
            }
        }
    }
};


// Hybrid algorithm: Quick Sort + Bubble Sort
// Quick sort is used until the split depth reaches c * log2(n), then bubble sort
class HybridQuickBubble : public SortingAlgorithm {
private:
    double c;

    void bubbleSort(Image* image, int left, int right, SortingDirection direction) {
        for (int i = 0; i < right - left; i++) {
            bool swapped = false;
            for (int j = left; j < right - i; j++) {
                int index1 = image->getElement(j);
                int index2 = image->getElement(j + 1);

                if (compareIndices(index1, index2, direction)) {
                    image->swapElements(j, j + 1);
                    swapped = true;
                }
            }
            if (!swapped) break;
        }
    }

    // Hoare-style partitioning with a random pivot
    int quickSort(Image* image, int low, int high, SortingDirection direction) {
        int range = high - low + 1;
        int randomOffset = rand() % range;
        int randomPivotIndex = low + randomOffset;

        image->swapElements(low, randomPivotIndex);

        int pivotVal = image->getElement(low);

        int i = low + 1;
        int j = high;

        while (true) {
            while (i <= high && !compareIndices(image->getElement(i), pivotVal, direction)) {
                i++;
            }

            while (j >= low + 1 && compareIndices(image->getElement(j), pivotVal, direction)) {
                j--;
            }

            if (i >= j) {
                break;
            }

            image->swapElements(i, j);
            i++;
            j--;
        }

        image->swapElements(low, j);

        return j; // Final position of the pivot
    }
    // Alternative: Lomuto partitioning (unused)
    /*
    int quickSort(Image* image, int low, int high, SortingDirection direction) {
        int pivotIndex = image->getElement(high);
        int i = (low - 1);

        for (int j = low; j <= high - 1; j++) {
            int currentIndex = image->getElement(j);
            if (!compareIndices(currentIndex, pivotIndex, direction)) {
                i++;
                image->swapElements(i, j);
            }
        }
        image->swapElements(i + 1, high);
        return (i + 1);
    }
    */

public:
    HybridQuickBubble(double constantC) : c(constantC) {}

    void sort(Image* image, SortingDirection direction) override {
        if (!image) return;
        int n = image->getElementCount();
        if (n <= 1) return;

        int maxDepth = (int)(c * log2(n));

        struct State {
            int left;
            int right;
            int depth;
        };

        stack<State> callStack;
        callStack.push({0, n - 1, 0});

        while (!callStack.empty()) {
            State current = callStack.top();
            callStack.pop();

            int l = current.left;
            int r = current.right;
            int depth = current.depth;

            if (l >= r) continue;

            if (depth >= maxDepth) {
                bubbleSort(image, l, r, direction);
            } else {
                int p = quickSort(image, l, r, direction);
                callStack.push({p + 1, r, depth + 1});
                callStack.push({l, p - 1, depth + 1});
            }
        }
    }
};


// std::sort baseline
class StdLibSort : public SortingAlgorithm {
public:
    void sort(Image* image, SortingDirection direction) override {
        if (!image) return;
        int n = image->getElementCount();
        if (n <= 1) return;

        // 1. Copy the indices into a vector so std::sort can be used.
        // Required because Image exposes neither the array nor iterators.
        vector<int> indexes(n);
        // Map: index -> current position in the Image object.
        // Used to apply the sorted order back to the Image efficiently.
        map<int, int> currentPositions;

        for(int i = 0; i < n; i++) {
            int index = image->getElement(i);
            indexes[i] = index;
            currentPositions[index] = i;
        }

        // 2. Sort the vector with std::sort
        if (direction == ASCENDING) {
            std::sort(indexes.begin(), indexes.end());
        } else {
            std::sort(indexes.begin(), indexes.end(), std::greater<int>());
        }

        // 3. Apply the sorted order to the Image using swapElements
        for (int i = 0; i < n; i++) {
            int targetIndex = indexes[i];
            int currentIndex = image->getElement(i);
            if (targetIndex != currentIndex) {
                int posOfTarget = currentPositions[targetIndex];
                image->swapElements(i, posOfTarget);
                currentPositions[currentIndex] = posOfTarget;
            }
        }
    }
};

// Main program and menu
const vector<string> files = {
    "image_1_asc.txt",
    "image_1_desc.txt",
    "image_1_rand.txt",
    "image_2_asc.txt",
    "image_2_desc.txt",
    "image_2_rand.txt",
    "image_3_asc.txt",
    "image_3_desc.txt",
    "image_3_rand.txt",
    "image_4_asc.txt",
    "image_4_desc.txt",
    "image_4_rand.txt"
};

void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int getIntegerInput(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            return value;
        } else {
            cout << "Error: please enter a valid integer." << endl;
            clearInputBuffer();
        }
    }
}

double getDoubleInput(const string& prompt) {
    double value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            return value;
        } else {
            cout << "Error: please enter a valid number." << endl;
            clearInputBuffer();
        }
    }
}

void reconstructWithAlgorithm(SortingAlgorithm* algorithmType) {
    for (const auto& filename : files) {
        string outFile = filename.substr(0, filename.find_last_of('.')) + "_sorted.txt";
        cout << "Processing file: " << filename << " -> " << outFile << " ... ";
        ImageProcessor::reconstructImage(filename.c_str(), outFile.c_str(), algorithmType);
    }
}

void benchmarkAlgorithms() {
    cout << "\n--- Benchmark parameters ---" << endl;
    int k = getIntegerInput("Enter constant k for Merge/Insertion (partition size threshold): ");
    double c = getDoubleInput("Enter constant c for Quick/Bubble (depth limit c * log2(n)): ");

    if (k < 1) k = 1;
    if (c < 0) c = 1;

    cout << "\n======================================================================================" << endl;
    cout << left << setw(24) << "File"
         << setw(24) << "Merge+Insertion (ms)"
         << setw(24) << "Quick+Bubble (ms)"
         << setw(24) << "std::sort (ms)" << endl;
    cout << "============================================================================================" << endl;

    for (const auto& filename : files) {
        long long time1 = 0, time2 = 0, time3 = 0;
        Stopwatch sw;
        string tempOut = "temp_out.txt";

        // Merge + Insertion
        {
            HybridMergeInsertion h1(k);
            sw.start();
            ImageProcessor::reconstructImage(filename.c_str(), tempOut.c_str(), &h1);
            time1 = sw.stop();
        }

        // Quick + Bubble
        {
            HybridQuickBubble h2(c);
            sw.start();
            ImageProcessor::reconstructImage(filename.c_str(), tempOut.c_str(), &h2);
            time2 = sw.stop();
        }

        // std::sort
        {
            StdLibSort h3;
            sw.start();
            ImageProcessor::reconstructImage(filename.c_str(), tempOut.c_str(), &h3);
            time3 = sw.stop();
        }

        cout << left << setw(24) << filename
             << setw(24) << time1
             << setw(24) << time2
             << setw(24) << time3 << endl;
    }
    cout << "============================================================================================" << endl;
}

int main() {
    srand(time(0));
    while (true) {
        cout << "\n=== IMAGE RECONSTRUCTION MENU ===" << endl;
        cout << "1. Reconstruct images (Merge Sort + Insertion Sort)" << endl;
        cout << "2. Reconstruct images (Quick Sort + Bubble Sort)" << endl;
        cout << "3. Benchmark (compare with std::sort)" << endl;
        cout << "4. Exit" << endl;
        cout << "Choose an option: ";

        int choice;
        if (!(cin >> choice)) {
            cout << "Invalid input. Please try again." << endl;
            clearInputBuffer();
            continue;
        }

        if (choice == 4) {
            cout << "Goodbye." << endl;
            break;
        }

        try {
            switch (choice) {
                case 1: {
                    int k = getIntegerInput("Enter partition size k (threshold for switching to insertion sort): ");
                    if (k < 1) k = 1;
                    HybridMergeInsertion algorithmHMI(k);
                    reconstructWithAlgorithm(&algorithmHMI);
                    break;
                }
                case 2: {
                    double c = getDoubleInput("Enter constant c (switch to bubble sort at depth c*log2(n)): ");
                    if (c < 0) c = 1;
                    HybridQuickBubble algorithmQB(c);
                    reconstructWithAlgorithm(&algorithmQB);
                    break;
                }
                case 3: {
                    benchmarkAlgorithms();
                    break;
                }
                default:
                    cout << "Unknown option." << endl;
            }
        } catch (const exception& e) {
            cout << "An error occurred during execution: " << e.what() << endl;
        } catch (...) {
            cout << "An unknown error occurred." << endl;
        }
    }

    return 0;
}