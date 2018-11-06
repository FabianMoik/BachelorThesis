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
#include <iostream>
#include <fstream>
#include "../ai/aiRaiser.h"

const int NUM_OF_NN_WEIGHTS = (NUM_NEURONS_L1 + 1) * NUM_NEURONS_L2 + (NUM_NEURONS_L2 + 1) * NUM_NEURONS_L3;
double weightsLoaded[NUM_OF_NN_WEIGHTS];
std::vector<unsigned> topo {NUM_NEURONS_L1, NUM_NEURONS_L2, NUM_NEURONS_L3};
int NUM_AI_AGENTS = 9;
int NUM_OF_TOURNAMENTS = 1000;


float avg_place_interval_low = 1;
float avg_place_interval_high = 9;
float mean_money_interval_low = 0;
float mean_money_interval_high = 4000;   // per game
float hands_won_interval_low = 0;
float hands_won_interval_high = 10;    // per game


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

double convertToRange(double value, double from1, double to1, double from2, double to2) {
    return (value - from1) / (to1 - from1) * (to2 - from2) + from2;
}

bool loadNeuralNetworkWeightsIntoArray(std::string path) {
    printf("Loading nn_weights.DAT file...");
    memset(weightsLoaded, 0, sizeof(weightsLoaded));
    FILE * fin = fopen(path.c_str(), "rb");
    if (!fin)
        return false;
    size_t bytesread = fread(weightsLoaded, sizeof(weightsLoaded), 1, fin);	// get the HandRank Array
    fclose(fin);
    printf("complete.\n\n");
    return true;
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

void runOneGeneration(std::vector<Player*> players, Rules* rules, int numTourney);
void evaluateAgentVsOpponents();
void evaluateFitnessForAllMixedFitnessFunction(Game &game, std::vector<Player*> &population, int numTourney);

int main() {

    //////////// SETUP ///////////////
    seedRandomFastWithRandomSlow();

    // Setup 2+2 method
    TPTEvaluator::initEvaluator();

    TPTEvaluator::loadPreflopTables();

    evaluateAgentVsOpponents();

    return 0;
}

void evaluateAgentVsOpponents() {
/////////// PLAYER CREATION ///////////

    // Creating 8 opponents and our test agent
    std::vector<Player*> players;

    // Vector containing the generation numbers to test against
    std::vector<int> gens {1941, 1787, 901, 850, 1064, 1644, 574, 896, 1999};

    // Create our agent
    Player *testAgent;
    for (int i = 0; i < NUM_AI_AGENTS; i++) {
        std::vector<double> weights;
        std::string path = "../NN_weights/Testing/nn_weightsGen" + std::to_string(gens.at(i)) + ".dat";
        loadNeuralNetworkWeightsIntoArray(path);
        for (int i = 0; i < NUM_OF_NN_WEIGHTS; i++) {
            weights.push_back(weightsLoaded[i]);
        }


        AIOwn *ai = new AIOwn();
        ai->setName("BestPlayerOnEarth"+std::to_string(i));
        ai->topology_ = topo;
        testAgent = new Player(ai);
        testAgent->setID(i);
        players.push_back(testAgent);
        dynamic_cast<AIOwn *>(testAgent->getAI())->net_.setOutputWeights(weights);
    }

    ///////////////// RUN GAMES /////////////////
    double start_time, end_time;
    start_time = my_clock();

    // Setup the rules
    Rules* rules = defineRules();

    runOneGeneration(players, rules, NUM_OF_TOURNAMENTS);

#ifdef DEBUG
    outputFile.close();
#endif

    end_time = my_clock();
    double totalTime = end_time - start_time;
    std::cout << "Game simulation took: " << totalTime  << "seconds" << std::endl;
}


void runOneGeneration(std::vector<Player*> players, Rules* rules, int numTourney) {

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

    //evaluateFitnessAveragePlacement(game, players, numTourney);
    //evaluateFitnessForMixedFitnessFunction(game, players, numTourney);
    evaluateFitnessForAllMixedFitnessFunction(game, players, numTourney);

    game.resetStatistics();
}

void evaluateFitnessForAllMixedFitnessFunction(Game &game, std::vector<Player*> &population, int numTourney) {
    ///////////////// EVALUATE FITNESSFUNCTIONS ////////////

    /// 1.) AVERAGE PLACEMENT PLACED
    // Sort players by performance
    std::vector<std::pair<double,long>> gameResultsPaired;
    std::vector<long> gameResults = game.getOverallGameResults();
    for (int i = 0; i < gameResults.size(); i++) {
        gameResultsPaired.push_back(std::make_pair(gameResults[i] / (float)numTourney, i));
    }

    // now gameResultsPaired holds an ascending list of pairs - pair.first = value, pair.second = index
    std::sort(gameResultsPaired.begin(), gameResultsPaired.end());

    float total = 0.0;
    for (auto &pair : gameResultsPaired) {
        std::cout << "Player " << pair.second << " average place:  " << pair.first << std::endl;
        total +=  pair.first ;
    }
    std::cout << "Total: " << total << std::endl;

    /// 2.) HANDS WON PLACED
    std::vector<std::pair<double,long>> handsWonPaired;
    std::vector<long> handsWonResults = game.getOverallHandsWonResults();
    for (int i = 0; i < handsWonResults.size(); i++) {
        handsWonPaired.push_back(std::make_pair(handsWonResults[i] / (float)numTourney, i));
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

    for (int i = 0; i < gameResultsPaired.size(); i++) {
        for (int j = 0; j < handsWonPaired.size(); j++ ) {
            for (int k = 0; k < meanMoneyWonPaired.size(); k++) {

                int index1 = gameResultsPaired[i].second;
                int index2 = handsWonPaired[j].second;
                int index3 = meanMoneyWonPaired[k].second;

                if (index1 == index2 && index2 == index3) {
                    double fitnessScore = gameResultsPaired[i].first * FITNESS_WEIGHT_1 +
                                          handsWonPaired[j].first * FITNESS_WEIGHT_2 +
                                          meanMoneyWonPaired[k].first * FITNESS_WEIGHT_3;

                    double averaged_score = convertToRange(gameResultsPaired[i].first, avg_place_interval_low, avg_place_interval_high, 1, 0) * FITNESS_WEIGHT_1 +
                                            convertToRange(handsWonPaired[i].first, hands_won_interval_low, hands_won_interval_high, 0, 1) * FITNESS_WEIGHT_2 +
                                            convertToRange(meanMoneyWonPaired[i].first, mean_money_interval_low, mean_money_interval_high, 0, 1) * FITNESS_WEIGHT_3;



                    placeAndHandsWonPaired.push_back(std::make_pair(averaged_score, index1));
                    population.at(index1)->overallFitness_ = averaged_score;
                }
            }
        }
    }

    // sort descendingly (best player, lowest score)
    std::sort(placeAndHandsWonPaired.begin(), placeAndHandsWonPaired.end());

    for (auto &pair : placeAndHandsWonPaired) {
        std::cout << "Player " << pair.second << " fitnessScore:  " << pair.first << "   ->   VPIP: " << population.at(pair.second)->VPIP()
                  << "  PFR: " << population.at(pair.second)->PFR() << "  AFQ: " << population.at(pair.second)->AFQ() << std::endl;
        totalHandsWon +=  pair.first;
    }
}