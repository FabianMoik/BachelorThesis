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
int NUM_AI_AGENTS = 1;
int NUM_FOLD_AGENTS = 11;
int NUM_RANDOM_AGENTS = 11;
int NUM_CALL_AGENTS = 11;
int NUM_RAISE_AGENTS = 11;

float avg_place_interval_low = 1;
float avg_place_interval_high = 40;
float mean_money_interval_low = 0;
float mean_money_interval_high = 20000;   // per game
float hands_won_interval_low = 0;
float hands_won_interval_high = 12;    // per game


// Payout structure
float buy_in = 1;
float p_1st = 0.31;
float p_2nd = 0.215;
float p_3rd = 0.165;
float p_4th = 0.125;
float p_5th = 0.09;
float p_6th = 0.06;
float p_7th = 0.035;


std::ofstream myfile;

std::vector<int> generations;
std::vector<double> avg_place;
std::vector<double> hands_won;
std::vector<double> mean_money;
std::vector<double> o_fitness;
std::vector<double> vpip;
std::vector<double> pfr;
std::vector<double> afq;
std::vector<double> dollar_won;

bool sScoreGreaterThan(const std::pair<double,int>& a, const std::pair<double,int>& b) {
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
void evaluateFitnessAveragePlacement(Game &game, std::vector<Player*> &players, int numTourney);
void evaluateFitnessForMixedFitnessFunction(Game &game, std::vector<Player*> &population, int numTourney);
void evaluateFitnessForAllMixedFitnessFunction(Game &game, std::vector<Player*> &population, int numTourney);
std::vector<std::pair<double,int>> averageForAllAgents(std::vector<std::pair<double,int>> &pairedResults);

int main() {

    //////////// SETUP ///////////////
    seedRandomFastWithRandomSlow();

    // Setup 2+2 method
    TPTEvaluator::initEvaluator();

    TPTEvaluator::loadPreflopTables();

    myfile.open ("../Results/results.csv");
    myfile << "gens,average_placement,hands_won,mean_money,fitness\n";

    for (int i = 987; i <988; i++) {
        std::cout << "----- LOOP " << i << " FINISHED-----" << std::endl;
        std::string path = "../NN_weights/Testing/nn_weightsGen" + std::to_string(i) + ".dat";
        loadNeuralNetworkWeightsIntoArray(path);
        myfile << "gen" + std::to_string(i) + ",";
        generations.push_back(i);
        evaluateAgentVsOpponents();
    }

    for (int i = 0; i < generations.size(); i++) {
        std::cout << "gen" << generations.at(i) << "," << avg_place.at(i) << "," << hands_won.at(i) << "," << mean_money.at(i)
                  << "," << o_fitness.at(i) << "," << vpip.at(i) << "," << pfr.at(i) << "," << afq.at(i) << "," << dollar_won.at(i) << std::endl;
    }

    myfile.close();

    return 0;
}

void evaluateAgentVsOpponents() {
/////////// PLAYER CREATION ///////////

    // Creating 8 opponents and our test agent
    std::vector<Player*> players;

    std::vector<double> weights;
    for (int i = 0; i < NUM_OF_NN_WEIGHTS; i++) {
        weights.push_back(weightsLoaded[i]);
    }

    // Create our agent
    Player *testAgent;
    for (int i = 0; i < NUM_AI_AGENTS; i++) {
        AIOwn *ai = new AIOwn();
        ai->setName("Own");
        ai->topology_ = topo;
        testAgent = new Player(ai);
        testAgent->setID(i);
        players.push_back(testAgent);
        dynamic_cast<AIOwn *>(testAgent->getAI())->net_.setOutputWeights(weights);
    }

    // Creating the rest of the players

    for (int i = NUM_AI_AGENTS; i < NUM_AI_AGENTS + NUM_FOLD_AGENTS; i++) {          // Fold
        Player *player;
        player = new Player(new AIFold());
        player->setID(i);
        players.push_back(player);
    }

    for (int i = NUM_AI_AGENTS + NUM_FOLD_AGENTS; i < NUM_AI_AGENTS + NUM_FOLD_AGENTS + NUM_RANDOM_AGENTS; i++) {         // Random
        Player *player;
        player = new Player(new AIRandom());
        player->setID(i);
        players.push_back(player);
    }

    for (int i = NUM_AI_AGENTS + NUM_FOLD_AGENTS + NUM_RANDOM_AGENTS; i < NUM_AI_AGENTS + NUM_FOLD_AGENTS + NUM_RANDOM_AGENTS + NUM_CALL_AGENTS; i++) {         // Call
        Player *player;
        player = new Player(new AICall());
        player->setID(i);
        players.push_back(player);
    }

    for (int i = NUM_AI_AGENTS + NUM_FOLD_AGENTS + NUM_RANDOM_AGENTS + NUM_CALL_AGENTS; i < NUM_AI_AGENTS + NUM_FOLD_AGENTS + NUM_RANDOM_AGENTS + NUM_CALL_AGENTS + NUM_RAISE_AGENTS; i++) {         // Raiser
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

    runOneGeneration(players, rules, 100000);

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

void evaluateFitnessAveragePlacement(Game &game, std::vector<Player*> &players, int numTourney) {
    ///////////////// EVALUATE FITNESSFUNCTIONS //////////////

    /// 1.) AVERAGE PLACEMENT PLACED
    // Sort players by performance
    std::vector<std::pair<long,long>> gameResultsPaired;
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

        if (pair.second == 0) {
            myfile << "" + std::to_string(pair.first / (float)numTourney) + ",";
        }
    }
    std::cout << "Total: " << total << std::endl;
}

void evaluateFitnessForMixedFitnessFunction(Game &game, std::vector<Player*> &population, int numTourney) {
    ///////////////// EVALUATE FITNESSFUNCTIONS //////////////

    /// 1.) AVERAGE PLACEMENT PLACED
    // Sort players by performance
    std::vector<std::pair<long,long>> gameResultsPaired;
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
        if (pair.second == 0) {
            myfile << "" + std::to_string(pair.first) + ",";
        }
    }
    std::cout << "Total hands won by all players: " << totalHandsWon << std::endl;

    /// COMBINE ALL 3 TOGETHER
    std::vector<std::pair<double,int>> placeAndHandsWonPaired;

    for (int p = 0; p < gameResultsPaired.size(); p++) { // This ranks them according to their fitness
        gameResultsPaired[p].first = p + 1;
        handsWonPaired[p].first = p + 1;
    }

    for (int i = 0; i < gameResultsPaired.size(); i++) {
        for (int j = 0; j < handsWonPaired.size(); j++ ) {
            int index1 = gameResultsPaired[i].second;
            int index2 = handsWonPaired[j].second;

            if (index1 == index2) {
                double fitnessScore = gameResultsPaired[i].first * FITNESS_WEIGHT_1 +
                                      handsWonPaired[j].first * FITNESS_WEIGHT_2;

                placeAndHandsWonPaired.push_back(std::make_pair(fitnessScore, index1));
                population.at(index1)->overallFitness_ = fitnessScore;

                if (index1 == 0) {
                    myfile << "" + std::to_string(fitnessScore) + ",";
                }
            }
        }
    }

    // sort descendingly (best player, lowest score)
    std::sort(placeAndHandsWonPaired.begin(), placeAndHandsWonPaired.end());

    for (auto &pair : placeAndHandsWonPaired) {
        std::cout << "Player " << pair.second << " fitnessScore:  " << pair.first << std::endl;
    }
}

void evaluateFitnessForAllMixedFitnessFunction(Game &game, std::vector<Player*> &population, int numTourney) {
    ///////////////// EVALUATE FITNESSFUNCTIONS //////////////

    /// 1.) AVERAGE PLACEMENT PLACED
    // Sort players by performance
    std::vector<std::pair<double,int>> gameResultsPaired;
    std::vector<long> gameResults = game.getOverallGameResults();
    for (int i = 0; i < gameResults.size(); i++) {
        gameResultsPaired.push_back(std::make_pair(gameResults[i] / (float)numTourney, i));
    }

    std::vector<std::pair<double,int>> averagedResultsPaired;
    averagedResultsPaired = averageForAllAgents(gameResultsPaired);

    for (auto &result: averagedResultsPaired) {
        std::cout << result.first << ", " << result.second << std::endl;
    }

    // now gameResultsPaired holds an ascending list of pairs - pair.first = value, pair.second = index
    std::sort(gameResultsPaired.begin(), gameResultsPaired.end());

    float total = 0.0;
    for (auto &pair : gameResultsPaired) {
        std::cout << "Player " << pair.second << " average place:  " << pair.first << std::endl;
        total +=  pair.first;
        if (pair.second == 0) {
            myfile << "" + std::to_string(pair.first) + ",";
            avg_place.push_back(pair.first);
        }
    }
    std::cout << "Total: " << total << std::endl;

    /// 2.) HANDS WON PLACED
    std::vector<std::pair<double,int>> handsWonPaired;
    std::vector<long> handsWonResults = game.getOverallHandsWonResults();
    for (int i = 0; i < handsWonResults.size(); i++) {
        handsWonPaired.push_back(std::make_pair(handsWonResults[i] / (float)numTourney, i));
    }

    // Get the average result
    std::vector<std::pair<double,int>> averagedHandsWonPaired;
    averagedHandsWonPaired = averageForAllAgents(handsWonPaired);

    for (auto &result: averagedHandsWonPaired) {
        std::cout << result.first << ", " << result.second << std::endl;
    }

    // now handsWonResults holds a list where .second is the playerID and .first is his hands won
    std::sort(handsWonPaired.begin(), handsWonPaired.end(), sScoreGreaterThan);

    float totalHandsWon = 0.0;
    for (auto &pair : handsWonPaired) {
        std::cout << "Player " << pair.second << " hands won:  " << pair.first << std::endl;
        totalHandsWon +=  pair.first;
        if (pair.second == 0) {
            myfile << "" + std::to_string(pair.first) + ",";
            hands_won.push_back(pair.first);
        }
    }
    std::cout << "Total hands won by all players: " << totalHandsWon << std::endl;

    /// 3.) MEAN MONEY WON
    std::vector<std::pair<double,int>> meanMoneyWonPaired;
    std::vector<double> meanMoneyWonResults = game.getMeanMoneyWonResults();
    for (int i = 0; i < meanMoneyWonResults.size(); i++) {
        meanMoneyWonPaired.push_back(std::make_pair(meanMoneyWonResults[i], i));
    }

    // Get the average result
    std::vector<std::pair<double,int>> averagedMeanMoneyWonPaired;
    averagedMeanMoneyWonPaired = averageForAllAgents(meanMoneyWonPaired);

    for (auto &result: averagedMeanMoneyWonPaired) {
        std::cout << result.first << ", " << result.second << std::endl;
    }

    std::sort(meanMoneyWonPaired.begin(), meanMoneyWonPaired.end(), sMoneyGreaterThan);

    for (auto &pair : meanMoneyWonPaired) {
        std::cout << "Player " << pair.second << " mean money won:  " << pair.first << std::endl;
        if (pair.second == 0) {
            myfile << "" + std::to_string(pair.first) + ",";
            mean_money.push_back(pair.first);
        }
    }

    /// COMBINE ALL 3 TOGETHER
    std::vector<std::pair<double,int>> placeAndHandsWonPaired;

    for (int p = 0; p < gameResultsPaired.size(); p++) { // This ranks them according to their fitness
        gameResultsPaired[p].first = p + 1;
        handsWonPaired[p].first = p + 1;
        meanMoneyWonPaired[p].first = p + 1;
    }
    /*
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



                    placeAndHandsWonPaired.push_back(std::make_pair(fitnessScore, index1));
                    population.at(index1)->overallFitness_ = fitnessScore;

                    if (index1 == 0) {
                        myfile << "" + std::to_string(fitnessScore) + ",";
                        myfile << "" + std::to_string(population.at(index1)->VPIP()) + ",";
                        myfile << "" + std::to_string(population.at(index1)->PFR()) + ",";
                        myfile << "" + std::to_string(population.at(index1)->AFQ()) + "\n";
                        o_fitness.push_back(fitnessScore);
                        vpip.push_back(population.at(index1)->VPIP());
                        pfr.push_back(population.at(index1)->PFR());
                        afq.push_back(population.at(index1)->AFQ());

                    }
                }
            }
        }
    }*/

    for (int i = 0; i < averagedResultsPaired.size(); i++) {
        for (int j = 0; j < averagedHandsWonPaired.size(); j++ ) {
            for (int k = 0; k < averagedMeanMoneyWonPaired.size(); k++) {

                int index1 = averagedResultsPaired[i].second;
                int index2 = averagedHandsWonPaired[j].second;
                int index3 = averagedMeanMoneyWonPaired[k].second;

                if (index1 == index2 && index2 == index3) {

                    double averaged_score = convertToRange(averagedResultsPaired[i].first, avg_place_interval_low, avg_place_interval_high, 1, 0) * FITNESS_WEIGHT_1 +
                                            convertToRange(averagedHandsWonPaired[i].first, hands_won_interval_low, hands_won_interval_high, 0, 1) * FITNESS_WEIGHT_2 +
                                            convertToRange(averagedMeanMoneyWonPaired[i].first, mean_money_interval_low, mean_money_interval_high, 0, 1) * FITNESS_WEIGHT_3;



                    placeAndHandsWonPaired.push_back(std::make_pair(averaged_score, index1));
                    population.at(index1)->overallFitness_ = averaged_score;

                    if (index1 == 0) {
                        myfile << "" + std::to_string(averaged_score) + ",";
                        myfile << "" + std::to_string(population.at(index1)->VPIP()) + ",";
                        myfile << "" + std::to_string(population.at(index1)->PFR()) + ",";
                        myfile << "" + std::to_string(population.at(index1)->AFQ()) + "\n";
                        o_fitness.push_back(averaged_score);
                        vpip.push_back(population.at(index1)->VPIP());
                        pfr.push_back(population.at(index1)->PFR());
                        afq.push_back(population.at(index1)->AFQ());

                        // $ won
                        float price_pool = GLOBAL_NUM_PLAYERS * buy_in * 0.9;
                        float won = population.at(index1)->tournament_1st * p_1st * price_pool +
                                    population.at(index1)->tournament_2nd * p_2nd * price_pool +
                                    population.at(index1)->tournament_3rd * p_3rd * price_pool +
                                    population.at(index1)->tournament_4th * p_4th * price_pool +
                                    population.at(index1)->tournament_5th * p_5th * price_pool +
                                    population.at(index1)->tournament_6th * p_6th * price_pool +
                                    population.at(index1)->tournament_7th * p_7th * price_pool  - numTourney * buy_in;
                        dollar_won.push_back(double(won));
                    }
                }
            }
        }
    }

    // sort descendingly (best player, lowest score)
    std::sort(placeAndHandsWonPaired.begin(), placeAndHandsWonPaired.end());

    for (auto &pair : placeAndHandsWonPaired) {
        std::cout << "Player " << pair.second << " fitnessScore:  " << pair.first << "   ->   VPIP: " << population.at(pair.second)->VPIP()
                  << "  PFR: " << population.at(pair.second)->PFR() << "  AFQ: " << population.at(pair.second)->AFQ()
                  << " ----- " << "1st: " << population.at(pair.second)->tournament_1st << " 2nd: " << population.at(pair.second)->tournament_2nd
                  << " 3rd: " << population.at(pair.second)->tournament_3rd << " 4th: " << population.at(pair.second)->tournament_4th
                  << " 5th: " << population.at(pair.second)->tournament_5th << " 6th: " << population.at(pair.second)->tournament_6th << " 7th: "
                  << population.at(pair.second)->tournament_7th << std::endl;
        totalHandsWon +=  pair.first;

        // $ won
        float price_pool = GLOBAL_NUM_PLAYERS * buy_in * 0.9;
        float won = population.at(pair.second)->tournament_1st * p_1st * price_pool +
                    population.at(pair.second)->tournament_2nd * p_2nd * price_pool +
                    population.at(pair.second)->tournament_3rd * p_3rd * price_pool +
                    population.at(pair.second)->tournament_4th * p_4th * price_pool +
                    population.at(pair.second)->tournament_5th * p_5th * price_pool +
                    population.at(pair.second)->tournament_6th * p_6th * price_pool +
                    population.at(pair.second)->tournament_7th * p_7th * price_pool  - numTourney * buy_in;

        std::cout << "Prices won($): " << won << std::endl;
    }

    //std::vector<int> indices {0, 5, 16, 27, 38};

    std::vector<int> indices {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                              21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44};

    float acc_fold = 0;
    float acc_random = 0;
    float acc_call = 0;
    float acc_raise = 0;

    for (auto &i: indices) {
        // $ won
        float price_pool = GLOBAL_NUM_PLAYERS * buy_in * 0.9;
        float won = population.at(i)->tournament_1st * p_1st * price_pool +
                    population.at(i)->tournament_2nd * p_2nd * price_pool +
                    population.at(i)->tournament_3rd * p_3rd * price_pool +
                    population.at(i)->tournament_4th * p_4th * price_pool +
                    population.at(i)->tournament_5th * p_5th * price_pool +
                    population.at(i)->tournament_6th * p_6th * price_pool +
                    population.at(i)->tournament_7th * p_7th * price_pool  - numTourney * buy_in;

        std::cout << "Prices won($): " << won << std::endl;

        if (i <= 11 && i != 0) {
            acc_fold += won;
        } else if (i > 11 && i <= 22){
            acc_random += won;
        } else if (i > 22 && i <= 33) {
            acc_call += won;
        } else if (i > 33 && i >= 44) {
            acc_raise += won;
        }

    }

    std::cout << "Prices won($) by Folder: " << acc_fold / 11 << std::endl;
    std::cout << "Prices won($) by Random: " << acc_random / 11 << std::endl;
    std::cout << "Prices won($) by Caller: " << acc_call / 11 << std::endl;
    std::cout << "Prices won($) by Raiser: " << acc_raise / 11 << std::endl;


}

std::vector<std::pair<double,int>> averageForAllAgents(std::vector<std::pair<double,int>> &pairedResults) {

    std::vector<std::pair<double,int>> averaged_pairs;
    double own_sum = 0;
    long own_count = 0;
    double fold_sum = 0;
    long fold_count = 0;
    double random_sum = 0;
    long random_count = 0;
    double call_sum = 0;
    long call_count = 0;
    double raise_sum = 0;
    long raise_count = 0;


    for(auto &pair: pairedResults) {
        if (pair.second < NUM_AI_AGENTS) {
            own_sum += pair.first;
            own_count += 1;
        } else if (pair.second < NUM_AI_AGENTS + NUM_FOLD_AGENTS) {
            fold_sum += pair.first;
            fold_count += 1;
        } else if (pair.second < NUM_AI_AGENTS + NUM_FOLD_AGENTS + NUM_RANDOM_AGENTS) {
            random_sum += pair.first;
            random_count += 1;
        } else if (pair.second < NUM_AI_AGENTS + NUM_FOLD_AGENTS + NUM_RANDOM_AGENTS + NUM_CALL_AGENTS) {
            call_sum += pair.first;
            call_count += 1;
        } else if (pair.second < NUM_AI_AGENTS + NUM_FOLD_AGENTS + NUM_RANDOM_AGENTS + NUM_CALL_AGENTS + NUM_RAISE_AGENTS) {
            raise_sum += pair.first;
            raise_count += 1;
        }
    }

    averaged_pairs.push_back(std::make_pair(own_sum / own_count, 0));
    averaged_pairs.push_back(std::make_pair(fold_sum / fold_count, 1));
    averaged_pairs.push_back(std::make_pair(random_sum / random_count, 2));
    averaged_pairs.push_back(std::make_pair(call_sum / call_count, 3));
    averaged_pairs.push_back(std::make_pair(raise_sum / raise_count, 4));

    return averaged_pairs;
}



