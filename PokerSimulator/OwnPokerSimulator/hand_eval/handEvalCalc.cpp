//
// Created by Fabian Moik on 30.01.18.
//

#include "handEvalCalc.h"
#include "pokereval.h"
#include "pokereval2.h"
#include "../hand_eval/2+2/2P2Evaluator.h"
#include "../random.h"
#include "math.h"
#include <stdlib.h>

#define FLOP_SIZE 7
#define TURN_SIZE 8
#define RIVER_SIZE 9

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//////////////////////USED BY MYSELF ///////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

int TPTIndex(const Card& card)
{
    int v = (card.value_ - 2) * 4;
    return v + card.suit_ + 1;      //returns a value between 1 and 52 (2c = 1, 2d = 2, 2h = 3, 2s = 4, ..., As = 52‚)
}

double getHandStrength(const std::vector<Card>& holeCards, const std::vector<Card>& boardCards, int numOpponents) {

    //an array of size @arraySize (2 hole cards, 2 opp cards, and .size() board cards)
    long arraySize = holeCards.size() + boardCards.size() + 2;
    int c[arraySize];

    // Prefill array with -1
    for (int i = 0; i < arraySize; i++) {
        c[i] = -1;
    }

    // Index preperation for eval2
    c[0] = TPTIndex(holeCards[0]);
    c[1] = TPTIndex(holeCards[1]);
    for (int i = 0; i < boardCards.size(); i++) {
        c[i + 2] = TPTIndex(boardCards[i]);
    }

    // Fill others array with remaining cards and index them for the 2P2Eval algorithm
    int NUMOTHER = 50 - int(boardCards.size());

    int *otherCards = NULL;
    otherCards = getRemainingCards(c, NUMOTHER);

    //Start of algorithm
    double ahead = 0, behind = 0, tied = 0;

    // Get our rank
    long length = 2 + boardCards.size();
    int ourVal = TPTEvaluator::lookupHandForNCards(&c[0], int(length));

    for (int k = 0; k < NUMOTHER - 1; k++) //opponent hand
    {
        for (int l = k + 1; l < NUMOTHER; l++) //opponent hand
        {
            c[arraySize - 2] = otherCards[k];
            c[arraySize - 1] = otherCards[l];

            int oppVal = TPTEvaluator::lookupHandForNCards(&c[2], int(length));

            if (ourVal > oppVal) ahead++;
            else if (ourVal == oppVal) tied++;
            else behind++;
        }
    }

    delete[] otherCards;

    double handStrenght = (ahead + tied / 2) / (ahead + tied + behind);

    // If there are multiple opponents there is a good approximation where you take the reuslt of the hand
    // strength calculation to the power of the number of opponents.
    handStrenght = pow(handStrenght, numOpponents);

    return handStrenght;
}

