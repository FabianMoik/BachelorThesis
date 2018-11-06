#include <iostream>
#include "game_assets/game.h"
#include "ai/aiRandom.h"
#include "ai/aiOwn.h"
#include <thread>
#include <vector>
#include "random.h"
#include "hand_eval/2+2/2P2Evaluator.h"
#include <sys/time.h>
#include <random>
#include <math.h>
#include "hand_eval/handEvalCalc.h"
#include "hand_eval/pokereval.h"
#include "ai/aiCaller.h"
#include "ai/aiFolder.h"
#include "ai/aiRaiser.h"

bool sScoreGreaterThan(const std::pair<long,long>& a, const std::pair<long,long>& b) {
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

/// FORWARD DECLARATIONS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Rules* defineRules();

std::vector<Player*> runOneGeneration(std::vector<Player*> &population, std::vector<Player*> &players, Rules* rules, int numElitePlayersToKeep,
                                      int numTourney, std::vector<Player*> &hallOfFame);
std::vector<Player*> runOneGenerationWithMultiThreading(std::vector<Player*> players, int numElitePlayersToKeep,
                                                        int numTourney, int numThreads);
std::vector<long> startOneGameCicle(Game *game, std::vector<std::vector<long>> &resultVector, int id);
void evolvePlayers(std::vector<Player*> players, const std::vector<Player*> elitePlayers, double mutationLikelihood);

/// SAVE NN WEIGHTS
bool saveNeuralNetworkWeightsToFile(std::vector<double> weights);
bool saveNeuralNetworkWeightsOfAllAgentsToFile(std::vector<double> weights, int index);
void loadWeightsForAllAgents(std::vector<Player*> &population);

/// HALL OF FAME
void initializeHallOfFameWithOwnAi(std::vector<Player*> &hallOfFame);
void updateHallOfFame(std::vector<Player*> &players, std::vector<Player*> elite, std::vector<Player*> &hallOfFame);

/// FITNESS FUNCTIONS
std::vector<Player*> returnEliteForAveragePlacement(Game &game, std::vector<Player*> &players, int numElitePlayersToKeep,
                                                    int numTourney);
std::vector<std::pair<double,int>> sortPopulationForMixedFitnessFunction(Game &game, std::vector<Player*> &players, int numElitePlayersToKeep,
                                                        int numTourney);
std::vector<std::pair<double,int>> sortPopulationForAllMixedFitnessFunction(Game &game, std::vector<Player*> &players, int numElitePlayersToKeep,
                                                                         int numTourney);


/// PROPERTIES
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const int NUM_OF_NN_WEIGHTS = (NUM_NEURONS_L1 + 1) * NUM_NEURONS_L2 + (NUM_NEURONS_L2 + 1) * NUM_NEURONS_L3;

double weightsLoaded[NUM_OF_NN_WEIGHTS];

std::vector<unsigned> topo {NUM_NEURONS_L1, NUM_NEURONS_L2, NUM_NEURONS_L3};
int generation = 0;

int NUM_AI_AGENTS = 35;
int NUM_HALL_OF_FAME = GLOBAL_NUM_PLAYERS - NUM_AI_AGENTS;
// The number of elite player needs to be smaller than the hall of fame size
int NUM_ELITE_PLAYERS = int(ceil(GLOBAL_NUM_PLAYERS * 0.1));  //TODO Some calculations in the evolve methode use hardcoded values so change it
float MUTATION_SDV = 0.1;
float MUTATION_LIKELIHOOD = 0.08;
int numOfGenerations = 1001;
int numOfTournaments = 200;

/// PROGRAMM START
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main() {

    //////////// SETUP & INITIALIZATION ///////////////

    // Creating the HandRanks.dat lookup table if not yet created
    //PokerEval::InitTheEvaluator();

    seedRandomFastWithRandomSlow();

    // Setup 2+2 method
    bool success = TPTEvaluator::initEvaluator();

    if (!success) {
        std::cerr << "Wasn't able to load table !!! " << std::endl;
        return -1;
    }

    //Load Preflop Lookup Tables
    TPTEvaluator::loadPreflopTables();

    /////////// PLAYER CREATION ///////////

    std::cout << "TOPO: " << topo.at(0) << topo.at(1) << topo.at(2) << std::endl;
    std::cout << "WEIGHTS Count: " << NUM_OF_NN_WEIGHTS << std::endl;


    // Creating the players for the first generation
    std::vector<Player*> players;
    std::vector<Player*> hallOfFame;

    for (int i = 0; i < NUM_AI_AGENTS; i++) {
        Player *player;
        AIOwn *ai = new AIOwn();
        ai->setName(std::to_string(i));
        ai->setTopology(topo);
        player = new Player(ai);
        player->setID(i);
        players.push_back(player);
    }

    initializeHallOfFameWithOwnAi(hallOfFame);

    /// CREATING THE PLAYING POPULATION (players + hallOfFame)
    std::vector<Player*> population;
    population.reserve(players.size() + hallOfFame.size());
    population.insert(population.end(), players.begin(), players.end());
    population.insert(population.end(), hallOfFame.begin(), hallOfFame.end());

    loadWeightsForAllAgents(population);

    ///////////////// RUN GAMES /////////////////
    double start_time, end_time;
    start_time = my_clock();

    generation = 500;
    for (int i = 500; i < numOfGenerations; i++) {

        /// EVALUATION PHASE
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        if (generation % 20 == 0) {
            for (auto &agent: population) {
                saveNeuralNetworkWeightsOfAllAgentsToFile(dynamic_cast<AIOwn *>(agent->getAI())->net_.getOutputWeights(), agent->getID());

            }
        }

        // Setup the rules
        Rules* rules = defineRules();

        // Keep x% of the best players
        int numOfElitePlayersToKeep = NUM_ELITE_PLAYERS;

        // Get elite players of each generation
        std::vector<Player*> elite;
        elite = runOneGeneration(population, players, rules, numOfElitePlayersToKeep, numOfTournaments, hallOfFame);

        for (auto &p: elite) {
            std::cout << "Player " << p->getID() << " is in the ELITE" << std::endl;
        }

        /// EVOLUTION PHASE
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        std::vector<Player*> playersToEvolve(players.begin(), players.begin() + NUM_AI_AGENTS);
        evolvePlayers(playersToEvolve, elite, MUTATION_LIKELIHOOD);

        saveNeuralNetworkWeightsToFile(dynamic_cast<AIOwn *>(elite.at(0)->getAI())->net_.getOutputWeights());

        //saveNeuralNetworkWeightsToFile(dynamic_cast<AIOwn *>(players.at(0)->getAI())->net_.getOutputWeights());
        //loadNeuralNetworkWeightsIntoArray(); //This fills the global array

        delete rules;
        std::cout << "GENERATION-NR: " << i << " DONE" << std::endl;
        generation++;
    }

    end_time = my_clock();
    double totalTime = end_time - start_time;
    std::cout << "Full " << numOfGenerations << " generations took: " << totalTime  << "seconds" << std::endl;

    return 0;
}

std::vector<Player*> runOneGeneration(std::vector<Player*> &population, std::vector<Player*> &players, Rules* rules, int numElitePlayersToKeep,
                                      int numTourney, std::vector<Player*> &hallOfFame) {

    // Create a game
    Game game;
    game.setRules(rules);

    // Add players to the game
    for (auto &player: population) {
        game.addPlayer(player);
    }

    double start_time, end_time;
    start_time = my_clock();

    // Play @numTourney of tournaments with the same agents
    for (long i = numTourney; i > 0; i--) {
        game.playGame();
        game.cleanUpGame();
        std::cout << "Tournament (" << i << ") finished!" << std::endl;
    }
    end_time = my_clock();
    double time = end_time - start_time;
    std::cout << numTourney << " tournaments took: " << time  << " seconds to finish" << std::endl;

    std::vector<std::pair<double,int>> sorted_population_results = sortPopulationForAllMixedFitnessFunction(game, population, numElitePlayersToKeep, numTourney);


    ////////////// EXTRACT ELITE PLAYERS AND UPDATE HALL OF FAME ////////////////
    // keep #num of elite players and return them
    std::vector<Player*> elite_players;
    elite_players.resize(numElitePlayersToKeep);
    int eliteIndex = 0;
    int hallOfFameIndex = 0;
    for (int i = 0; i < population.size(); i++) {

        //Check if agant is hallOfFame member or not
        // If it is sort the hallOfFame
        if (!population[sorted_population_results[i].second]->hallOfFameMember_ && eliteIndex < numElitePlayersToKeep) {
            elite_players.at(eliteIndex) = population[sorted_population_results[i].second];
            eliteIndex++;
        } else if (population[sorted_population_results[i].second]->hallOfFameMember_){ // This sorts the hallOfFame
            hallOfFame.at(hallOfFameIndex) = population[sorted_population_results[i].second];
            hallOfFameIndex++;
        }
    }

    //UPDATE HALL OF FAME
    updateHallOfFame(population, elite_players, hallOfFame);

    // Check if the hall of fame update worked
    for (auto &player: population) {
        std::cout << "Player " << player->getID() << " ai: " << player->getAI()->getAIName() << std::endl;

    }

    // RESETTING THE GAME STATISTICS AFTER EACH GENERATION
    game.resetStatistics();

    // Returning the top players that we want to keep
    return elite_players;
}

std::vector<Player*> returnEliteForAveragePlacement(Game &game, std::vector<Player*> &players, int numElitePlayersToKeep,
                                                    int numTourney) {
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

    // Extract the elite players from the players
    std::vector<Player*> elitePlayers;
    elitePlayers.resize(numElitePlayersToKeep);
    for (int i = 0; i < numElitePlayersToKeep; i++) {
        elitePlayers.at(i) = players[gameResultsPaired[i].second];
    }
    return elitePlayers;
}

std::vector<std::pair<double,int>> sortPopulationForMixedFitnessFunction(Game &game, std::vector<Player*> &population, int numElitePlayersToKeep,
                                                    int numTourney) {
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
            }
        }
    }

    // sort descendingly (best player, lowest score)
    std::sort(placeAndHandsWonPaired.begin(), placeAndHandsWonPaired.end());

    for (auto &pair : placeAndHandsWonPaired) {
        std::cout << "Player " << pair.second << " fitnessScore:  " << pair.first  << "   ->   VPIP: " << population.at(pair.second)->VPIP()
                  << "  PFR: " << population.at(pair.second)->PFR() << "  AFQ: " << population.at(pair.second)->AFQ() << std::endl;
        totalHandsWon +=  pair.first;
    }

    return placeAndHandsWonPaired;
}

