//
// Created by Corny on 09.11.2017.
//


/*  Level       SB          BB          Ante
 *  Level 1     10          20          3
 *  Level 2     15          30          4
 *  Level 3     25          50          6
 *  Level 4     40          80          10
 *  Level 5     60          120         15
 *  Level 6     80          160         20
 *  Level 7     100         200         25
 *  Level 8     150         300         25
 *  Level 9     200         400         25
 *  Level 10    300         600         50
 *  Level 11    400         800         50
 *  Level 12    500         1000        60
 *  Level 13    600         1200        75
 *  Level 14    800         1600        75
 *  Level 15    1000        2000        100
 *  Level 16    1500        3000        150
 *  Level 17    2000        4000        200
 *  Level 18    3000        6000        300
 *  Level 19    4000        8000        400
 *  Level 20    6000        12000       600
 *  Level 21    8000        16000       800
 *  Level 22    10000       20000       1000
 *  Level 23    15000       30000       1500
 *  Level 24    20000       40000       2000
 *  Level 25    25000       50000       2500
 *  Level 26    30000       60000       3000
 *
 *  How many hands per blind level (per 5min?) 12hands per blind level?
 *  TODO check back with hand history on pc
 *  */

#ifndef OWNPOKERSIMULATOR_RULES_H
#define OWNPOKERSIMULATOR_RULES_H

#include <vector>

enum Round
{
  R_PRE_FLOP,
  R_FLOP,
  R_TURN,
  R_RIVER,
  R_SHOWDOWN //not everything uses this (some things immediatly reset it to PRE_FLOP). This is reached when the river betting is settled and there are still multiple players in the game.
};

struct BlindLevel
{
    int smallBlind_;
    int bigBlind_;
    int ante_;
    BlindLevel(int s, int b, int a) : smallBlind_(s),
                                      bigBlind_(b),
                                      ante_(a)
    {
    }
};

struct Rules
{
    Rules();

    int buyIn_; //the starting stack
    int totalNumberOfPlayers_;
    int maxNumPlayersPerTable_;
    BlindLevel blindLevel_;

    std::vector<BlindLevel> blindLevels_ {BlindLevel(10,20,3),
                                          BlindLevel(15,30,4),
                                          BlindLevel(25,50,6),
                                          BlindLevel(40,80,10),
                                          BlindLevel(60,120,15),
                                          BlindLevel(80,160,20),
                                          BlindLevel(100,200,25),
                                          BlindLevel(150,300,25),
                                          BlindLevel(200,400,25),
                                          BlindLevel(300,600,50),
                                          BlindLevel(400,800,50),
                                          BlindLevel(500,1000,60),
                                          BlindLevel(600,1200,75),
                                          BlindLevel(800,1600,75),
                                          BlindLevel(1000,2000,100),
                                          BlindLevel(1500,3000,150),
                                          BlindLevel(2000,4000,200),
                                          BlindLevel(3000,6000,300),
                                          BlindLevel(4000,8000,400),
                                          BlindLevel(6000,12000,600),
                                          BlindLevel(8000,16000,800),
                                          BlindLevel(10000,20000,1000),
                                          BlindLevel(15000,30000,1500),
                                          BlindLevel(20000,40000,2000),
                                          BlindLevel(25000,50000,2500),
                                          BlindLevel(30000,60000,3000),
                                          BlindLevel(40000,80000,4000),
                                          BlindLevel(50000,100000,5000),
                                          BlindLevel(60000,1200000,6000)};


};

#endif //OWNPOKERSIMULATOR_RULES_H
