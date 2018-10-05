//
// Created by Fabian Moik on 16.12.17.
//
#include "../game_assets/deck.h"
#include "../hand_eval/handEvalCalc.h"
#include "../hand_eval/2+2/2P2Evaluator.h"
#include "../hand_eval/pokermath.h"
#include <tuple>
#include <stdio.h>
#include "../random.h"

void testHandEvaluationSpeed();
void testGetHandStrengthVsNOpponents();

int main()
{

    seedRandomFastWithRandomSlow();
    // Setup 2+2 method
    TPTEvaluator::initEvaluator();

    testHandEvaluationSpeed();
    //testGetHandStrengthVsNOpponents();

    return 0;
}

void testHandEvaluationSpeed() {
    // Initialise hand ranking tables for PokerEval2
    int cards1[] = { 52, 48, 44, 40, 36, 12, 16}; // Royal Flush
    int cards2[] = { 52, 48, 44, 40, 36, 12}; // Royal Flush
    int cards3[] = { 52, 48, 44, 40, 36}; // Royal Flush

    int cards4[] = { TPTIndex(Card(8, S_HEARTS)), TPTIndex(Card(9, S_HEARTS)), TPTIndex(Card(10, S_HEARTS)), TPTIndex(Card(11, S_HEARTS)),
                     TPTIndex(Card(12, S_HEARTS)), TPTIndex(Card(2, S_CLUBS)), TPTIndex(Card(14, S_DIAMONDS))}; // Straight Flush

    int cards4_1[] = {TPTIndex(Card(12, S_HEARTS)), TPTIndex(Card(8, S_HEARTS)), TPTIndex(Card(3, S_SPADES)), TPTIndex(Card(12, S_DIAMONDS)), TPTIndex(Card(11, S_HEARTS)),
                      TPTIndex(Card(2, S_CLUBS)), TPTIndex(Card(2, S_DIAMONDS))}; // Straight Flush
    /*Fast 5,6 or 7-card evaluator using disk cache.
            Input: array of 7 cards (values: 2c = 1 2d = 2 2h = 3 2s = 4 3c = 5 3d = 6 3h = 7 3s = 8 4c = 9 4d = 10 4h = 11 4s = 12
    5c = 13 5d = 14 5h = 15 5s = 16 6c = 17 6d = 18 6h = 19 6s = 20 7c = 21 7d = 22 7h = 23 7s = 24 8c = 25 8d = 26 8h = 27
    8s = 28 9c = 29 9d = 30 9h = 31 9s = 32 Tc = 33 Td = 34 Th = 35 Ts = 36 Jc = 37 Jd = 38 Jh = 39 Js = 40 Qc = 41 Qd = 42
    Qh = 43 Qs = 44 Kc = 45 Kd = 46 Kh = 47 Ks = 48 Ac = 49 Ad = 50 Ah = 51 As = 52)*/
    int cards5[] = { 27, 31, 35, 39, 43, 1, 50}; // Royal Flush

    // general setup
    int result = 0;

    /*
    //analyse time demand for poker_eval2
    clock_t start1 = clock();

    for (long i=100000000; i > 0; i--) {
        result = eval7(cards1);
    }

    double time1 = ((double)clock() - start1 ) / CLOCKS_PER_SEC;
    std::cout << "handEvaluator (eval2): 100 000 hands took: " << time1 << "result: " << result << std::endl;
    */

    /*
    //analyse time demand for poker_eval
    clock_t start2 = clock();

    for (long i=10000000; i > 0; i--) {
        result = PokerEval::eval_7hand(cards1);
    }

    double time2 = ((double)clock() - start2 ) / CLOCKS_PER_SEC;
    std::cout << "handEvaluator (eval): 100 000 hands took: " << time2 << "result: " << result << std::endl;
    */

    //analyse time demand for 2+2
    clock_t start3 = clock();

    for (long i=100000000; i > 0; i--) {
        //TODO returning 0 for some reason... NOT WORKING?...
        //result = PokerEval::GetHandValue(cards1);
        //result = PokerEval::eval_5hand(cards3);
        result = TPTEvaluator::lookupHandForNCards(cards4_1, 7);
    }

    double time3 = ((double)clock() - start3 ) / CLOCKS_PER_SEC;
    printf("Category: %d\n", result >> 12);
    printf("Salt: %d\n", result & 0x00000FFF);
    std::cout << "handEvaluator (2+2): 100 000 hands took: " << time3 << "result: " << result << std::endl;
}

void testGetHandStrengthVsNOpponents() {

    // Vs random opponent the expected win chance is ~.585. Vs 5 opponents it is ~.069
    /*
    std::vector<Card> holeCards;
    holeCards.emplace_back(10, S_HEARTS);
    holeCards.emplace_back(10, S_DIAMONDS);

    std::vector<Card> boardCards;
    boardCards.emplace_back(4, S_DIAMONDS);
    boardCards.emplace_back(12, S_DIAMONDS);
    boardCards.emplace_back(13, S_HEARTS);
    */

    std::vector<Card> holeCards;
    holeCards.emplace_back(14, S_CLUBS);
    holeCards.emplace_back(9, S_CLUBS);

    std::vector<Card> boardCards;
    boardCards.emplace_back(5, S_DIAMONDS);
    boardCards.emplace_back(6, S_DIAMONDS);
    boardCards.emplace_back(7, S_DIAMONDS);


    int numOpponents = 2;
    int numSamples = 2000;
    double smallest = 1.0;
    double biggest = 0.0;

    // Get win change
    double result = 0.0;
    double result2 = 0.0;

    std::tuple<double, double> potentials(0.0, 0.0);

    //analyse time demand
    clock_t start = clock();

    for (long i=1000; i > 0; i--) {

        //result = getEHS(holeCards, boardCards, numOpponents);
        double resultTemp = getEHSVsNOpponents(holeCards, boardCards, numOpponents, numSamples);
        if (resultTemp < smallest) {
            smallest = resultTemp;
        }
        if (resultTemp > biggest) {
            biggest = resultTemp;
        }

        result += resultTemp;
        //result = getPotEquity(holeCards, boardCards, numOpponents, numSamples);
        //result2 = getEHSForMultipleOpponents(holeCards, boardCards, numOpponents);

    }
    result = result / 1000;
    double time = ((double)clock() - start) / CLOCKS_PER_SEC;
    std::cout << "getHandStrength (%) vs " << numOpponents << " opponents: (" << result << "|" << result2 << ") in " << time << " seconds" << std::endl;
    std::cout << "average: " << result << " smallest: " << smallest << " biggest: " << biggest << std::endl;

}