std::vector<std::pair<double,int>> sortPopulationForAllMixedFitnessFunction(Game &game, std::vector<Player*> &population, int numElitePlayersToKeep,
                                                                         int numTourney) {
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
        meanMoneyWonPaired[p].first = p +1;
    }

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
                }
            }
        }
    }

    // sort descendingly (best player, lowest score)
    std::sort(placeAndHandsWonPaired.begin(), placeAndHandsWonPaired.end());

    for (auto &pair : placeAndHandsWonPaired) {
        std::cout << "Player " << pair.second << " fitnessScore:  " << pair.first
                  << "   ->   VPIP: " << population.at(pair.second)->VPIP()
                  << "  PFR: " << population.at(pair.second)->PFR() << "  AFQ: " << population.at(pair.second)->AFQ() << std::endl;
        totalHandsWon +=  pair.first;
    }

    return placeAndHandsWonPaired;
}

/*
 * // This should later be used to create the pseudo code for the thesis
void RunEvolution(int num_generations, int num_players, int num_elite_player_to_keep, int num_tournaments) {

    std::vector<Player*> players;
    std::vector<Player*> hall_of_fame;

    // Creating players for the first generation

    for (int i = 0; i < num_players; i++) {
        Player *player;
        AIOwn *ai = new AIOwn();
        player = new Player(ai);
        player->setID(i);
        players.push_back(player);
    }

    // Initializing Hall of Fame
    initializeHallOfFameWithOwnAi(players, hall_of_fame);

    for (int i = 0; i < num_generations; i++) {

        // Elite players per generation
        std::vector<Player*> elite;

        elite = runOneGeneration(players, rules, num_elite_player_to_keep, num_tournaments, hall_of_fame);

        // Evolve agents
        double mutationLikelihood = 0.10;

        std::vector<Player*> playersToEvolve(players.begin(), players.begin() + num_players);
        evolvePlayers(playersToEvolve, elite, mutationLikelihood);

        generation++;
    }
}*/