double getEHS(const std::vector<Card>& holeCards, const std::vector<Card>& boardCards, int numOpponents) {
// Hand potential array, each index represents ahead, tied, behind.
    int HP[3][3] = {{0}}; //TODO see if it is zeroed
    int HPTotal[3] = {0}; //more or less the same as the ahead++, tied++ ... from above

    //an array of size @arraySize (2 hole cards, 2 opp cards, and .size() board cards)
    long arraySize = holeCards.size() + boardCards.size() + 2;
    int c[arraySize];

    // Prefill array with -1
    for (int i = 0; i < arraySize; i++) {
        c[i] = -1;
    }

    // Index preperation for eval2
    c[0] = TPTIndex(holeCards[0]);
    c[1] = TPTIndex(holeCards[1]);
    for (int i = 0; i < boardCards.size(); i++) {
        c[i + 2] = TPTIndex(boardCards[i]);
    }

    // Fill others array with remaining cards and index them for the 2P2Eval algorithm
    int NUMOTHER = 50 - int(boardCards.size());
    int *otherCards;
    otherCards = getRemainingCards(c, NUMOTHER);

    int ahead = 0;
    int tied = 1;
    int behind = 2;
    int index;

    //Start of algorithm

    // Get our rank
    long length = 2 + boardCards.size();
    int ourVal = TPTEvaluator::lookupHandForNCards(&c[0], int(length));

    // There are 47 choose 2 (1081 combinations) on the flop

    double numOfBoardSimulations = 0;

    for(int k = 0; k < NUMOTHER - 1; k++) //opponent hand
    {
        for(int l = k + 1; l < NUMOTHER; l++) //opponent hand
        {
            c[arraySize-2] = otherCards[k];
            c[arraySize-1] = otherCards[l];

            int oppVal = TPTEvaluator::lookupHandForNCards(&c[2], int(length));


            if(ourVal > oppVal) index = ahead;
            else if(ourVal == oppVal) index = tied;
            else index = behind;
            HPTotal[index] += 1;

            //get remaining cards
            // Fill others array with remaining cards and index them for the 2P2Eval algorithm
            static const int NUMREMAIN = NUMOTHER - 2;

            int *remainingCards;
            remainingCards = getRemainingCards(c, NUMREMAIN);

            // Fill the array so that the first two cards are the players cards then there are 5 board cards and the
            // last two cards are opponent cards
            int fullBoardPlusHands[9]; // 5 board cards and 2 cards for both player and opponent

            for (int i = 0; i < int(boardCards.size() + 2); i++) {
                fullBoardPlusHands[i] = c[i];
            }
            fullBoardPlusHands[7] = c[arraySize - 2]; // opp card1
            fullBoardPlusHands[8] = c[arraySize - 1]; // opp card2

            //It depends on which street we are (FLOP, TURN or RIVER), to decide
            if (arraySize == FLOP_SIZE) {

                numOfBoardSimulations = 0;
                //Two cards to come (45 choose 2 = 990 combinations)
                for(int b1 = 0; b1 < NUMREMAIN - 1; b1++) //board
                {
                    for(int b2 = b1 + 1; b2 < NUMREMAIN; b2++) //board
                    {
                        fullBoardPlusHands[5] = remainingCards[b1];
                        fullBoardPlusHands[6] = remainingCards[b2];

                        int ourBest = TPTEvaluator::lookupHandForNCards(&fullBoardPlusHands[0], 7);
                        int oppBest = TPTEvaluator::lookupHandForNCards(&fullBoardPlusHands[2], 7);

                        if(ourBest > oppBest) HP[index][ahead] += 1;
                        else if(ourBest == oppBest) HP[index][tied] += 1;
                        else HP[index][behind] += 1;
                        numOfBoardSimulations++;
                    }
                }
            } else if (arraySize == TURN_SIZE){
                //One cards to come (44 combinations)
                for(int b1 = 0; b1 < NUMREMAIN; b1++) //board
                {
                    fullBoardPlusHands[6] = remainingCards[b1];

                    int ourBest = TPTEvaluator::lookupHandForNCards(&fullBoardPlusHands[0], 7);
                    int oppBest = TPTEvaluator::lookupHandForNCards(&fullBoardPlusHands[2], 7);

                    if(ourBest > oppBest) HP[index][ahead] += 1;
                    else if(ourBest == oppBest) HP[index][tied] += 1;
                    else HP[index][behind] += 1;
                    numOfBoardSimulations++;
                }
            } else if (arraySize == RIVER_SIZE){
                // No more cards to come to we have no potential and can return
                double HS = (HPTotal[ahead] +  HPTotal[tied] / double(2)) / (HPTotal[ahead] + HPTotal[tied] + HPTotal[behind]);
                return HS;
            } else {
                //TODO raise exeption, should not end up here
            }

            delete[] remainingCards;
        }
    }

    delete[] otherCards;

    // Get the hand strength
    double HS = (HPTotal[ahead] +  HPTotal[tied] / double(2)) / (HPTotal[ahead] + HPTotal[tied] + HPTotal[behind]);

    // If there are multiple opponents there is a good approximation where you take the result of the hand
    // strength calculation to the power of the number of opponents.
    HS = pow(HS, numOpponents);

    //PPot: we were behind but moved ahead
    double ppot = (HP[behind][ahead] + HP[behind][tied] / 2 + HP[tied][ahead] / 2) /
                  (HPTotal[behind] + HPTotal[tied]) / numOfBoardSimulations;

    //NPot: we were ahead but fell behind
    double npot = (HP[ahead][behind] + HP[tied][behind] / 2 + HP[ahead][tied] / 2) /
                  (HPTotal[ahead] + HPTotal[tied]) / numOfBoardSimulations;

    double EHS = HS * (1 - npot) + (1 - HS) * ppot;

    /*
    std::cout << "Transformationmatrix output:" << std::endl;
    std::cout << "       Ahead     Tied     Behind" << std::endl;
    std::cout << "Ahead " << HP[ahead][ahead] << " " << HP[ahead][tied] << " " << HP[ahead][behind] << " " << std::endl;
    std::cout << "Tied " << HP[tied][ahead] << " " << HP[tied][tied] << " " << HP[tied][behind] << " " << std::endl;
    std::cout << "Behind " << HP[behind][ahead] << " " << HP[behind][tied] << " " << HP[behind][behind] << " " << std::endl;
    std::cout << "Ahead-t: " << HPTotal[ahead] << " Tied-t: " << HPTotal[tied] << " Behind-t: " << HPTotal[behind] << std::endl;
     */

    return EHS;
}

