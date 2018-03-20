//
// Created by Corny on 05.11.2017.
//

#include "deck.h"
#include "../random.h"

Deck::Deck()
        : index_(0)
{
    for(size_t i = 0; i < 52; i++) cards_[i].setIndex(i);
}

void Deck::shuffle()
{
    index_ = 0;

    Card old[52];
    for(size_t i = 0; i < 52; i++) old[i] = cards_[i];

    //Fisher-Yates shuffle
    for(size_t i = 0; i < 52; i++)
    {
        int r = (int)(getRandom() * (52 - i));
        cards_[i] = old[r];
        std::swap(old[r], old[(52 - 1 - i)]);

    }
}

Card Deck::next()
{
    if(index_ >= 52) return Card();  // returns an invalid card for sanity check

    Card result = cards_[index_];
    index_++;
    return result;
}
