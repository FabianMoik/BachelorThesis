//
// Created by Fabian Moik on 18.12.17.
//

#ifndef OWNPOKERSIMULATOR_AIOWN_H
#define OWNPOKERSIMULATOR_AIOWN_H

#include "ai.h"
#include "NN_agent/NeuralNet.h"

//Naive AI. This AI will do a random action, completely independent of its cards.
class AIOwn : public AI {
public:

    // Creating a Neural Network with {numNeuron/layer0, numNeuron/layer1, ...}
    //      -   for now just test it with some basic input value and generate a softmax output
    //      -   the output represents (fold, check/call, bet/raise)
    //          -   the raise output could be split into 3 parts (small r, medium r, large r) -> resulting in 5 output N.
    std::vector<unsigned> topology_ = {NUM_NEURONS_L1, NUM_NEURONS_L2, NUM_NEURONS_L3};
    std::vector<double> inputValues_;
    std::vector<double> resultValues_;

    std::string name_ = "Default";

    NeuralNet net_ = NeuralNet(topology_);

    Action doTurn() override;

    std::string getAIName() override;
    void setName(std::string name) override;
    void setTopology (std::vector<unsigned> topo);

    AIOwn();
    ~AIOwn();
    // Copy Constructor
    AIOwn(const AIOwn& ai);

    void fillInputValues(std::vector<double> &input) override;
    std::vector<double> getResult();

};

#endif //OWNPOKERSIMULATOR_AIOWN_H
