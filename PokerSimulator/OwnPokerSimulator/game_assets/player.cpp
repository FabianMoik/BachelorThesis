//
// Created by Corny on 05.11.2017.
//

#include "player.h"
#include "../ai/ai.h"
#include "../ai/aiOwn.h"
#include <cmath>
#include <sys/time.h>
#include <random>

Player::Player() : id_(-1), stack_(0), wager_(0), folded_(false), showdown_(false), tableID_(-1),
                   handsWon_(0), handsLost_(0), chipsWonTotal_(0), chipsLostTotal_(0), hallOfFameMember_(false),
                   overallFitness_(-1), NUM_FOLDS(0), NUM_CHECKS(0), NUM_CALLS(0), NUM_RAISES(0),
                   NUM_CALLS_PREFLOP(0), NUM_RAISES_PREFLOP(0), NUM_HANDS_PREFLOP(0), preflop_action_was_counted(false)
{

}

Player::Player(AI *ai) : id_(-1), ai_(ai), stack_(0), wager_(0), folded_(false), showdown_(false), tableID_(-1),
                         handsWon_(0), handsLost_(0), chipsWonTotal_(0), chipsLostTotal_(0), hallOfFameMember_(false),
                         overallFitness_(-1), NUM_FOLDS(0), NUM_CHECKS(0), NUM_CALLS(0), NUM_RAISES(0),
                         NUM_CALLS_PREFLOP(0), NUM_RAISES_PREFLOP(0), NUM_HANDS_PREFLOP(0), preflop_action_was_counted(false)
{

}

//Copy constructor
Player::Player(const Player& player)
{
    id_ = player.getID();
    stack_ = player.stack_;
    wager_ = player.wager_;
    holeCard1_ = player.holeCard1_;
    holeCard2_ = player.holeCard2_;
    folded_ = player.folded_;
    showdown_ = player.showdown_;
    tableID_ = player.tableID_;
    handsWon_ = 0;
    handsLost_ = 0;
    chipsWonTotal_ = 0;
    chipsLostTotal_ = 0;
    hallOfFameMember_ = 0;
    overallFitness_ = -1;
    NUM_FOLDS = 0;
    NUM_CHECKS = 0;
    NUM_CALLS = 0;
    NUM_RAISES = 0;
    NUM_CALLS_PREFLOP = 0;
    NUM_RAISES_PREFLOP = 0;
    NUM_HANDS_PREFLOP = 0;
    preflop_action_was_counted = false;

    ai_ = new AIOwn(*dynamic_cast<AIOwn*>(player.ai_));
}

Player::~Player()
{
    delete ai_;
}

void Player::setID(int id)
{
    this->id_ = id;
}
int Player::getID()const {
    return id_;
}

AI* Player::getAI() {
    return ai_;
}


bool Player::isAllIn() const
{
    return stack_ <= 0 && wager_ > 0;
}

bool Player::isOut() const
{
    return stack_ <= 0 && wager_ <= 0;
}

bool Player::hasFolded() const
{
    return folded_;
}

bool Player::canDecide() const
{
    return stack_ > 0 && !folded_;
}

// PLAYER ACTION FUNCTIONS

/*
If the amount is larger than the players stack, it puts him all-in
Returns the amount actually moved from the player to the table.
*/
long Player::placeMoney(long amount)
{
    if(amount <= stack_)
    {
        stack_ -= amount;
        wager_ += amount;
        return amount;
    }
    else
    {
        long result = stack_;
        wager_ += stack_;
        stack_ = 0;
        return result;
    }
}

Action Player::doTurn()
{
    return ai_->doTurn();
}
/*
 * BDC_LO_UPPER      = 0.06, // Upper boundary for SmallBet range - they range between 0% and 6% of players chips
    BDC_LO_INSIDE     = 0.7,  // Likelihood that SmallBet will be inside SmallBet range
    BDC_MED_LOWER     = 0.1,  // Lower boundary for MedBet range
    BDC_MED_UPPER     = 0.2,  // Upper boundary for MedBet range
    BDC_MED_INSIDE    = 0.6,  // Likelihood that MedBet will be inside MedBet range
    BDC_MED_OUT_LO    = 0.1,  // Likelihood that MedBet will be less than MedLower
    BDC_MED_OUT_HI    = 0.3,  // Likelihood that MedBet will be greater than MedUpper
    BDC_HIGH_POINT    = 0.3,  // Point which represents "standard" LargeBet
    BDC_HIGH_ABOVE    = 0.95, // Likelihood that LargeBet will be greater than HighPoint
    BDC_HIGH_BELOW    = 0.05, // Likelihood that LargeBet will be less than HighPoint
    BDC_HIGH_ALLIN    = 0.1*/

