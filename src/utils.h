#ifndef ASP2_DZ3_UTILS_H
#define ASP2_DZ3_UTILS_H

#include <chrono>

using namespace std;

class Stopwatch {
private:
    chrono::steady_clock::time_point startTime;
    bool running = false;

public:
    void start() {
        running = true;
        startTime = chrono::steady_clock::now();
    }

    long long stop() {
        if (!running) {
            return 0;
        }

        const chrono::time_point endTime = chrono::steady_clock::now();
        running = false;

        return chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();
    }
};

#endif //ASP2_DZ3_UTILS_H