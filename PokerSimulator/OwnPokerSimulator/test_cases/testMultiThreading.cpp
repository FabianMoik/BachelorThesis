//
// Created by Fabian Moik on 04.02.18.
//

#include <iostream>
#include <thread>
#include <vector>
#include <sys/time.h>

inline double my_clock(void) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (1.0e-6*t.tv_usec + t.tv_sec);
}

int threadFunction(int *a, int *b, std::vector<int> &results, int id) {
    for (int i = 0; i < 100000000; i++) {
        results[id] += *a + *b;
    }
}

int main() {

    std::thread *t_array[100];
    std::vector<int> resultVector;
    for (int i = 0; i < 100; i++) {
        resultVector.push_back(0);
    }

    int c = 1, d = 2;
    int *a = &c;
    int *b = &d;

    double start_time, end_time;
    start_time = my_clock();

    for (int k = 0; k < 100; k++) {
        for (int i = 0; i < 100000000; i++) {
            resultVector[k] += *a + *b;
        }
    }

    for (int i = 0; i < resultVector.size(); i++) {
        std::cout << "result " << i << ": " << resultVector[i] << std::endl;
    }


//    for (int i = 0; i < 100; i++) {
//        t_array[i] = new std::thread(threadFunction, a, b, std::ref(resultVector), i);
//    }
//
//    for (int i = 0; i < 100; i++) {
//        t_array[i]->join();
//    }
//
//    for (int i = 0; i < resultVector.size(); i++) {
//        std::cout << "result " << i << ": " << resultVector[i] << std::endl;
//    }


    end_time = my_clock();
    double totalTime = end_time - start_time;
    std::cout << 100 << " function calls took " << totalTime  << "seconds" << std::endl;

    return 0;
}

