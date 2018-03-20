//
// Created by Fabian Moik on 30.01.18.
//

#pragma once

#include "../game_assets/card.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
    Fast 5,6 or 7-card evaluator using disk cache.
    Input: array of 7 cards (values: 2c = 1 2d = 2 2h = 3 2s = 4 3c = 5 3d = 6 3h = 7 3s = 8 4c = 9 4d = 10 4h = 11 4s = 12
    5c = 13 5d = 14 5h = 15 5s = 16 6c = 17 6d = 18 6h = 19 6s = 20 7c = 21 7d = 22 7h = 23 7s = 24 8c = 25 8d = 26 8h = 27
    8s = 28 9c = 29 9d = 30 9h = 31 9s = 32 Tc = 33 Td = 34 Th = 35 Ts = 36 Jc = 37 Jd = 38 Jh = 39 Js = 40 Qc = 41 Qd = 42
    Qh = 43 Qs = 44 Kc = 45 Kd = 46 Kh = 47 Ks = 48 Ac = 49 Ad = 50 Ah = 51 As = 52)
    Output: integer, the higher the better combination. Return value >> 12 gives combination rank (1 for high card to 9 for straight flush), return value & 0xFFF gives rank within that combination.
*/
int TPTIndex(const Card& card);
double getHandStrength(const std::vector<Card>& holeCards, const std::vector<Card>& boardCards, int numOpponents);
std::tuple<double, double> getHandPotential(const std::vector<Card>& holeCards, const std::vector<Card>& boardCards, int numOpponents);
double getEHS(const std::vector<Card>& holeCards, const std::vector<Card>& boardCards, int numOpponents);
double getEHSVsNOpponents(const std::vector<Card>& holeCards, const std::vector<Card>& boardCards, int numOpponents, int numSamples);
double getEHSVsNOpponentsPreflop(const std::vector<Card>& holeCards, int numOpponents);
int* getRemainingCards(const int* knownCards, int numOtherCards);
void shuffleN(int* values, int size, int amount);

double getEHSForMultipleOpponents(const std::vector<Card>& holeCards, const std::vector<Card>& boardCards, int numOpponents);

std::vector<double> createPreflopLookupTable();