void evolvePlayers(std::vector<Player*> players, const std::vector<Player*> elitePlayers, double mutationLikelihood) {
    // Should I use this as generator setup?
    //std::random_device rd;  //Will be used to obtain a seed for the random number engine
    //std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::discrete_distribution<int> distribution {4,25,6,1,1,1,1};
    std::uniform_int_distribution<int> uniformDistribution(0, int(elitePlayers.size() - 1));
    std::uniform_real_distribution<> uniformRealDistribution(0,1);
    std::normal_distribution<double> normalDistribution(0, MUTATION_SDV);

    //Before creating children save the elite players at the beginning of the players vector and
    // swap them with the current players there so you just mutate the rest
    std::vector<Player*> tempPlayers;

    for (auto &elitePlayer: elitePlayers) {
        tempPlayers.push_back(elitePlayer);
    }

    for (auto &player: players) {
        if(std::find(tempPlayers.begin(), tempPlayers.end(), player) == tempPlayers.end())
        {
            //player does not exist in tempPlayers, add him
            tempPlayers.push_back(player);
        }
    }

    //What if elitePlayers count = 1?

    // This creates new players from elite players
    // For a given number of elite players, new agents can be generated by combining 1 to #eliteplayers players
    //  -   for choosing the number of parents an exponential distribution is used, such that 2 is the most likely outcome
    //      but 1 or up to #eliteplayers is also possible
    //
    //  -   after this #parents parents are chosen from the elite players via a uniform distribution
    //      It is not allowed to chose the same player as parent twice

    for (long p = elitePlayers.size(); p < tempPlayers.size(); p++) {

        // get random number of parents according to exponential distribution
        int parentCount = distribution(generator) + 1; // because range is 0...6

        if ((parentCount) > elitePlayers.size()) {
            parentCount = int(elitePlayers.size());
        }

        std::vector<Player *> parents;
        std::vector<double> parentsEvolutionWeights;


        // Chose the parents from the elite pool
        //  - parents must be unique and can't be chosen twice
        std::vector<int> used_indices;
        for (int i = 0; i < parentCount; i++) {
            bool success = false;
            int index = -1;

            while (!success) {
                index = uniformDistribution(generator);

                if(std::find(used_indices.begin(), used_indices.end(), index) == used_indices.end()) {
                    used_indices.push_back(index);
                    success = true;
                }
            }

            parents.push_back(elitePlayers.at(index));

            // Give a weight to the parents influence when reproducing
            double weight = uniformRealDistribution(generator);
            parentsEvolutionWeights.push_back(weight);
        }

        //normalize influence weights
        double sum = 0.0;
        for (auto &weight: parentsEvolutionWeights)
            sum += weight;

        for (auto &weight: parentsEvolutionWeights)
            weight = weight / sum;

        //TODO check if the parents are choosen correctly with a print
        std::cout << "Choose " << parentCount << " parents. (";
        for (auto &i: used_indices) {
            std::cout << i << ", ";
        }
        std::cout << ")" << std::endl;

        // This section applys new weigths to the children
        //  1.)     get the current child's weights
        //  2.)     now go through all parents and accumulate the parents' weights for each position while applying the
        //          influence factor to the weight of the current parent.
        std::vector<double> childWeights = dynamic_cast<AIOwn *>(tempPlayers.at(p)->getAI())->net_.getOutputWeights();

        // Get all the child's weights
        for (int w = 0; w < childWeights.size(); w++) {
            double value = 0.0;
            // Go through parents and get their weights for the current child's weight position
            for (int par = 0; par < parents.size(); par++) {
                std::vector<double> parentWeights = dynamic_cast<AIOwn *>(parents.at(
                        par)->getAI())->net_.getOutputWeights();
                value += parentWeights[w] * parentsEvolutionWeights[par];
            }

            //add random noise with a likelihood
            double random = uniformRealDistribution(generator);

            if (random < mutationLikelihood) {
                //Choose a normal distribution with mean = 0 and std = 0.1
                // This promotes many small changes as opposed to a few large changes
                double noiseValue = normalDistribution(generator);

                if (value + noiseValue > 1) {
                    value = 1;
                } else if (value + noiseValue < -1) {
                    value = -1;
                } else {
                    value += noiseValue;
                }
            }
            childWeights.at(w) = value;
        }

        // give child the new weights
        dynamic_cast<AIOwn *>(tempPlayers.at(p)->getAI())->net_.setOutputWeights(childWeights);
    }
}

