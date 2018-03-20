//
// Created by Fabian Moik on 18.12.17.
//

#include "Neuron.h"
#include <cmath>
#include <iostream>
#include <random>

Neuron::Neuron(unsigned numOutputs, unsigned myIndex)
{
    for (unsigned c = 0; c < numOutputs; ++c) {
        outputWeights_.push_back(Connection());
        outputWeights_.back().weight = randomWeight();
    }

    myIndex_ = myIndex;
}

void Neuron::feedForward(const Layer &prevLayer)
{
    double sum = 0.0;

    // Sum the previous layer's outputs (which are our inputs)
    // Include the bias node from the previous layer.

    for (unsigned n = 0; n < prevLayer.size(); ++n) {
        sum += prevLayer[n].getOutputVal() *
               prevLayer[n].outputWeights_[myIndex_].weight;
    }

    outputVal_ = Neuron::activationFunction(sum);
}

double Neuron::activationFunction(double x)
{
    // tanh - output range [-1.0..1.0]
    // use sigmoid in futur? - really slow to compute

    // possible activation functions
    //atan(pi*x/2)*2/pi   24.1 ns
    //atan(x)             23.0 ns
    //1/(1+exp(-x))       20.4 ns
    //1/sqrt(1+x^2)       13.4 ns
    //erf(sqrt(pi)*x/2)    6.7 ns
    //tanh(x)              5.5 ns
    //x/(1+|x|)            5.5 ns
    return 1/(1+exp(-x)); //sigmoid
    //return tanh(x);
}

std::vector<Connection>& Neuron::getOutputWeights() {
    return outputWeights_;
}

double Neuron::randomWeight() {

   /*
    * for hyperbolic tangent units: sample a Uniform(-r,r) with r = sqrt(6 / (fanIn + fanOut))
    * where fanIn is the number of inputs of the unit and fanOut the number of outputweights
    *
    * for sigmoid units: use  r = 4 * sqrt(6 / (fanIn + fanOut))
    *
    *
    *  GOOD:    another alternative approach recently often used is (only for sigmoid?? because tanh could output
    *  negative values aswell):
    *  U([0,n]) * sqrt(2.0/n) - where n is the number of inputs of your NN
    */


    // TODO find a way to get the number of input neurons
    std::random_device rd;  //Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
    std::uniform_real_distribution<> dis(-1.0, 1.0);
    return dis(gen); //Each call to dis(gen) generates a new random double
}