double getEHSVsNOpponentsPreflop(const std::vector<Card>& holeCards, int numOpponents) {

    //Convert hand to index
    // given two card values between 2-14 (2...A) we can convert those hands to map 169 distinct hands
    // by applying following equation, which was directly derived from my mapping of 169 into the array
    //
    // the abs value is needed because the equation assumes that the cards are in descending order
    // the -1 in the off suited case is gone because the index of off-suited hands are +1 to their suited partners
    int index = 0;

    // This is needed because equation assumes that A is 13 and not 14 and that hcOneVal is the bigger one
    int hcOneVal = holeCards.at(0).value_ - 1;
    int hcTwoVal = holeCards.at(1).value_ - 1;

    if (hcOneVal < hcTwoVal) {
        int temp = hcTwoVal;
        hcTwoVal = hcOneVal;
        hcOneVal = temp;
    }

    if (hcOneVal == hcTwoVal) { //pocket pair
        index = 169 - hcOneVal * hcOneVal;
    } else {
        if (holeCards.at(0).suit_ == holeCards.at(1).suit_) { // suited
            index = 169 - hcOneVal * hcOneVal + (hcOneVal - hcTwoVal) * 2 - 1;
        } else { //off suited
            index = 169 - hcOneVal * hcOneVal + (hcOneVal - hcTwoVal) * 2;
        }
    }

    //index for multiple opponents
    index = index + 169 * (numOpponents - 1);

    //get the corresponding float win percentage
    float winChange = TPTEvaluator::lookupWinChanceForHandVsNOpponentsPreflop(index);

    if (winChange == -1) { std::cerr << "handEvalCalc: invalid preflop win percentage" << std::endl; return -1;}

    return (double)winChange;
}

// Uses a Monte Carlo Simulation to simulate some hand runouts to determine the winner in a n-opponent hand
// Works super fast
double getEHSVsNOpponents(const std::vector<Card>& holeCards, const std::vector<Card>& boardCards, int numOpponents, int numSamples) {

    //an array of size 9 to hold 2 hero cards, 5 board cards and 2 opp cards
    int c[9];

    // Index preperation for eval2
    c[0] = TPTIndex(holeCards[0]);
    c[1] = TPTIndex(holeCards[1]);
    for (int i = 0; i < boardCards.size(); i++) {
        c[i + 2] = TPTIndex(boardCards[i]);
    }

    int numRemainingBoardCards = 5 - int(boardCards.size());

    // Fill others array with remaining cards and index them for the 2P2Eval algorithm
    int NUMOTHER = 50 - int(boardCards.size());
    int *otherCards;
    otherCards = getRemainingCards(c, NUMOTHER);

    //Start of algorithm
    int wins = 0;
    int ties = 0;
    int losses = 0;

    for(int i = 0; i < numSamples; i++)
    {
        shuffleN(otherCards, NUMOTHER, numRemainingBoardCards + numOpponents * 2); //the two extra table cards, and the cards of all opponents
        if (numRemainingBoardCards == 5) {
            c[2] = otherCards[0];
            c[3] = otherCards[1];
            c[4] = otherCards[2];
            c[5] = otherCards[3];
            c[6] = otherCards[4];
            // shouldn't be here because we distinguish between preflop and postflop
        }
        if (numRemainingBoardCards == 2) {
            c[5] = otherCards[0];
            c[6] = otherCards[1];
        } else if (numRemainingBoardCards == 1) {
            c[6] = otherCards[0];
        } else { /*shouldn't be here*/}

        int yourVal = TPTEvaluator::lookupHandForNCards(&c[0], 7);

        int status = 2; //2: you win, 1: you tie, 0: you lose

        for(int j = 0; j < numOpponents; j++)
        {
            //opponents hand
            c[7] = otherCards[numRemainingBoardCards + j * 2];
            c[8] = otherCards[numRemainingBoardCards + 1 + j * 2];

            int opponentVal = TPTEvaluator::lookupHandForNCards(&c[2], 7);

            if(opponentVal == yourVal) status = 1; //tie
            else if(opponentVal > yourVal) { status = 0; break; } //lose, stop rest of loop.
        }

        if(status == 0) losses++;
        else if(status == 1) ties++;
        else wins++;
    }

    // Get the hand strength
    double HS = (wins + ties/double(2)) / (wins + ties + losses);

    /*
    std::cout << "Your hand: " << holeCards.at(0).getShortName() << ", " << holeCards.at(1).getShortName() << std::endl;
    std::cout << "Board: ";
    for (auto &card: boardCards) {
        std::cout << card.getShortName() << ", ";
    }
    std::cout << std::endl;
    std::cout << "Num opp: " << numOpponents << std::endl;
    std::cout << "Hand stregth: " << HS << std::endl;
    */

    return HS;
}

