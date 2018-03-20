//
// Created by Fabian Moik on 18.12.17.
//

#ifndef OWNPOKERSIMULATOR_NEURON_H
#define OWNPOKERSIMULATOR_NEURON_H

#include <vector>

class Neuron;

typedef std::vector<Neuron> Layer;

struct Connection
{
    double weight;
    double deltaWeight; // needed for later?
};

class Neuron
{
public:
    Neuron(unsigned numOutputs, unsigned myIndex);
    void setOutputVal(double val) { outputVal_ = val; }

    //TODO use different activation function for output layer
    double getOutputVal() const { return outputVal_; }
    void feedForward(const Layer &prevLayer);
    std::vector<Connection>& getOutputWeights();

private:
    unsigned myIndex_;
    double outputVal_;
    std::vector<Connection> outputWeights_; // a value for each weight to the next neuron

    // maps the output value of a neuron to a range between -1..1 or 0...1
    double activationFunction(double x);

    // Create a random weight - decide which function to use for random
    static double randomWeight();


};

#endif //OWNPOKERSIMULATOR_NEURON_H