long Player::getBetAmount(int betType, long maxBet, long minBet) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);

    // Distribution Calculation
    double loSigma = 2.0 * (1 - BDC_LO_INSIDE) / (BDC_LO_INSIDE * sqrt(M_PI) * sqrt(2));
    double medSigmaLo = 2.0 * (BDC_MED_UPPER - BDC_MED_LOWER) * BDC_MED_OUT_LO / (BDC_MED_INSIDE * sqrt(M_PI) * sqrt(2));
    double medSigmaHi = 2.0 * (BDC_MED_UPPER - BDC_MED_LOWER) * BDC_MED_OUT_HI / (BDC_MED_INSIDE * sqrt(M_PI) * sqrt(2));
    double highSigmaHi = 2.0  * BDC_HIGH_ABOVE / (sqrt(M_PI) * sqrt(2)); //TODO how to calculate these two?
    double highSigmaLo = 2.0  * BDC_HIGH_BELOW / (sqrt(M_PI) * sqrt(2));

    std::uniform_real_distribution<> uniformRealDistribution(0,1);
    std::normal_distribution<double> normalLoSigmaDistribution(0.06, loSigma);
    std::normal_distribution<double> normalMedSigmaLoDistribution(0.1, medSigmaLo);
    std::normal_distribution<double> normalMedSigmaHiDistribution(0.2, medSigmaHi);
    std::normal_distribution<double> normalHighSigmaHiDistribution(0.3, highSigmaHi);
    std::normal_distribution<double> normalHighSigmaLoDistribution(0.3, highSigmaLo);

    long finalBet = 0;
    double percentile = uniformRealDistribution(generator);

    if (maxBet < minBet) return maxBet;

    if (betType == B_SMALL_BET) {
        if (percentile < BDC_LO_INSIDE){
            percentile = uniformRealDistribution(generator) * BDC_LO_UPPER;                                             //What does it mean?
        } else {
            percentile = BDC_LO_UPPER + abs(normalLoSigmaDistribution(generator));
        }
    } else if (betType == B_MEDIUM_BET) {
        if (percentile < BDC_MED_INSIDE){                                                                               // 60% of the time it's a normal medium Bet
            percentile = BDC_MED_LOWER + uniformRealDistribution(generator) * (BDC_MED_UPPER - BDC_MED_LOWER);
        } else if (percentile < BDC_MED_INSIDE + BDC_MED_OUT_LO) {                                                      // 10% of the time it's lower than medium
            percentile = BDC_MED_LOWER - abs(normalMedSigmaLoDistribution(generator));
        } else {                                                                                                        // 30% it's higher than medium
            percentile = BDC_MED_UPPER + abs(normalMedSigmaHiDistribution(generator));
        }
    } else if (betType == B_LARGE_BET) {
        if (percentile < BDC_HIGH_ABOVE){ //10% all-in can be includes here if std dev is 0.1 at 100%
            percentile = BDC_HIGH_POINT + abs(normalHighSigmaHiDistribution(generator));                                //What does it mean?
        } else {
            percentile = BDC_HIGH_POINT - abs(normalHighSigmaLoDistribution(generator));                                //What does it mean?
        }
    }

    //Create finat bet size
    finalBet = std::round(percentile * maxBet);
    if (finalBet < minBet) finalBet = minBet;
    else if (finalBet > maxBet) finalBet = maxBet;

    return finalBet;
}

void Player::setUpAI(std::vector<double> &input) const {
    ai_->fillInputValues(input);
}

// STATISTICS

double Player::VPIP() {
    if (NUM_HANDS_PREFLOP == 0) {
        return -1;
    }
    return (NUM_CALLS_PREFLOP + NUM_RAISES_PREFLOP) * 100.0 / NUM_HANDS_PREFLOP ;
}

double Player::PFR() {
    if (NUM_HANDS_PREFLOP == 0) {
        return -1;
    }
    return NUM_RAISES_PREFLOP * 100.0 / NUM_HANDS_PREFLOP;
}

double Player::AFQ() {
    if ((NUM_RAISES + NUM_CALLS + NUM_FOLDS) == 0) {
        return -1;
    }
    return NUM_RAISES * 100.0 / (NUM_RAISES + NUM_CALLS + NUM_FOLDS);
}
