// normal_distribution
#include <iostream>
#include <string>
#include <random>

int main()
{
    const int nrolls=10000;  // number of experiments
    const int nstars=100;    // maximum number of stars to distribute

    std::default_random_engine generator;
    std::normal_distribution<double> distribution(0,0.2);

    int p[10]={};

    for (int i=0; i<nrolls; ++i) {
        double number = distribution(generator);
        std::cout << number << std::endl;
    }

    std::cout << "normal_distribution (0,0.4):" << std::endl;

    for (int i=-2; i<2; ++i) {
        std::cout << i << "-" << (i+1) << ": ";
        std::cout << std::string(p[i]*nstars/nrolls,'*') << std::endl;
    }

    return 0;
}
