//
// Created by Corny on 05.11.2017.
//

#include "game.h"
#include "cardDealer.h"
#include "../random.h"
#include <math.h>       /* ceil */
#include <stdexcept>
#include "../util.h"
#include <random>
#include <sys/time.h>
#include <algorithm>

Game::Game()
{
    //rules_ = new Rules();
}

Game::~Game()
{
    //cleanup
    //for(int i = 0; i < observers.size(); i++) delete observers[i];
    //for (auto &player : players_) delete player; //TODO players have to be deleted at some point
    for (auto &table : tables_) delete table;
    //delete rules_;
}

void Game::addPlayer(Player* player) {
    players_.push_back(player);
}

void Game::addTable(Table* table)
{
    tables_.push_back(table);
}

void Game::setRules(Rules* r)
{
    rules_ = r;
}
void Game::playGame()
{
    //give players a buyin and reset all values
    for (auto &player : players_) {
        player->stack_ = rules_->buyIn_;
        player->wager_ = 0;
        player->tableID_ = -1;
        player->holeCard1_ = Card();
        player->holeCard2_ = Card();
        player->lastAction_ = Action();
        player->folded_ = false;
        player->showdown_ = false;
    }

    setupTablesAndSeatPlayers();

    runTables();

    // CREATE STATISTICS FOR FITNESS FUNCTION
    // TODO check why I use this evaluation of hand won and lost

    int place = GLOBAL_NUM_PLAYERS;
    for (auto &player : playersOut_) {
        gameResults_.at(player->getID()) += place;

        if (place == 7) {
            player->tournament_7th += 1;
        } else if (place == 6) {
            player->tournament_6th += 1;
        } else if (place == 5) {
            player->tournament_5th += 1;
        } else if (place == 4) {
            player->tournament_4th += 1;
        } else if (place == 3) {
            player->tournament_3rd += 1;
        } else if (place == 2) {
            player->tournament_2nd += 1;
        } else if (place == 1) {
            player->tournament_1st += 1;
        }
        place--;
    }

    for (auto &player : players_) {
        //std::cout << place << ".) player - " << player->getID() << std::endl;
        handsWonResults_.at(player->getID()) = player->handsWon_;
        if (player->handsWon_ != 0 && player->handsWon_ != 0) {
            meanMoneyWonResults_.at(player->getID()) = player->chipsWonTotal_ * 1.0 / player->handsWon_ - player->chipsLostTotal_ * 1.0 / player->handsLost_;
        } else {
            if (player->handsWon_ == 0 && player->handsLost_ == 0) {
                meanMoneyWonResults_.at(player->getID()) = 0;

            } else if (player->handsWon_ == 0) {
                meanMoneyWonResults_.at(player->getID()) = - player->chipsLostTotal_ * 1.0 / player->handsLost_;
            } else {
                meanMoneyWonResults_.at(player->getID()) = player->chipsWonTotal_ * 1.0 / player->handsWon_;
            }
        }
    }

    globalGameCounter++;
    outputFile.close();
}

// All tables play one hand at the time and once a hand has finished, a TableCheck reseats players if needed to
// balance the game. After that another hand is player, and so on.
void Game::runTables() {
    isRunning_ = true;

    // Initialize a random dealer on each table and set the table to running
    for (auto &table : tables_) {
        table->dealer_ = getRandom(0, int(table->players_.size() - 1));
        table->isRunning_ = true;
    }

    int handCount = 0;

    // While the game is running, deal one hand at a time on each table
    while (isRunning_) {
#ifdef DEBUG
        outputFile << "###################### Hand: " << handCount << " -- " << "Table Count: ( ";
        for (auto &table : tables_) {
            outputFile << table->players_.size() << " , ";
        }
        outputFile << ") ######################" << std::endl << std::endl;
#endif

        // Increase blinds and antes every 12 hands (~ 5min in real life)
        if (handCount % 6 == 0) {
            if (handCount / 6 < rules_->blindLevels_.size()) {
                rules_->blindLevel_ = rules_->blindLevels_.at(handCount/6);
#ifdef DEBUG
                outputFile << "///////////////////// BlindLevel (" << rules_->blindLevel_.smallBlind_ << " , "
                          << rules_->blindLevel_.bigBlind_ << " , "<< rules_->blindLevel_.ante_ <<
                          " )/////////////////////" << std::endl;
#endif

            } else {
                std::cerr << "Invalid Blind level!!! " << std::endl;
            }
        }

        isRunning_ = false;
        for (auto &table : tables_) {
            table->numTotalPlayers = players_.size();
            table->numPlayersLeftInTournament = players_.size() - playersOut_.size();
            if (table->isRunning_) {
                playOneHand(table);
                isRunning_ = true;
            }
        }

        //Check if only one player left in the whole game
        if (playersOut_.size() == players_.size()-1) {
            tables_.at(0)->isRunning_ = false;
            playersOut_.push_back(tables_[0]->players_[0]);
            isRunning_ = false;
        } else {
            // clean up tables from busted players and save them in the playersOut_ of game
            // TODO for ordering players and get their final position it should be checked, which player in this hand
            // TODO on which table had less chips at the beginning of the hand
            // TODO for now if 3 guys als bust within one hand they are randomly thrown into the outPlayers_ vector, regardless
            // TODO of their stack....
            cleanUpEmptySeats();
        }

        // Check if tables are balanced, reseat elsewise
        //TODO check if this works buy logging something out
        if (tables_.size() > 1) movePlayersToTableAndMergeTables();

        handCount++;
    }
}