void initializeHallOfFameWithOwnAi(std::vector<Player*> &hallOfFame) {
    for (int i = NUM_AI_AGENTS; i < GLOBAL_NUM_PLAYERS; i++) {
        AIOwn *ai = new AIOwn();
        ai->setName(std::to_string(i));
        ai->setTopology(topo);
        Player *player;
        player = new Player(ai);
        player->setID(i);
        player->hallOfFameMember_ = true;
        hallOfFame.push_back(player);
    }
}

void updateHallOfFame(std::vector<Player*> &population, std::vector<Player*> elite, std::vector<Player*> &hallOfFame) {
    // Check if elite players performed better than worst hallOfFamePlayers, if so replace them
    // Clone the player that replaces the worst player and set its id to the replaced player's id.
    // Also set the hallOfFame flag to TRUE
    // also don't forget to replace the player in the players vector
    int j = 0;

    if (NUM_HALL_OF_FAME == 0) {return;}

    for (int i = 0; i < elite.size(); i++) {
        if (hallOfFame.at(NUM_HALL_OF_FAME - elite.size() + i)->overallFitness_ > elite.at(j)->overallFitness_) {
            // Copy elite player and replace it with current hallOfFame player
            Player *copyElitePlayer = new Player(*elite.at(j));

            // set id to old players id and set the hallOfFame flag
            copyElitePlayer->setID(hallOfFame.at(NUM_HALL_OF_FAME - elite.size() + i)->getID());
            copyElitePlayer->hallOfFameMember_ = true;
            hallOfFame.at(NUM_HALL_OF_FAME - elite.size() + i) = copyElitePlayer;
            population.at(copyElitePlayer->getID()) = copyElitePlayer;
            j++;
        }
    }
}

