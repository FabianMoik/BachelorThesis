//
// Created by Corny on 05.11.2017.
//

#ifndef OWNPOKERSIMULATOR_DECK_H
#define OWNPOKERSIMULATOR_DECK_H

#include "card.h"

class Deck
{
    /*
    Deck of cards, that can be randomly shuffled using the true random.h, and
    allows easily selecting next cards.
    */

private:

    Card cards_[52]; //card 0 is the top card
    int index_;

public:

    Deck();
    void shuffle();
    Card next(); //never call this more than 52 times in a row.
};

#endif //OWNPOKERSIMULATOR_DECK_H
