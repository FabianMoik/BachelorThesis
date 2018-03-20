//
// Created by Fabian Moik on 02.02.18.
//

#ifndef OWNPOKERSIMULATOR_2P2EVALUATOR_CPP_H
#define OWNPOKERSIMULATOR_2P2EVALUATOR_CPP_H

namespace TPTEvaluator {

    bool initEvaluator();
    bool loadPreflopTables();
    int lookupHandForNCards(int* pCards, int length);
    float lookupWinChanceForHandVsNOpponentsPreflop(int index);
    void enumerateAll7CardHands();
}

#endif //OWNPOKERSIMULATOR_2P2EVALUATOR_CPP_H