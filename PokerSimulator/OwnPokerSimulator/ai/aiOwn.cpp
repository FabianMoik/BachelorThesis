//
// Created by Fabian Moik on 18.12.17.
//

#include "aiOwn.h"
#include <random>

AIOwn::AIOwn(){

}

AIOwn::~AIOwn() {

}

AIOwn::AIOwn(const AIOwn& ai) {
    topology_ = ai.topology_;
    inputValues_ = ai.inputValues_;
    resultValues_ = ai.resultValues_;

    name_ = ai.name_;
    net_ = ai.net_;
}

void AIOwn::setTopology (std::vector<unsigned> topo) {
    topology_ = topo;
}

Action AIOwn::doTurn() {

    Action action;
    net_.feedForward(inputValues_);
    net_.getResults(resultValues_);


    // Draw from a discrete distribution to get the action
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(0.0, 1.0);

    double randomSample = dis(gen);
    int actionCommand = -1;

/*
    //Choose via distribution
    if (randomSample < resultValues_[0]) {
        actionCommand = 0;
    } else if (randomSample >= resultValues_[0] && randomSample < resultValues_[0] + resultValues_[1]) {
        actionCommand = 1;
    } else {
        actionCommand = 2;
    }
    */

    // Choose highest value
    if (resultValues_[0] > resultValues_[1] && resultValues_[0] > resultValues_[2]) {
        actionCommand = 0;
    } else if (resultValues_[1] > resultValues_[0] && resultValues_[1] > resultValues_[2]) {
        actionCommand = 1;
    } else {
        actionCommand = 2;
    }

    if (actionCommand == 0) {
        action = Action(A_FOLD);
    } else if (actionCommand == 1) {
        // Check outside if A_CALL is valid, if not then A_CHECK
        action = Action(A_CALL);
    } else if (actionCommand == 2) {
        //A raise could either be a small, medium or large raise
        //TODO change this to a more appropriate method
        action = Action(A_RAISE);
        if (resultValues_[2] < 0.4) {
            action.betType_ = B_SMALL_BET;
        } else if (resultValues_[2] >= 0.4 && resultValues_[2] < 0.6) {
            action.betType_ = B_MEDIUM_BET;
        } else {
            action.betType_ = B_LARGE_BET;
        }
    }
    //Debug purpose
    int a = 0;
    return action;
}

std::string AIOwn::getAIName() {
    return name_;
}

void AIOwn::setName(std::string name) {
    name_ = name;
};

void AIOwn::fillInputValues(std::vector<double> &input) {
    // size of input should equal the number of input neurons of the first layer

    if (input.size() == topology_[0]) {
        inputValues_ = input;
    } else {
        std::cerr << "AI:   too few features provided" << std::endl;
    }
}

std::vector<double> AIOwn::getResult() {
    return resultValues_;
}

