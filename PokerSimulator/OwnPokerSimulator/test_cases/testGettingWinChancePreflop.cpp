//
// Created by Fabian Moik on 06.02.18.
//

#include <iostream>
#include "../random.h"
#include "../hand_eval/2+2/2P2Evaluator.h"
#include "../hand_eval/handEvalCalc.h"
#include "../game_assets/card.h"
#include <vector>
#include <sys/time.h>

inline double my_clock(void) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (1.0e-6*t.tv_usec + t.tv_sec);
}

int main() {

    seedRandomFastWithRandomSlow();

    // Setup 2+2 method
    TPTEvaluator::initEvaluator();

    // Setup Preflop Lookup Table
    TPTEvaluator::loadPreflopTables();

    int numOpponents = 5;
    double winChange;
    std::vector<Card> holeCards {Card(2, S_HEARTS), Card(2, S_DIAMONDS)};

    double start_time, end_time;
    start_time = my_clock();

    winChange = getEHSVsNOpponentsPreflop(holeCards, numOpponents);

    end_time = my_clock();
    double totalTime = end_time - start_time;
    std::cout << "Preflop lookup took : " << totalTime  << "seconds - with result: " << winChange << std::endl;

    return 0;
}