Rules* defineRules() {
    // Set the rules
    Rules *rules = new Rules();
    rules->buyIn_ = 1500;
    rules->blindLevel_ = BlindLevel(10, 20, 3);

    // How many players do compete in the game
    rules->totalNumberOfPlayers_ = GLOBAL_NUM_PLAYERS;

    // How many players should participate?
    // 9 players per table are allowed
    rules->maxNumPlayersPerTable_ = 9;

    return rules;
}

bool saveNeuralNetworkWeightsToFile(std::vector<double> weights) {
    /////////////////// SAVING /////////////////////////////
    // save weights array to file

    double weightsArray[NUM_OF_NN_WEIGHTS] = {0};
    for (int i = 0; i < weights.size(); i++) {
        weightsArray[i] = weights.at(i);
    }

    std::string filename = "../NN_weights/nn_weightsGen" + std::to_string(generation) + ".dat";
    FILE * fout = fopen(filename.c_str(), "wb");
    if (!fout) {
        printf("Problem creating the Output File!\n");
        return 1;
    }
    fwrite(weightsArray, sizeof(weightsArray), 1, fout);  // big write, but quick

    fclose(fout);
    return true;
}

bool saveNeuralNetworkWeightsOfAllAgentsToFile(std::vector<double> weights, int index) {
    /////////////////// SAVING /////////////////////////////
    // save weights array to file

    double weightsArray[NUM_OF_NN_WEIGHTS] = {0};
    for (int i = 0; i < weights.size(); i++) {
        weightsArray[i] = weights.at(i);
    }

    std::string filename = "../NN_weights/Snapshot/nn_weightsGen" + std::to_string(generation) + "-" + std::to_string(index) + ".dat";
    FILE * fout = fopen(filename.c_str(), "wb");
    if (!fout) {
        printf("Problem creating the Output File!\n");
        return 1;
    }
    fwrite(weightsArray, sizeof(weightsArray), 1, fout);  // big write, but quick

    fclose(fout);
    return true;
}