void Game::playOneHand(Table *table) {

    // give every player the first and second card
    table->cardDealer_->dealCards();

    // force antes and blinds
    table->cardDealer_->applyForcedBets(rules_);

#ifdef DEBUG
    outputFile << "###################### Table: " << table->getID() << " -- " << "is running: " << table->isRunning_
              << " -- dealer: " << table->players_[table->dealer_]->getID() << " ######################" << std::endl;
    outputFile << "//////////////// STACK ///// WAGER ///// CARDS  ///// TABLE-ID ////////////// " << std::endl;
    for (auto &player : table->players_) {  // Print player ids, cards and stacks
        outputFile << "// Player " << player->getID() << "|hands won: " << player->handsWon_ << ":     " << player->stack_ << "        " << player->wager_ << "        " <<
                  player->holeCard1_.getShortNameUnicode() << " " << player->holeCard2_.getShortNameUnicode()<<  "        " <<
                  player->tableID_ << std::endl;
    }
    outputFile << std::endl;
#endif

    // set the last raise amount to the size of the bigblind
    table->lastRaiseAmount_ = rules_->blindLevel_.bigBlind_;

    //  deal the rounds (PREFLOP, FLOP, TURN, RIVER)
    table->cardDealer_->dealRounds();

#ifdef DEBUG
    outputFile << "////////////////////////////// HAND FINISHED ////////////////////////////////" << std::endl;
    outputFile << "//////////////// STACK ///// WAGER ///// CARDS  ///// TABLE-ID ////////////// " << std::endl;
    for (auto &player : table->players_) {  // Print player ids, cards and stacks
        outputFile << "// Player " << player->getID() << ":     " << player->stack_ << "        " << player->wager_ << "        " <<
                  player->holeCard1_.getShortNameUnicode() << " " << player->holeCard2_.getShortNameUnicode()<<  "        " <<
                  player->tableID_ << std::endl;
    }
    outputFile << std::endl;
#endif
}

// moves busted players from table to game.playersOut_ to clear table seat and keep track of busted players
void Game::cleanUpEmptySeats() {
    for (auto &table : tables_) {
        // moves busted players to the game vector of busted players
        playersOut_.insert(std::end(playersOut_), std::begin(table->playersOut_), std::end(table->playersOut_));

        //clear table specific vector of busted players
        table->playersOut_.clear();
    }
}

