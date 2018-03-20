//
// Created by Fabian Moik on 06.02.18.
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
#include "../ai/aiCaller.h"
#include "../ai/aiFolder.h"
#include "../ai/aiRaiser.h"


// VARIABLES

double weightsLoaded[NUM_OF_NN_WEIGHTS];
std::vector<unsigned> topo {15,8,3};
int NUM_AI_AGENTS = 1;
int NUM_ELITE_PLAYERS = 4;

bool sScoreGreaterThan(const std::pair<int,int>& a, const std::pair<int,int>& b) {
    return a.first > b.first;
}

bool sMoneyGreaterThan(const std::pair<double,long>& a, const std::pair<double,long>& b) {
    return a.first > b.first;
}

inline double my_clock(void) {
    struct timeval t;
    gettimeofday(&t, NULL);
    return (1.0e-6*t.tv_usec + t.tv_sec);
}

bool loadNeuralNetworkWeightsIntoArray() {
    printf("Loading nn_weights.DAT file...");
    memset(weightsLoaded, 0, sizeof(weightsLoaded));
    FILE * fin = fopen("../LookupTables/nn_weightsGen400.dat", "rb");
    if (!fin)
        return false;
    size_t bytesread = fread(weightsLoaded, sizeof(weightsLoaded), 1, fin);	// get the HandRank Array
    fclose(fin);
    printf("complete.\n\n");
    return true;
}

std::vector<Player*> runOneGeneration(std::vector<Player*> players, Rules* rules, int numElitePlayersToKeep, int numTourney);
Rules* defineRules();

