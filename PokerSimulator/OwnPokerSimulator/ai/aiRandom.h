//
// Created by Fabian Moik on 11.12.17.
//

#ifndef OWNPOKERSIMULATOR_AIRANDOM_H
#define OWNPOKERSIMULATOR_AIRANDOM_H

#pragma once

#include "ai.h"

//Naive AI. This AI will do a random action, completely independent of its cards.
class AIRandom : public AI
{
public:

    virtual Action doTurn();
    virtual std::string getAIName();
};

#endif //OWNPOKERSIMULATOR_AIRANDOM_H
