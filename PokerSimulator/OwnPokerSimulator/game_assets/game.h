//
// Created by Corny on 05.11.2017.
//

#ifndef OWNPOKERSIMULATOR_GAME_H
#define OWNPOKERSIMULATOR_GAME_H

#include <vector>
#include "player.h"
#include "table.h"

class Game {

private:
    std::vector<Table *> tables_;
    std::vector<Player *> players_;
    std::vector<Player *> playersOut_;
    std::vector<long> gameResults_ = std::vector<long>(GLOBAL_NUM_PLAYERS, 0); //TODO what? why this initializer?
    std::vector<long> handsWonResults_ = std::vector<long>(GLOBAL_NUM_PLAYERS, 0);
    std::vector<double> meanMoneyWonResults_ = std::vector<double>(GLOBAL_NUM_PLAYERS, 0);

    Rules *rules_;
    bool isRunning_;

protected:

public:

    Game();

    ~Game();

    int initialNumberOfTables_;

    void addPlayer(Player *player);

    void addTable(Table *table);

    void setupTablesAndSeatPlayers();

    void setRules(Rules *rules);


    void runTables();

    void playOneHand(Table *table);


    void cleanUpEmptySeats();

    void resetStatistics();


    void movePlayersToTableAndMergeTables();

    Table *getTableWithLeastPlayers();

    Table *getTableWithMostPlayers();

    void playGame();

    void cleanUpGame();


    std::vector<long> getOverallGameResults();

    std::vector<long> getOverallHandsWonResults();

    std::vector<double> getMeanMoneyWonResults();

};

#endif //OWNPOKERSIMULATOR_GAME_H
