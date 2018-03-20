//
// Created by Fabian Moik on 04.02.18.
//

#include <iostream>
#include "../game_assets/game.h"
#include "../ai/aiRandom.h"
#include "../ai/aiOwn.h"
#include "../ai/ai_checkfold.h"
#include <thread>
#include "../random.h"
#include "../hand_eval/2+2/2P2Evaluator.h"
#include <vector>

// Forward Declaration
Rules* defineRules();

std::vector<Player*> runOneGeneration(std::vector<Player*> players, Rules* rules, int numElitePlayersToKeep, int numTourney);

int main() {

    seedRandomFastWithRandomSlow();
    // Setup 2+2 method
    TPTEvaluator::initEvaluator();

    // How many tournaments
    int numOfTournaments = 100;

    // Setup the rules
    Rules* rules = defineRules();

    // Creating the players for the first generation
    std::vector<Player*> players;
    for (int i = 0; i < rules->totalNumberOfPlayers_; i++) {
        Player *player;
        player = new Player(new AIOwn());
        player->setID(i);
        players.push_back(player);
    }

    // Create a game
    Game game;

    // Add players to the game
    for (auto &player: players) {
        game.addPlayer(player);
    }

    clock_t start = clock();

    // Play #num of games with the same agents
    for (long i = numOfTournaments; i > 0; i--) {
        game.setRules(rules);
        game.playGame();
        game.cleanUpGame();
    }
    double time = ((double) clock() - start) / CLOCKS_PER_SEC;
    std::cout << numOfTournaments << " games took: " << time << std::endl;

    // Sort players by performance
    std::vector<std::pair<int,int>> gameResultsPaired;
    std::vector<int> gameResults = game.getOverallGameResults();
    for (int i = 0; i < gameResults.size(); i++) {
        gameResultsPaired.push_back(std::make_pair(gameResults[i], i));
    }

    // now gameResultsPaired holds an ascending list of pairs - pair.first = value, pair.second = index
    std::sort(gameResultsPaired.begin(), gameResultsPaired.end());

    for (auto &pair : gameResultsPaired) {
        std::cout << "Player " << pair.second << " average place:  " << pair.first / (float)numOfTournaments << std::endl;
    }

    return 0;
}

void testSetupTablesAndSeatPlayers(Game &game) {
    game.setupTablesAndSeatPlayers();
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