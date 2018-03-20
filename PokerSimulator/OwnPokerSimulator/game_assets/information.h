//
// Created by Fabian Moik on 17.12.17.
//

#ifndef OWNPOKERSIMULATOR_INFORMATION_H
#define OWNPOKERSIMULATOR_INFORMATION_H

#include "../rules.h"
#include "action.h"
#include "player.h"

class Info {
public:
    Rules* rules_;
    std::vector<Player*> players_;    // Information about the player can be accessed directly through the player itself

    Info();
    ~Info();

};


#endif //OWNPOKERSIMULATOR_INFORMATION_H
