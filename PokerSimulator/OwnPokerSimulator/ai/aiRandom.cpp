//
// Created by Fabian Moik on 11.12.17.
//


#include "aiRandom.h"
#include "../random.h"

Action AIRandom::doTurn()
{

    //TODO create a class where Information is saved that the AI can access about his player or the table in general
    int r = (int)(getRandom() * 100);

    if(r < 25) //check or fold
    {
        Action action = Action(Command((int)(getRandom()*2)));
        //std::cout << "AIRandom: check or fold command - " << action.command_ << std::endl;
        return action;
    }
    else if(r < 26) //all-in
    {
        Action action = Action(A_RAISE, (int)(getRandom()*500));
        //std::cout << "AIRandom: all_in command - " << action.command_ << " amount: " << action.amount_ << std::endl;
        return action;
    }
    else if(r < 70) //call
    {
        Action action = Action(A_CALL);
        //std::cout << "AIRandom: call command - " << action.command_ << std::endl;
        return action;
    }
    else /*if(r == 2)*/ //raise
    {
        Action action = Action(A_RAISE, (int)(getRandom()*500));
        //std::cout << "AIRandom: raise command - " << action.command_ << " amount: " << action.amount_ << std::endl;
        return action;
    }
}

std::string AIRandom::getAIName()
{
    return "Random";
}