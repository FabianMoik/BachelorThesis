//
// Created by Fabian Moik on 09.02.18.
//

#include <iostream>
#include "../random.h"
#include "../hand_eval/2+2/2P2Evaluator.h"
#include "../hand_eval/handEvalCalc.h"
#include "../game_assets/card.h"
#include <vector>
#include <sys/time.h>
#include "../rules.h"
#include "../game_assets/player.h"
#include "../game_assets/game.h"
#include "../ai/aiRandom.h"

inline double my_clock(void) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (1.0e-6*t.tv_usec + t.tv_sec);
}

std::vector<Player*> runOneGeneration(std::vector<Player*> players, Rules* rules, int numElitePlayersToKeep, int numTourney);
Rules* defineRules();

int main() {

    //////////// SETUP ///////////////
    seedRandomFastWithRandomSlow();

    // Setup 2+2 method
    TPTEvaluator::initEvaluator();

    TPTEvaluator::loadPreflopTables();


    /////////// PLAYER CREATION ///////////

    // Creating 44 players and our test agent
    std::vector<Player*> players;

    // Creating the rest of the players
    for (int i = 0; i < 45; i++) {          // own random AI
        Player *player;
        player = new Player(new AIRandom());
        player->setID(i);
        players.push_back(player);
    }

    ///////////////// RUN GAMES /////////////////
    double start_time, end_time;
    start_time = my_clock();

    // Setup the rules
    Rules* rules = defineRules();

    // Keep 10% of the best players
    int numOfElitePlayersToKeep = rules->totalNumberOfPlayers_ / 10;

    runOneGeneration(players, rules, numOfElitePlayersToKeep, 100);

    end_time = my_clock();
    double totalTime = end_time - start_time;
    std::cout << "Full preflop table creation took: " << totalTime  << "seconds" << std::endl;

    return 0;
}

std::vector<Player*> runOneGeneration(std::vector<Player*> players, Rules* rules, int numElitePlayersToKeep, int numTourney) {

    // Create a game
    Game game;
    game.setRules(rules);

    // Add players to the game
    for (auto &player: players) {
        game.addPlayer(player);
    }

    double start_time, end_time;
    start_time = my_clock();

    // Play #num of games with the same agents
    for (long i = numTourney; i > 0; i--) {
        game.playGame();
        game.cleanUpGame();
        std::cout << "game " << i << " finsihed!" << std::endl;
    }
    end_time = my_clock();
    double time = end_time - start_time;
    std::cout << numTourney << " games took: " << time << std::endl;

    // Sort players by performance
    std::vector<std::pair<int,int>> gameResultsPaired;
    std::vector<int> gameResults = game.getOverallGameResults();
    for (int i = 0; i < gameResults.size(); i++) {
        gameResultsPaired.push_back(std::make_pair(gameResults[i], i));
    }

    // now gameResultsPaired holds an ascending list of pairs - pair.first = value, pair.second = index
    std::sort(gameResultsPaired.begin(), gameResultsPaired.end());

    // keep #num of elite players and return them
    std::vector<Player*> elitePlayers;
    elitePlayers.resize(numElitePlayersToKeep);
    for (int i = 0; i < numElitePlayersToKeep; i++) {
        elitePlayers.at(i) = players[gameResultsPaired[i].second];
    }

    float total = 0.0;
    for (auto &pair : gameResultsPaired) {
        std::cout << "Player " << pair.second << " average place:  " << pair.first / (float)numTourney << std::endl;
        total +=  pair.first / (float)numTourney;
    }
    std::cout << "Total: " << total << std::endl;

    // Returning the top players that we want to keep
    return elitePlayers;
}

Rules* defineRules() {
    // Set the rules
    Rules *rules = new Rules();
    rules->buyIn_ = 1500;
    rules->blindLevel_ = BlindLevel(10, 20, 3);

    // How many players do compeat in the game
    rules->totalNumberOfPlayers_ = 45;

    // How many players should participate?
    // 9 players per table are allowed
    rules->maxNumPlayersPerTable_ = 9;

    return rules;
}

