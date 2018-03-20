//
// Created by Fabian Moik on 18.12.17.
//

#ifndef OWNPOKERSIMULATOR_NEURALNET_H
#define OWNPOKERSIMULATOR_NEURALNET_H

#include <vector>
#include "Neuron.h"
#include <string>

class NeuralNet {
public:
    NeuralNet(const std::vector<unsigned> &topology);
    void feedForward(const std::vector<double> &inputVals);
    void getResults(std::vector<double> &resultVals) const;
    void softmaxActivation(Layer &layer);
    std::vector<double> getOutputWeights();
    void setOutputWeights(std::vector<double>);

private:
    std::vector<Layer> layers_; // m_layers[layerNum][neuronNum]
};

#endif //OWNPOKERSIMULATOR_NEURALNET_H
