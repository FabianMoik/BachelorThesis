//
// Created by Fabian Moik on 08.12.17.
//

#ifndef OWNPOKERSIMULATOR_CARDDEALER_H
#define OWNPOKERSIMULATOR_CARDDEALER_H

#include "deck.h"
#include "../rules.h"
#include "action.h"

// Forward declaration so CardDealer knows of the existance of Table
class Table;

class CardDealer {
public:

    Table* table_;
    Rules* rules_;

    explicit CardDealer(Table *table, Rules* rules);
    ~CardDealer();

    void dealCards();
    void dealRounds();
    void settleBets();                      //TODO should be function of table
    bool betsSettled(int prev_current);     //TODO should be function of table
    void applyAction(const Action& action, long callAmount);
    void provideInputForAI();

    void burnCard();
    void applyForcedBets(const Rules* rules);
    void determineWinnerAndSplitPot();
    void dividePot(std::vector<long>& wins, const std::vector<long>& wager, const std::vector<long>& score, const std::vector<bool>& folded);

    int getInitialPlayer(Round round);
    int getNumDecidingPlayers();
    int getCurrentOrNextActivePlayer(int current);
    int getNextActivePlayer(int current);
    int getNumActivePlayers() const;
    void kickOutPlayers();

private:
    Deck deck_;
};


#endif //OWNPOKERSIMULATOR_CARDDEALER_H