void loadWeightsForAllAgents(std::vector<Player*> &population) {
    printf("Loading nn_weights.DAT file...");
    for (auto &agent: population) {
        std::string path = "../NN_weights/Snapshot/nn_weightsGen500-" + std::to_string(agent->getID()) + ".dat";
        memset(weightsLoaded, 0, sizeof(weightsLoaded));
        FILE * fin = fopen(path.c_str(), "rb");
        if (!fin)
            return;
        size_t bytesread = fread(weightsLoaded, sizeof(weightsLoaded), 1, fin);
        fclose(fin);

        std::vector<double> weights;
        for (int i = 0; i < NUM_OF_NN_WEIGHTS; i++) {
            weights.push_back(weightsLoaded[i]);
        }
        dynamic_cast<AIOwn *>(agent->getAI())->net_.setOutputWeights(weights);
    }

    printf("complete.\n\n");
}

/// MULTITHREADED

std::vector<Player*> runOneGenerationWithMultiThreading(std::vector<Player*> players, int numElitePlayersToKeep, int numTourney, int numThreads) {

    std::vector<std::vector<long>> resultVector;
    resultVector.resize(numTourney);

    // Create a game
    std::vector<Game *> gameVector;
    std::vector<Rules *> rulesVector;

    for (int i = 0; i < numTourney; i++) {
        Game *game = new Game();
        gameVector.push_back(game);
        Rules *rules = defineRules();
        rulesVector.push_back(rules);
        game->setRules(rules);
    }

    // Add players to the game
    // Be patient while multithreading... we can't give the players vector to all threads because they would
    // manipulate all the pointers simultaniously!
    // SOLUTION: we want to create a new players pointer vector for each game and also new AI but copy the values of
    // the old ones -> copy constructor
    for (auto &game: gameVector) {
        for (auto &player: players) {
            Player *newPlayer = new Player(*player); //Copy the players
            newPlayer->setID(player->getID());
            game->addPlayer(newPlayer);
            newPlayer->getAI()->setName("New");
        }
    }

    double start_time, end_time;
    start_time = my_clock();

    // Play #num of games with the same agents
    std::vector<std::thread> t_vector;
    int numGames = 0;
    while (numGames < numTourney) {
        for (int i = 0; i < numThreads; i++) {
            t_vector.push_back(std::thread(startOneGameCicle, gameVector[numGames], std::ref(resultVector), numGames));
            numGames++;
        }

        for (auto &thread: t_vector) {
            thread.join();
        }
        if (numGames + t_vector.size() > numTourney) {
            numThreads = numTourney - numGames;
        }

        t_vector.clear();
    }


    //TODO when to delete what?
    for (auto &i : gameVector) {
        delete i;
    }

    end_time = my_clock();
    double time = end_time - start_time;
    std::cout << numTourney << " games took: " << time << std::endl;

    std::cout << "Got all results - ready to print" << std::endl;
    std::vector<int> gameResults;
    for (int i = 0; i < 45; i++) {
        gameResults.push_back(0);
    }

    for (int i = 0; i < numTourney; i++) {
        for (int j = 0; j < 45; j++) {
            gameResults[j] += resultVector[i][j];
        }
    }
    // Sort players by performance
    std::vector<std::pair<int,int>> gameResultsPaired;
    for (int i = 0; i < gameResults.size(); i++) {
        gameResultsPaired.push_back(std::make_pair(gameResults[i], i));
    }

    // now gameResultsPaired holds an ascending list of pairs - pair.first = value, pair.second = index
    std::sort(gameResultsPaired.begin(), gameResultsPaired.end());

    // keep #num of elite players and return them
    std::vector<Player*> elitePlayers;
    elitePlayers.resize(numElitePlayersToKeep);
    for (int i = 0; i < numElitePlayersToKeep; i++) {
        elitePlayers.at(i) = players[gameResultsPaired.at(i).second];
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

std::vector<long> startOneGameCicle(Game *game, std::vector<std::vector<long>> &resultVector, int id) {
    game->playGame();
    game->cleanUpGame();

    resultVector[id] = game->getOverallGameResults();
    std::cout << "Thread " << id << "finished game" << std::endl;

    return game->getOverallGameResults();
}

