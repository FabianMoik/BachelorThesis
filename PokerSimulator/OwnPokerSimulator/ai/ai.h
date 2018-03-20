//
// Created by Fabian Moik on 11.12.17.
//

#ifndef OWNPOKERSIMULATOR_AI_H
#define OWNPOKERSIMULATOR_AI_H

#pragma once

#include "../game_assets/action.h"

class AI //interface class, used for bots
{
public:

    virtual ~AI(){}

    /*
    doTurn:
    make a decision for this turn: fold, check, call or raise?
    */
    virtual Action doTurn() = 0;

    /*TODO - needed?
    boastCards:
    called at the end of a deal, only if this AI wasn't required to show his cards.
    */
    //virtual bool boastCards(const Info& info);

    /*
    getAIName:
    This is not the name of the player, but the name of his "type of brains".
    */
    virtual std::string getAIName() = 0;

    virtual void setName(std::string name) {};

    virtual void fillInputValues(std::vector<double> &input) {};

};


#endif //OWNPOKERSIMULATOR_AI_H
