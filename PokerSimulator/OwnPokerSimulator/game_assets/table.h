//
// Created by Corny on 05.11.2017.
//

#ifndef OWNPOKERSIMULATOR_TABLE_H
#define OWNPOKERSIMULATOR_TABLE_H

#include <iostream>
#include <vector>
#include "player.h"
#include "card.h"
#include "../rules.h"

class CardDealer;

class Table
{
public:
    bool isRunning_;
    std::vector<Player*> players_;
    std::vector<Player*> playersOut_;
    CardDealer* cardDealer_;

    int dealer_; //index of the dealer in the players vector
    int current_; //index of the current player making a decision

    Round round_; // Enum in Rules
    int turn_; //how many decision making turns this round has had so far (if people keep raising all the time this could take forever!)

    /*
    This is roughly the last person who raised. This is used to know when a betting round stops.
    This is made so that if the current player is the lastRaiser, the round ends.
    This takes the fact that the big blind can make a decision into account.
    */
    int lastRaiser_;
    long lastRaiseAmount_;   //last raise amount during this deal. This is used to disallow smaller raises. Initially this
                            // is set to the big blind. All-ins don't count towards this amount, so that it's possible to
                            // form a side-pot with smaller bets. TODO all-ins should be counted, if minimum raise amount is covered
    long extra_ = 0;              // is needed if there is an All-in which is not a real raise, then the next real raise has to be at least
                            // as big as lastRaiseAmount + extra

    //NOTE: the values of these cards are only valid if the Round is correct.
    Card boardCard1_;     //flop card 1
    Card boardCard2_;     //flop card 2
    Card boardCard3_;     //flop card 3
    Card boardCard4_;     //turn card
    Card boardCard5_;     //river card


    Table();
    ~Table();

    void setID(int id);
    int getID();
    void seatPlayer(Player* player);
    void assignCardDealer(CardDealer* dealer);

    int wrap(int index) const; //wrap: convert any index into a valid player index. For example if you do "yourIndex - 1", this gets converted to the index of the player left of you, even if yourIndex was 0
    int getSmallBlindIndex() const;
    int getBigBlindIndex() const;
    int getHighestWager() const;
    int getCallAmount() const;
    int getPot() const;
    int getPositionOfPlayer(int index) const;    // returns relative position of player to dealer


private:

    int id_;
};

#endif //OWNPOKERSIMULATOR_TABLE_H
