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


    int numOpponents = 8;
    int numSamples = 1000000;
    double winChange;
    std::vector<Card> holeCards;
    holeCards.resize(2);
    std::vector<Card> boardCards;

    float winChanges[169][numOpponents];

    double start_time, end_time;
    start_time = my_clock();

    for (int p = 0; p < numOpponents; p++) {
        int distinctHandsCount = 0;
        for (int i = 14; i > 1; i--) {
            for (int j = i; j > 1; j--) {
                if (i == j) { //pocket pair
                    holeCards.at(0) = Card(i, S_SPADES);
                    holeCards.at(1) = Card(j, S_DIAMONDS);
                    winChange = getEHSVsNOpponents(holeCards, boardCards, p + 1, numSamples);
                    winChanges[distinctHandsCount][p] = float(winChange);
                    distinctHandsCount++;

                } else {
                    // two cases, either suited or unsuited, therefore two calculations

                    //suited
                    holeCards.at(0) = Card(i, S_SPADES);
                    holeCards.at(1) = Card(j, S_SPADES);
                    winChange = getEHSVsNOpponents(holeCards, boardCards, p + 1, numSamples);
                    winChanges[distinctHandsCount][p] = float(winChange);

                    //off-suited
                    holeCards.at(0) = Card(i, S_SPADES);
                    holeCards.at(1) = Card(j, S_DIAMONDS);
                    winChange = getEHSVsNOpponents(holeCards, boardCards, p + 1, numSamples);
                    winChanges[distinctHandsCount + 1][p] = float(winChange);
                    distinctHandsCount += 2;
                }
                std::cout << "Hands done: " << distinctHandsCount << std::endl;
            }
        }
        std::cout << "Distinct Hands Count: " << distinctHandsCount << std::endl;
    }

    for (int i = 0; i < 169; i++) {
        for (int j = 0; j < numOpponents; j++) {
            std::cout << winChanges[i][j] << " - ";
        }
        std::cout << std::endl;
    }

    end_time = my_clock();
    double totalTime = end_time - start_time;
    std::cout << "Full preflop table creation took: " << totalTime  << "seconds" << std::endl;


    /////////////////// SAVING /////////////////////////////
    // output the array now that I have it!!

    //Convert 2-d array to 1-d array
    float winChangesOneD[169 * numOpponents];

    int index = 0;
    for (int j = 0; j < numOpponents; j++) {
        for (int i = 0; i < 169; i++) {
            winChangesOneD[index] = winChanges[i][j];
            index++;
        }
    }

    std::cout << "Reference Value: " << winChangesOneD[222] << std::endl;

    FILE * fout = fopen("../LookupTables/preflopTables.dat", "wb");
    if (!fout) {
        printf("Problem creating the Output File!\n");
        return 1;
    }
    fwrite(winChangesOneD, sizeof(winChangesOneD), 1, fout);  // big write, but quick

    fclose(fout);

    ////////////////////// LOADING /////////////////////////////

    // Load the preflopTables.DAT file and map it into the array
    printf("Loading preflopTables.DAT file...");
    float winChangesInput[169 * numOpponents];
    memset(winChangesInput, 0, sizeof(winChangesInput));
    FILE * fin = fopen("../LookupTables/preflopTables.dat", "rb");
    if (!fin)
        return false;
    size_t bytesread = fread(winChangesInput, sizeof(winChangesInput), 1, fin);	// get the HandRank Array
    fclose(fin);
    printf("complete.\n\n");

    std::cout << "Control Value: " << winChangesInput[222] << std::endl;

    return 0;
}