//TODO add missing implementation
void Game::movePlayersToTableAndMergeTables()
{
    // First try to merge tables on fixed numberOfPlayersLeft (<9, <18, <27, <36)
    // At those fixed times in the game take the smallest table and split it onto other tables
    auto numRemainingPlayers = players_.size() - playersOut_.size();
    auto maxPlayersPerTable = ceil(float(players_.size()) / initialNumberOfTables_);

    int minRequiredTables = int(ceil((float)numRemainingPlayers / maxPlayersPerTable));

    int runningTables = 0;
    for (auto &table : tables_) {
        if (table->isRunning_) runningTables += 1;
    }
    //std::cout << "Debug; numRemainingPlayers: " << numRemainingPlayers << std::endl;
    //std::cout << "Debug; maxPlayersPerTable: " << maxPlayersPerTable << std::endl;

    //std::cout << "Debug; minim required: " << minRequiredTables << std::endl;

    //std::cout << "Debug; running Tables: " << runningTables << std::endl;

    // merge if condition true
    // TODO what if two tables at the same time need to be merged?....
    if (runningTables > minRequiredTables && runningTables != 1) {
        //std::cout << "Debug: Split needed!" << std::endl;

        // take the table with smallest population and split it onto the other ones
        // for now just iterate over the other tables, check if they arn't full and play one player at a time on the Cut-Off
        Table* tableToSplit = tables_[0];
        int indexOfSplitTable = 0;
        for (int i = 0; i < tables_.size(); i++) {
            if (tables_[i]->players_.size() < tableToSplit->players_.size()) {
                tableToSplit = tables_[i];
                indexOfSplitTable = i;
            }
        }

        //remove table to split from tables
        tables_[indexOfSplitTable]->isRunning_ = false;
        tables_.erase(tables_.begin() + indexOfSplitTable);     //also delete the pointer when not needed anylonger

        //split all players on this table and seat them on the other tables (for now always on Cut-Off position)
        while (!tableToSplit->players_.empty())
        {
            for (auto &table : tables_) {
                std::vector<Player*>::iterator it;
                it = table->players_.begin() + table->wrap(table->dealer_ - 1);

                //check if all players where reseated and if there is enough space for a new player on a table
                if (tableToSplit->players_.size() > 0 && table->players_.size() < maxPlayersPerTable) {
                    Player* player = tableToSplit->players_.back();
                    player->tableID_ = table->getID();                  //set the new table id because player was moved
                    table->players_.insert(it , player);
                    tableToSplit->players_.pop_back();  // removes last element from vector
                    //std::cout << "Debug: Player moved, players left: " << tableToSplit->players_.size() << std::endl;
                }
                else continue;
            }
        }

        // delete pointer if not needed any longer
        delete tableToSplit;
    }

    // 1.) Calculate number of players remaining in the game
    // 2.) Calculate the number of players per table
    //      - resulted integer is number of min. players that should be at any table
    //      - maximum is this number +1
    // get next open seat on short handed table starting with sb and move according player of
    // most populated table to this position

    int minNumPlayersPerTable = int(numRemainingPlayers / tables_.size());
    int maxNumPlayersPerTable = minNumPlayersPerTable + 1;                      // e.g. t1: 9, t2: 4, t3: 4

    // Now check if there is a table which has @minNumPlayersPerTable players seated at it
    // or if it has maxNumPlayersPerTable seated
    for (auto& table: tables_) {
        // reseat a player from the bigger populated table to the less populated table
        // doesn't matter if you start with the smallest or biggest because in the end it should be balanced
        if (table->players_.size() < minNumPlayersPerTable)
        {
            // get table where to move player from, take player immediately to the right of the dealer (Cut-Off) and place
            // him to the right of the dealer on the less populated table
            Table* mostPopulatedTable = getTableWithMostPlayers();
            int movedPlayerIndex = mostPopulatedTable->wrap(mostPopulatedTable->dealer_ - 1);
            Player* movedPlayer = mostPopulatedTable->players_[movedPlayerIndex];
            movedPlayer->tableID_ = table->getID();

            // kick this player out of the old table and add him to the new one
            mostPopulatedTable->players_.erase(mostPopulatedTable->players_.begin() + movedPlayerIndex);
            //std::cout << "Debug: movedPlayerIndex: " << movedPlayerIndex << std::endl;

            std::vector<Player*>::iterator it;
            it = table->players_.begin() + table->wrap(table->dealer_ - 1);
            table->players_.insert(it , movedPlayer);
            //std::cout << "Debug: Reseat successfull!" << "player: " << movedPlayer->getID() << std::endl;

        } else if (table->players_.size() > maxNumPlayersPerTable)
        {
            // get table where to seat player at, take player on big table immediately to the right of the dealer (Cut-Off) and place
            // him to the right of the dealer on the less populated table
            int movedPlayerIndex = table->wrap(table->dealer_ - 1);
            Player* movedPlayer = table->players_[movedPlayerIndex];

            // kick this player out of the old table and add him to the new one
            table->players_.erase(table->players_.begin() + movedPlayerIndex);

            Table* leastPopulatedTable = getTableWithLeastPlayers();
            movedPlayer->tableID_ = leastPopulatedTable->getID();

            std::vector<Player*>::iterator it;
            it = leastPopulatedTable->players_.begin() + leastPopulatedTable->wrap(leastPopulatedTable->dealer_ - 1);
            leastPopulatedTable->players_.insert(it , movedPlayer);
            //std::cout << "Debug: Reseat successfull!" << "player: " << movedPlayer->getID() << std::endl;
        }
    }
}

