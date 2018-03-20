//
// Created by Fabian Moik on 18.12.17.
//

#include "NeuralNet.h"
#include <assert.h>
#include <math.h>
#include <cmath>
#include <iostream>

NeuralNet::NeuralNet(const std::vector<unsigned> &topology)
{
    unsigned numLayers = topology.size();
    for (unsigned layerNum = 0; layerNum < numLayers; layerNum++) {
        layers_.push_back(Layer());

        // Defines the number of needed weight connections for each neuron in each layer
        // if it is a neuron of the last layer, no output connections are needed
        unsigned numOutputs = layerNum == topology.size() - 1 ? 0 : topology[layerNum + 1];

        // We have a new layer, now fill it with neurons, and
        // add a bias neuron in each layer (<= therefore).
        for (unsigned neuronNum = 0; neuronNum <= topology[layerNum]; ++neuronNum) {
            layers_.back().push_back(Neuron(numOutputs, neuronNum));
        }

        // Force the bias node's output to 1.0 (it was the last neuron pushed in this layer):
        // Value of bias is not so important is just has to be != 0
        layers_.back().back().setOutputVal(1.0);
    }
}

void NeuralNet::feedForward(const std::vector<double> &inputVals)
{
    assert(inputVals.size() == (layers_[0].size() - 1));

    // Assign (latch) the input values into the input neurons
    //TODO what does ++i do here instead of i++?
    for (unsigned i = 0; i < inputVals.size(); ++i) {
        layers_[0][i].setOutputVal(inputVals[i]);
    }

    // forward propagate
    // If we want to use softmax for the last layer, the activation needs to be done by the neural network
    // because all output values of the neurons need to be know to perform a softmax activation
    for (unsigned layerNum = 1; layerNum < layers_.size(); ++layerNum) {
        Layer &prevLayer = layers_[layerNum - 1];

        for (unsigned n = 0; n < layers_[layerNum].size() - 1; ++n) {
            layers_[layerNum][n].feedForward(prevLayer);
        }
    }

    //For the last layer use softmax activation
    softmaxActivation(layers_.back());
}

//returns a vector of all output weights in the order of the layers, with suborder of neurons in the layer
//it is just a string of weights similar to a human DNA
std::vector<double> NeuralNet::getOutputWeights() {
    std::vector<double> outputWeights;
    for (auto &layer: layers_) {
        for (auto &neuron: layer) {
            for (auto &weight: neuron.getOutputWeights()) {
                outputWeights.push_back(weight.weight);
            }
        }
    }
    return outputWeights;
}

// gets a vector of weigths as input and sets it in the same order as they are retrieved when calling getOutputWeights()
void NeuralNet::setOutputWeights(std::vector<double> outputWeights) {
    int index = 0;
    for (auto &layer: layers_) {
        for (auto &neuron: layer) {
            for (auto &weight: neuron.getOutputWeights()) {
                if (index >= outputWeights.size()) {
                    //Something went wrong
                    std::cerr << "NeuralNet: invalid index for setting output weights" << std::endl;
                    return;
                }
                weight.weight = outputWeights.at(index);
                index++;
            }
        }
    }
}

// Fills the rusults vector with the values of the last layer's neurons
// TODO differentiate between raise sizes and calculate a desired raise amount
void NeuralNet::getResults(std::vector<double> &resultVals) const
{
    resultVals.clear();

    for (unsigned n = 0; n < layers_.back().size() - 1; ++n) {
        resultVals.push_back(layers_.back()[n].getOutputVal());
    }
}

// This results in the output neurons summing up to 1
// Useful for a classification problem
void NeuralNet::softmaxActivation(Layer &layer) {
    //This is not the original softmax function because I just normalize the output to 1. the real one would use the exp
    // but there are problems when using the softmax with values between 0 ... 1
    double sum = 0;

    // size() - 1 because we don't want the bias neuron
    for (unsigned n = 0; n < layers_.back().size() - 1; ++n) {
        sum += layers_.back()[n].getOutputVal();
    }

    //Normalize values

    for (unsigned n = 0; n < layers_.back().size() - 1; ++n) {
        // round to two digits
        double softmax = layers_.back()[n].getOutputVal() / sum;
        double roundedOwnValue = std::floor(softmax * 100 + 0.5) / 100;
        layers_.back()[n].setOutputVal(roundedOwnValue);
    }
}