int* getRemainingCards(const int* knownCards, int numOtherCards) {

    int NUMKNOWN = 52 - numOtherCards;
    int *otherCards1;
    otherCards1 = new int[numOtherCards];

    int j = 0;
    for(int i = 0; i < 52; i++)
    {
        int v = TPTIndex(Card(i));

        bool found = false;
        for (int k = 0; k < NUMKNOWN; k++) {
            if (v == knownCards[k]) found = true;
        }
        if (found) continue;
        if(j >= numOtherCards) break;

        otherCards1[j] = v;
        j++;
    }

    return otherCards1;
}

/*
This function is there for efficiently choosing "amount" unique random cards
out of a list of "size" cards. For that, this function shuffles the first "amount"
cards of the list.
The intention is that this function is:
-Fast (it's for monte carlo simulations of many hands)
-Properly Random (no errors that skew the randomness)
So the function is made O(amount), instead of shuffling
the whole deck all the time which would result in an O(size) function.
When the function is done, use the first "amount" cards from "values" to
have your really new randomly shuffled cards.
*/
void shuffleN(int* values, int size, int amount)
{
    for(int i = 0; i < amount; i++)
    {
        int r = getRandomFast(0, size - 1);
        std::swap(values[i], values[r]);
    }
}


///////////////////////// EXPERIMENTAL /////////////////////

double getEHSForMultipleOpponents(const std::vector<Card>& holeCards, const std::vector<Card>& boardCards, int numOpponents) {

    //an array of size @arraySize (2 hole cards, 2 opp cards, and .size() board cards)
    long arraySize = holeCards.size() + boardCards.size() + 2;
    int c[arraySize];

    // Prefill array with -1
    for (int i = 0; i < arraySize; i++) {
        c[i] = -1;
    }

    // Index preperation for TPT
    c[0] = TPTIndex(holeCards[0]);
    c[1] = TPTIndex(holeCards[1]);
    for (int i = 0; i < boardCards.size(); i++) {
        c[i + 2] = TPTIndex(boardCards[i]);
    }

    // Fill others array with remaining cards and index them for the 2P2Eval algorithm
    int NUMOTHER = 50 - int(boardCards.size());
    int *otherCards;
    otherCards = getRemainingCards(c, NUMOTHER);

    // Fill the array so that the first two cards are the players cards then there are 5 board cards and the
    // last two cards are opponent cards
    int fullBoardPlusHands[9]; // 5 board cards and 2 cards for both player and opponent

    for (int i = 0; i < int(boardCards.size() + 2); i++) {
        fullBoardPlusHands[i] = c[i];
    }

    double numOfBoardSimulations = 0;
    double totalWins = 0, totalTies = 0, totalShowdowns = 0;
    double EHS = 0, EHS2 = 0;

    if (arraySize == FLOP_SIZE) {

        numOfBoardSimulations = 0;
        //Two cards to come (45 choose 2 = 990 combinations)
        for(int b1 = 0; b1 < NUMOTHER - 1; b1++) //board
        {
            for (int b2 = b1 + 1; b2 < NUMOTHER; b2++) //board
            {
                fullBoardPlusHands[5] = otherCards[b1];
                fullBoardPlusHands[6] = otherCards[b2];

                int ourVal = TPTEvaluator::lookupHandForNCards(&fullBoardPlusHands[0], 7);

                int ahead = 0;
                int tied = 0;
                int showdown = 0;
                numOfBoardSimulations++;

                //get remaining cards
                // Fill others array with remaining cards and index them for the 2P2Eval algorithm
                static const int NUMREMAIN = NUMOTHER - 2;

                int *remainingCards;
                remainingCards = getRemainingCards(c, NUMREMAIN);

                for (int k = 0; k < NUMREMAIN - 1; k++) //opponent hand
                {
                    for (int l = k + 1; l < NUMREMAIN; l++) //opponent hand
                    {
                        fullBoardPlusHands[7] = remainingCards[k];
                        fullBoardPlusHands[8] = remainingCards[l];

                        int oppVal = TPTEvaluator::lookupHandForNCards(&fullBoardPlusHands[2], 7);

                        if (ourVal > oppVal) ahead++;
                        else if (ourVal == oppVal) tied++;
                        showdown++;
                    }
                }
                totalWins += ahead;
                totalTies += tied;
                totalShowdowns += showdown;
                double _ehs = (ahead + tied / 2) * 1.0 / showdown;
                EHS += _ehs;
                EHS2 += _ehs * _ehs * _ehs  ;
            }
        }
    }
    EHS = EHS / numOfBoardSimulations;
    EHS2 = EHS2 / numOfBoardSimulations;

    return EHS2;
}

std::vector<double> createPreflopLookupTable() {

    int *deck;
    deck = getRemainingCards(deck, 52);
}
