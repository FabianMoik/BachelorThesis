//
// Created by Corny on 05.11.2017.
//

#ifndef OWNPOKERSIMULATOR_PLAYER_H
#define OWNPOKERSIMULATOR_PLAYER_H

#include "card.h"
#include "action.h"
#include "../ai/aiOwn.h"

class AI;

class Player {

private:

    AI* ai_; //the AI for the player
    int id_;

public:

    int tableID_;

    long stack_; //chips in his stack
    long wager_; //how much chips this person currently has in the pot on the table (note: the "int stack" variable does NOT include these chips anymore, they're moved from stack to pot)

    Card holeCard1_;
    Card holeCard2_;

    bool folded_;
    bool showdown_; //this player (has to or wants to) show their cards
    Action lastAction_;
    long handsWon_;
    long handsLost_;
    long chipsWonTotal_;
    long chipsLostTotal_;

    bool hallOfFameMember_;
    int overallRanking_;

    //public Information for other players
    int getID()const;
    AI* getAI();

    bool isAllIn() const;
    bool isOut() const; //can't play anymore, has no more money
    bool hasFolded() const;
    bool canDecide() const; //returns true if stack > 0 and not folded

    ///////////////////////////////////////////////////////////////////
    Player();
    Player(AI *ai);
    Player(const Player& player);
    ~Player();

    // Action functions:
    void setID(int id);
    long placeMoney(long amount);
    Action doTurn();
    long getBetAmount(int betType, long maxBet, long minBet);
    void setUpAI(std::vector<double> &input) const;
};

#endif //OWNPOKERSIMULATOR_PLAYER_H