Table* Game::getTableWithLeastPlayers()
{
    Table* tableWithLeastPlayers = tables_[0];                  // starting with first table
    int smallestNumber = (int)tables_[0]->players_.size();      // starting with first table
    for (auto &table : tables_) {
        int numPlayers = (int)table->players_.size();
        if (numPlayers < smallestNumber) {
            smallestNumber = numPlayers;
            tableWithLeastPlayers = table;
        }

    }
    return tableWithLeastPlayers;
}

Table* Game::getTableWithMostPlayers()
{
    Table* tableWithMostPlayers = tables_[0];                  // starting with first table
    int biggestNumber = (int)tables_[0]->players_.size();      // starting with first table
    for (auto &table : tables_) {
        int numPlayers = (int)table->players_.size();
        if (numPlayers > biggestNumber) {
            biggestNumber = numPlayers;
            tableWithMostPlayers = table;
        }

    }
    return tableWithMostPlayers;
}

// TODO
// if we run x amounts of tournaments the seating should be mixed
// if we want to get rid of variance we have to to play 9 games with the same cards but with seat rotated by one
// each round
//
// This method only works well for tables with 9 players each because the seating with totalNumberOfPlayer % 9 != 0
// would make full tables and the remaining players would be seated on a different table, even if it would be 1 player
void Game::setupTablesAndSeatPlayers()
{
    //Generate random engine
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();

    // create x amount of tables to fit all players on them
    // Get the number of needed tables
    int num_of_players = rules_->totalNumberOfPlayers_;
    int num_of_tables = (num_of_players % rules_->maxNumPlayersPerTable_) ? num_of_players / rules_->maxNumPlayersPerTable_ + 1 :
                        num_of_players / rules_->maxNumPlayersPerTable_;

    initialNumberOfTables_ = num_of_tables;

    std::vector<Player*> tempPlayers(players_.begin(), players_.end());
    shuffle(tempPlayers.begin(), tempPlayers.end(), std::default_random_engine(seed));

    for (int i = 0; i < num_of_tables; i++)
    {
        // create a table, seat all players and add a dealer to the table
        Table *table = new Table();
        // give table a unique id
        table->setID(i);
        table->assignCardDealer(new CardDealer(table, rules_));

        for (int j = 0; j < rules_->maxNumPlayersPerTable_ && i * rules_->maxNumPlayersPerTable_ + j < num_of_players; j++)
        {
            // set players table id, so they know on which table they are
            Player* player = tempPlayers.at(i * rules_->maxNumPlayersPerTable_ + j);
            player->tableID_ = i;
            table->seatPlayer(player);
        }
        addTable(table);
    }
}

void Game::cleanUpGame() {

    for (auto &table : tables_) {
        delete table;
    }
    tables_.clear();
    playersOut_.clear();

    //reset rules
    rules_->blindLevel_ = rules_->blindLevels_.at(0);
}

std::vector<long> Game::getOverallGameResults() {
    return gameResults_;
}

std::vector<long> Game::getOverallHandsWonResults() {
    return handsWonResults_;
}

std::vector<double> Game::getMeanMoneyWonResults() {
    return meanMoneyWonResults_;
}

void Game::resetStatistics() {
    for (auto &player : players_) {
        player->handsWon_ = 0;
        player->chipsLostTotal_ = 0;
        player->chipsWonTotal_ = 0;
        player->handsLost_ = 0;
        player->tournament_1st = 0;
        player->tournament_2nd = 0;
        player->tournament_3rd = 0;
        player->tournament_4th = 0;
        player->tournament_5th = 0;
        player->tournament_6th = 0;
        player->tournament_7th = 0;

        player->overallFitness_ = -1;
        player->NUM_FOLDS = 0;
        player->NUM_CHECKS = 0;
        player->NUM_CALLS = 0;
        player->NUM_RAISES = 0;
        player->NUM_CALLS_PREFLOP = 0;
        player->NUM_RAISES_PREFLOP = 0;
        player->NUM_HANDS_PREFLOP = 0;
        player->preflop_action_was_counted = false;
    }
}



