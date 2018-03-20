/*
OOPoker

Copyright (c) 2010 Lode Vandevenne
All rights reserved.

This file is part of OOPoker.

OOPoker is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

OOPoker is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with OOPoker.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <fstream>

#define DEBUG 1
#define DEBUGRAISEONLY 1

extern std::map<int, std::string> bettingActionsAsString;
extern std::map<int, std::string> tableRoundsAsString;

extern std::ofstream outputFile;
extern std::ofstream raiseOutputFile;

extern int globalGameCounter;

/// GAME SPECIFIC DEFINES

#define GLOBAL_NUM_PLAYERS 9
#define NUM_OF_NN_WEIGHTS 163

/// TRAINING PARAMS
#define FITNESS_WEIGHT_1 0.7
#define FITNESS_WEIGHT_2 0
#define FITNESS_WEIGHT_3 0.3


// Error Handling
enum Error
{
    E_LESS_THAN_TWO_PLAYERS         = -2,
    E_NO_NEED_TO_SETTLE_BETS        = -3,
    E_NO_ACTIVE_PLAYER_AVAILABLE    = -4,
};

// Betting Type for neural network agent
enum BetType
{
    B_UNKNOWN_BET     = -1,
    B_SMALL_BET       = 0,
    B_MEDIUM_BET      = 1,
    B_LARGE_BET       = 2,
};

// Betting Decision constants

#define    BDC_LO_UPPER       0.06 // Upper boundary for SmallBet range
#define    BDC_LO_INSIDE      0.7  // Likelihood that SmallBet will be inside SmallBet range
#define    BDC_MED_LOWER      0.1  // Lower boundary for MedBet range
#define    BDC_MED_UPPER      0.2  // Upper boundary for MedBet range
#define    BDC_MED_INSIDE     0.6  // Likelihood that MedBet will be inside MedBet range
#define    BDC_MED_OUT_LO     0.1  // Likelihood that MedBet will be less than MedLower
#define    BDC_MED_OUT_HI     0.3  // Likelihood that MedBet will be greater than MedUpper
#define    BDC_HIGH_POINT     0.3  // Point which represents "standard" LargeBet
#define    BDC_HIGH_ABOVE     0.95 // Likelihood that LargeBet will be greater than HighPoint
#define    BDC_HIGH_BELOW     0.05 // Likelihood that LargeBet will be less than HighPoint
#define    BDC_HIGH_ALLIN     0.1   // Likelohood that LargeBet will be an all-in  -> this is implicitly included in the normalDistribution


//this can be used to get the correct index of previous and next players compared to you or the dealer
template<typename T, typename U>
T wrap(T a, U high) //wraps in range [0,high[, high NOT included!
{
  if(high == 0) return 0;

  if(a < 0) a += ((long)((-a) / (T)high) + 1) * ((T)high);
  if(a >= (T)high) a -= ((long)((a - high) / (T)high) + 1) * ((T)high);

  return a;
}

//convert any variable to a string
//usage: std::string str = valtostr(25454.91654654f);
template<typename T>
std::string valtostr(const T& val)
{
  std::ostringstream sstream;
  sstream << val;
  return sstream.str();
}

//convert string to a variable of type T
template<typename T>
T strtoval(const std::string& s)
{
  std::istringstream sstream(s);
  T val;
  sstream >> val;
  return val;
}

/*
getNearestRoundNumber: returns a number near the input, but it'll be a nice round value.
The result will always be smaller than or equal to the input. If the input is non-zero, the
number will never be zero.
It can return values such as 0, 1, 2, 5, 10, 20, 50, 100, 200, 500, 1000, 2000, ...
*/
int getNearestRoundNumber(int i);
