//
// Created by Fabian Moik on 11.12.17.
//

#ifndef OWNPOKERSIMULATOR_ACTION_H
#define OWNPOKERSIMULATOR_ACTION_H

#pragma once

#include "card.h"
#include "../util.h"

enum Command
{
    A_FOLD,
    A_CHECK,
    A_CALL,
    A_RAISE //also used to BET. Requires amount to be given, and amount must be amount of chips moved to table, not the amount raises with.
};

class Action {

public:
    Command command_;
    long amount_; //Only used for the A_RAISE command. This is NOT the raise amount. This is the total value of money you
                // move from your stack to the pot. So if the call amount was 50, and you raise with 100, then this
                // amount must be set to 150, not 100.
    int betType_;

    explicit Action(Command command, long amount = 0, int betType = B_UNKNOWN_BET);       // the amount param is optional and only needed for the A_RAISE command
    Action();                                               // Standard action is a A_FOLD action

    bool isValidAction(long stack, long wager, long highestWager, long highestRaise, long extra);
    bool isValidAllInAction(long stack, long wager, long highestWager, long highestRaise);
};

#endif //OWNPOKERSIMULATOR_ACTION_H