int main() {

    //////////// SETUP ///////////////
    seedRandomFastWithRandomSlow();

    // Setup 2+2 method
    TPTEvaluator::initEvaluator();

    TPTEvaluator::loadPreflopTables();

    loadNeuralNetworkWeightsIntoArray();

    globalGameCounter = 0;
    /////////// PLAYER CREATION ///////////

    // Creating 44 players and our test agent
    std::vector<Player*> players;

    std::vector<double> weights;
    for (int i = 0; i < NUM_OF_NN_WEIGHTS; i++) {
        weights.push_back(weightsLoaded[i]);
    }
    // Create our agent
    Player *testAgent;
    for (int i = 0; i < NUM_AI_AGENTS; i++) {
        AIOwn *ai = new AIOwn();
        ai->topology_ = topo;
        testAgent = new Player(ai);
        testAgent->setID(i);
        players.push_back(testAgent);
        dynamic_cast<AIOwn *>(testAgent->getAI())->net_.setOutputWeights(weights);
    }


    // Creating the rest of the players

    for (int i = 1; i < 12; i++) {          // Fold
        Player *player;
        player = new Player(new AIFold());
        player->setID(i);
        players.push_back(player);
    }

    for (int i = 12; i < 23; i++) {         // Random
        Player *player;
        player = new Player(new AIRandom());
        player->setID(i);
        players.push_back(player);
    }

    for (int i = 23; i < 34; i++) {         // Call
        Player *player;
        player = new Player(new AICall());
        player->setID(i);
        players.push_back(player);
    }

    for (int i = 34; i < 45; i++) {         // Raiser
        Player *player;
        player = new Player(new AIRaiser());
        player->setID(i);
        players.push_back(player);
    }

    ///////////////// RUN GAMES /////////////////
    double start_time, end_time;
    start_time = my_clock();

    // Setup the rules
    Rules* rules = defineRules();

    // Keep 10% of the best players
    int numOfElitePlayersToKeep = NUM_ELITE_PLAYERS;


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

    ///////////////// EVALUATE FITNESSFUNCTIONS //////////////

    /// 1.) AVERAGE PLACEMENT PLACED
    std::vector<std::pair<int,int>> gameResultsPaired;
    std::vector<long> gameResults = game.getOverallGameResults();
    for (int i = 0; i < gameResults.size(); i++) {
        gameResultsPaired.push_back(std::make_pair(gameResults[i], i));
    }

    // now gameResultsPaired holds an ascending list of pairs - pair.first = value, pair.second = index
    std::sort(gameResultsPaired.begin(), gameResultsPaired.end());

    float total = 0.0;
    for (auto &pair : gameResultsPaired) {
        std::cout << "Player " << pair.second << " average place:  " << pair.first / (float)numTourney << std::endl;
        total +=  pair.first / (float)numTourney;
    }
    std::cout << "Total: " << total << std::endl;

    /// 2.) HANDS WON PLACED
    std::vector<std::pair<long,long>> handsWonPaired;
    std::vector<long> handsWonResults = game.getOverallHandsWonResults();
    for (int i = 0; i < handsWonResults.size(); i++) {
        handsWonPaired.push_back(std::make_pair(handsWonResults[i], i));
    }

    // now handsWonResults holds a list where .second is the playerID and .first is his hands won
    std::sort(handsWonPaired.begin(), handsWonPaired.end(), sScoreGreaterThan);

    float totalHandsWon = 0.0;
    for (auto &pair : handsWonPaired) {
        std::cout << "Player " << pair.second << " hands won:  " << pair.first << std::endl;
        totalHandsWon +=  pair.first;
    }
    std::cout << "Total hands won by all players: " << totalHandsWon << std::endl;

    /// 3.) MEAN MONEY WON
    std::vector<std::pair<double,int>> meanMoneyWonPaired;
    std::vector<double> meanMoneyWonResults = game.getMeanMoneyWonResults();
    for (int i = 0; i < meanMoneyWonResults.size(); i++) {
        meanMoneyWonPaired.push_back(std::make_pair(meanMoneyWonResults[i], i));
    }

    std::sort(meanMoneyWonPaired.begin(), meanMoneyWonPaired.end(), sMoneyGreaterThan);

    for (auto &pair : meanMoneyWonPaired) {
        std::cout << "Player " << pair.second << " mean money won:  " << pair.first << std::endl;
    }

    /// COMBINE ALL 3 TOGETHER
    std::vector<std::pair<double,int>> placeAndHandsWonPaired;

    for (int p = 0; p < gameResultsPaired.size(); p++) { // This ranks them according to their fitness
        gameResultsPaired[p].first = p + 1;
        handsWonPaired[p].first = p + 1;
        meanMoneyWonPaired[p].first = p + 1;
    }

    for (int i = 0; i < gameResultsPaired.size(); i++) {
        for (int j = 0; j < handsWonPaired.size(); j++) {
            for (int k = 0; k < meanMoneyWonPaired.size(); k++) {
                int index1 = gameResultsPaired[i].second;
                int index2 = handsWonPaired[j].second;
                int index3 = meanMoneyWonPaired[k].second;

                if (index1 == index2 && index2 == index3) {
                    double fitnessScore = gameResultsPaired[i].first * FITNESS_WEIGHT_1
                                          + handsWonPaired[j].first * FITNESS_WEIGHT_2
                                          + meanMoneyWonPaired[k].first * FITNESS_WEIGHT_3;

                    placeAndHandsWonPaired.push_back(std::make_pair(fitnessScore, index1));
                }
            }
        }
    }

    // sort descendingly (best player, lowest score)
    std::sort(placeAndHandsWonPaired.begin(), placeAndHandsWonPaired.end());

    for (auto &pair : placeAndHandsWonPaired) {
        std::cout << "Player " << pair.second << " fitnessScore:  " << pair.first << std::endl;
        totalHandsWon +=  pair.first;
    }

    game.resetStatistics();

    ////////////// GET BEST PLAYERS OF OWN-AI ////////////////
    // keep #num of elite players and return them
    std::vector<Player*> elitePlayers;
    elitePlayers.resize(numElitePlayersToKeep);
    int eliteIndex = 0;
    for (int i = 0; i < players.size(); i++) {
        if (placeAndHandsWonPaired[i].second < NUM_AI_AGENTS && eliteIndex < NUM_ELITE_PLAYERS) { //TODO not hardcoded but make it so only AIOwn players are selected
            elitePlayers.at(eliteIndex) = players[placeAndHandsWonPaired[i].second];
            eliteIndex++;
        }
    }

    // Returning the top players that we want to keep
    return elitePlayers;
}

Rules* defineRules() {
    // Set the rules
    Rules *rules = new Rules();
    rules->buyIn_ = 1500;
    rules->blindLevel_ = BlindLevel(10, 20, 3);

    // How many players do compeat in the game
    rules->totalNumberOfPlayers_ = GLOBAL_NUM_PLAYERS;

    // How many players should participate?
    // 9 players per table are allowed
    rules->maxNumPlayersPerTable_ = 9;

    return rules;
}

