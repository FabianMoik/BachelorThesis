//
// Created by Fabian Moik on 08.12.17.
//

#include "cardDealer.h"
#include "table.h"
#include "../util.h"
#include "../random.h"
#include "../hand_eval/handEvalCalc.h"
#include "../hand_eval/2+2/2P2Evaluator.h"
#include "../hand_eval/pokermath.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                    ///HELPER FUNCTIONS AND STRUCTS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Used to help with the sorting process by @value (combinationValue, stack, wager ...)
struct SPlayer
{
    int index;
    long value;

    SPlayer(int index, long value)
            : index(index)
            , value(value)
    {
    }
};

//used for sorting players by best combination from best to worst
bool sPlayerGreaterThan(const SPlayer& a, const SPlayer& b)
{
    return a.value > b.value;
}

bool sPlayerSmallerThan(const SPlayer& a, const SPlayer& b)
{
    return a.value < b.value;
}

bool sScoreGreaterThan(const long& a, const long& b)
{
    return a > b;
}

struct SidePot
{
    long chips;
    std::vector<int> players;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                                                ///CLASS SPECIFIC
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CardDealer::CardDealer(Table *table, Rules* rules) : table_(table), rules_(rules)
{
}

CardDealer::~CardDealer()
{

}

void CardDealer::dealCards() {
    deck_.shuffle();

    // Deal 2 cards to each player
    for(auto &player : table_->players_) player->holeCard1_ = deck_.next();
    for(auto &player : table_->players_) player->holeCard2_ = deck_.next();
}

void CardDealer::dealRounds() {
    //deal
    for(int r = 0; r < 4 && table_->isRunning_; r++) //round: pre-flop to river
    {
        Round round = (Round)r;
        table_->turn_ = 0;
        table_->round_ = round;

#ifdef DEBUG
        outputFile << "////////////// BOARD(" << tableRoundsAsString.at(table_->round_) << ") //////////////" << std::endl;
#endif
        if(round == R_FLOP)
        {
            burnCard();
            table_->boardCard1_ = deck_.next();
            table_->boardCard2_ = deck_.next();
            table_->boardCard3_ = deck_.next();
#ifdef DEBUG
            outputFile << std::endl << "                 " << table_->boardCard1_.getShortNameUnicode() << "  "
                      << table_->boardCard2_.getShortNameUnicode() << "  "<< table_->boardCard3_.getShortNameUnicode() << "  "
                      << std::endl << std::endl;
#endif
        }
        else if(round == R_TURN)
        {
            burnCard();
            table_->boardCard4_ = deck_.next();
#ifdef DEBUG
            outputFile << std::endl << "                 " << table_->boardCard1_.getShortNameUnicode() << "  "
                      << table_->boardCard2_.getShortNameUnicode() << "  "<< table_->boardCard3_.getShortNameUnicode() << "    "
                      << table_->boardCard4_.getShortNameUnicode() << std::endl << std::endl;
#endif
        }
        if(round == R_RIVER)
        {
            burnCard();
            table_->boardCard5_ = deck_.next();
#ifdef DEBUG
            outputFile << std::endl << "                 " << table_->boardCard1_.getShortNameUnicode() << "  "
                      << table_->boardCard2_.getShortNameUnicode() << "  "<< table_->boardCard3_.getShortNameUnicode() << "  "
                      << table_->boardCard4_.getShortNameUnicode() << "    " << table_->boardCard5_.getShortNameUnicode() << std::endl << std::endl;
#endif
        }

        table_->current_ = getInitialPlayer(round);

        if (table_->current_ == E_LESS_THAN_TWO_PLAYERS) {
            // stop hand immediately
#ifdef DEBUG
            outputFile << "     ===> CardDealer: HAND STOPPED IMMEDIATELY (players < 2)" << std::endl;
#endif
            break;  // breaks out of for loop

        } else if (table_->current_ == E_NO_NEED_TO_SETTLE_BETS) {
#ifdef DEBUG
            outputFile << "     ===> CardDealer: NO_NEED_TO_SETTLE_BETS (0 or 1 player | > 0 all-in players | go to showdown)" << std::endl;
#endif
            // there is one or none player which can decide and there are > 0 all-in players -> get to showdown but no
            // settleBets() needed
            continue;
        } else {
#ifdef DEBUG
            outputFile << "( Initial Player: " << table_->players_[table_->current_]->getID() << ")" << std::endl;
#endif
            // let players do some action and betting
            settleBets();

            // only one player left in hand, all other players have folded
            if (getInitialPlayer(round) == E_LESS_THAN_TWO_PLAYERS) break;  // only one player left in hand, others got bluffed out

            // Reset the table_->lastRaiseAmount to the big blind for each round
            // reset the table_->extra_ to 0 on each round
            // Get the rules somehow!!
            //CHEAT FOR NOW
            table_->lastRaiseAmount_ = rules_->blindLevel_.bigBlind_;
            table_->extra_ = 0;
        }
    } //rounds

    // Set the showdown value of each player (if only one player left no showdown needed) //TODO is this property "showdown" really needed?
    for(auto &player : table_->players_)
    {
        player->showdown_ = true;
        if(player->folded_) player->showdown_ = false;
        if(getNumActivePlayers() <= 1) player->showdown_ = false;
    }

    // Calculates the winner, divides the pot and splits money on players
    determineWinnerAndSplitPot();

    for(auto &player : table_->players_)
    {
        player->folded_ = false;
        player->showdown_ = false;
    }

    // Move the dealer button by one position
    table_->dealer_ = table_->wrap(table_->dealer_ + 1);

    // kick out all players which have no money left and update the dealer button to a "non-out" player
    kickOutPlayers();

    //reset the board
    table_->boardCard1_ = Card();
    table_->boardCard2_ = Card();
    table_->boardCard3_ = Card();
    table_->boardCard4_ = Card();
    table_->boardCard5_ = Card();

}


void CardDealer::settleBets()
{
#ifdef DEBUG
    outputFile << "     ===> CardDealer: settleBets()" << std::endl;
#endif

    table_->lastRaiser_ = -1;
    int prev_current = -1; //the previous current player (used to detect when bets are settled)

    bool bets_running = true;

    //keeps running until all bets are settled
    while(bets_running)
    {
        if(betsSettled(prev_current))
        {
            //TODO Additionally check if the extra amount of an all-in not full raise was called by all remaining players
            for (auto &player: table_->players_) {
                //CHECK this here
            }
#ifdef DEBUG
            outputFile << "     ===> CardDealer: settleBets(): - BETS SETTLED"  << std::endl;
#endif

            break;
        }

        Player *player = table_->players_.at(table_->current_);

        // Here the AI should decide which action it want to play
        // In case of a NeuralNetwork Agent the player needs to provide an input feature vector
        // TODO activate if players are aiOwn
        if (dynamic_cast<AIOwn *>(player->getAI())) {
            if (player->getID() == 0) { //Debug purpose
                int b = 0;
            }
            provideInputForAI();
        }

        Action action = player->doTurn();

        //TRIGGER BETTING STRATEGY
        //@betType      -   smalle, medium or large?
        //@maxBet       -   players chip count (only other case when agent has more chips than any other agent) //TODO should be considered or not?
        //@minBet       -   either bigBlind or minimum raiseAmount
        long callAmountTemp = table_->getHighestWager() - player->wager_;
        if (action.command_ == A_RAISE && dynamic_cast<AIOwn *>(player->getAI())) {
            action.amount_ = player->getBetAmount(action.betType_, player->stack_, table_->lastRaiseAmount_ + callAmountTemp + table_->extra_);
        }

        //*********** ONLY FOR NN AGENTS *********************************
        // A_CALL and A_CHECK, if A_CALL is invalid, choose A_CHECK

        if (callAmountTemp == 0 && (action.command_ == A_CALL || action.command_ == A_FOLD)) {
            action = Action(A_CHECK);
        }

        //BETTING SYSTEM to decide the size of the raise/bet
        // When using the betting system and it chooses to take an invalid raise size, what to do?
        if (action.command_ == A_RAISE && action.amount_ < table_->lastRaiseAmount_ + callAmountTemp + table_->extra_) {
            action.amount_ = table_->lastRaiseAmount_ + callAmountTemp + table_->extra_;
        }

        //Also check if the raise amount is always smaller or equal to the stack size!
        // Also check if it would be a check or a Raise action
        if (action.command_ == A_RAISE && action.amount_ > (player->stack_ + player->wager_ - table_->getHighestWager())) {
            action.amount_ = player->stack_;
            if (action.amount_ <= callAmountTemp) {
                action.command_ = A_CALL;
            }
        }


        //****************************************************************
#ifdef DEBUG
        outputFile << "          --> Player " << player->getID() << ": " << bettingActionsAsString.at(action.command_) << " - " << action.amount_ <<
                   "    CPre(" << player->NUM_CALLS_PREFLOP << ") RPre(" << player->NUM_RAISES_PREFLOP << ") TPre(" << player->NUM_HANDS_PREFLOP << ")"
                   " --- TOTAL  ->  F: " << player->NUM_FOLDS << "  CH: " << player->NUM_CHECKS << "  CA: " << player->NUM_CALLS << "  R: " << player->NUM_RAISES <<
                   " --- VPIP: " << player->VPIP() << " PFR: " << player->PFR() << " AFQ: " << player->AFQ() << std::endl;
#endif
        //std::cout << "Player " << player->getID() << " wants to " << action.command_ << " for c: " << action.amount_ << std::endl;

        if(!action.isValidAction(player->stack_, player->wager_, table_->getHighestWager(), table_->lastRaiseAmount_, table_->extra_))
        {
#ifdef DEBUG
            outputFile << "               --> Player " << player->getID() << ": " << "INVALID ACTION - FORCED FOLD" << std::endl;
#endif
            action = Action(A_FOLD);
        }

        // Write the action to the players statistics and check if the action was a preflop action or postflop
        if (table_->round_ == R_PRE_FLOP) {
            if (!player->preflop_action_was_counted) {
                player->preflop_action_was_counted = true;
                switch (action.command_) {
                    case A_FOLD:
                        break;
                    case A_CHECK:
                        break;
                    case A_CALL:
                        player->NUM_CALLS_PREFLOP += 1;
                        break;
                    case A_RAISE:
                        player->NUM_RAISES_PREFLOP += 1;
                        break;
                }
                player->NUM_HANDS_PREFLOP += 1;
            }
        }

        switch (action.command_) {
            case A_FOLD:
                player->NUM_FOLDS += 1;
                break;
            case A_CHECK:
                player->NUM_CHECKS += 1;
                break;
            case A_CALL:
                player->NUM_CALLS += 1;
                break;
            case A_RAISE:
                player->NUM_RAISES += 1;
                break;
        }


        long callAmount = table_->getCallAmount();   // returns the call amount specific to the current player

        if (action.command_ == A_RAISE)
        {
            // if not valid allInAction then it was a normal valid raise action
            if(!action.isValidAllInAction(player->stack_, player->wager_, table_->getHighestWager(), table_->lastRaiseAmount_))
            {
                table_->lastRaiser_ = table_->current_;
                table_->lastRaiseAmount_ = action.amount_ - callAmount;
#ifdef DEBUG
                outputFile << "               --> Player " << player->getID() << ": " << "VALID RAISE ACTION (LR: "
                          << table_->players_.at(table_->lastRaiser_)->getID() << " A: " << table_->lastRaiseAmount_
                          << " E: " << table_->extra_ << ")" << std::endl;
#endif
            }
            else // it is an All-in action
            {
#ifdef DEBUG
                outputFile << "               --> Player " << player->getID() << ": " << "VALID ALL-IN ACTION ";
#endif
                // if it was a valid raise of twice the previous raise size, the all-in amount is set and the action is treated
                // as a normal raise
                // if it was for less than twice the previous raise size, the lastRaiseAmount stays the same but the
                // additional money of the all-in needs to be accounted in future raises of other players....

                // set the amount to the maximum of chip the player can bet this round
                action.amount_ = player->stack_;
                long raiseAmount = action.amount_ - callAmount; // amount is the total amount you wager (call amount + raise amount)

                //valid raise
                if (raiseAmount >= table_->lastRaiseAmount_) {
                    table_->lastRaiser_ = table_->current_;
                    table_->lastRaiseAmount_ = raiseAmount;
#ifdef DEBUG
                    outputFile << "(LR: " << table_->players_.at(table_->lastRaiser_)->getID() << " A: " << table_->lastRaiseAmount_
                              << " E: " << table_->extra_ << ") - full raise" << std::endl;
#endif
                }
                else {
                    table_->extra_ = raiseAmount;               // only set if it is not a real full raise
#ifdef DEBUG
                    if (table_->lastRaiser_ != -1) {
                        outputFile << "(LR: " << table_->players_.at(table_->lastRaiser_)->getID() << " A: " << table_->lastRaiseAmount_
                                  << " E: " << table_->extra_ << ") - no full raise (extra)" << std::endl;
                    } else {
                        outputFile << "(LR: " << "NONE" << " A: " << table_->lastRaiseAmount_
                                  << " E: " << table_->extra_ << ") - no full raise (extra)" << std::endl;
                    }

#endif
                }
            }
        }
        else if (table_->lastRaiser_ == -1 && (action.command_ == A_CALL || action.command_ == A_CHECK))
        {
            table_->lastRaiser_ = table_->current_;
#ifdef DEBUG
            outputFile << "               --> Player " << player->getID() << ": "
                      << " VALID CALL/CHECK ACTION (LR: " << table_->players_.at(table_->lastRaiser_)->getID() << " A: " << table_->lastRaiseAmount_
                      << " E: " << table_->extra_ << ")" << std::endl;
#endif

        }

        applyAction(action, callAmount);

        prev_current = table_->current_;
        table_->current_ = getNextActivePlayer(table_->current_);
        table_->turn_++;
    } //while bets running

    for (auto &player : table_->players_) {  // reset the statistics flag
        player->preflop_action_was_counted = false;
    }

#ifdef DEBUG
    outputFile << "//////////////// STACK ///// WAGER ///// CARDS  ///// TABLE-ID ////////////// " << std::endl;
    for (auto &player : table_->players_) {  // Print player ids, cards and stacks
        outputFile << "// Player " << player->getID() << ":     " << player->stack_ << "        " << player->wager_ << "        " <<
                   player->holeCard1_.getShortNameUnicode() << " " << player->holeCard2_.getShortNameUnicode()<<  "        " <<
                   player->tableID_ << std::endl;
    }
    outputFile << std::endl;
#endif
}


//Checks whether all bets are settled (so that the game can go to the next round (flop, ...)
bool CardDealer::betsSettled(int prev_current)
{
    if(table_->current_ < 0) return true; //there was no next player found during settling the bets, indicating it's done
    if(getNumDecidingPlayers() == 0) return true;   // everyone either folded or is all-in or out
    if(getNumActivePlayers() < 2) return true;      // player could be all_in or yet to decide -> player wins the pot immediately

    if(table_->lastRaiser_ < 0)
    {
        return false;   //this means the first players folded, lastRaise_ gets valid value when someone calls, checks or
        // raises, and if nobody calls and everyone folds, the getNumActivePlayers < 2 check will
        // eventually trigger, which means that the big blind wins the pot uncontested
    }
    else
    {
        // check if the last raiser is the current player, or if we skipped the raiser which is between prev_current and
        // current (due to him being all-in)
        int i = prev_current;
        while(i != table_->current_)
        {
            i = wrap(i + 1, table_->players_.size());
            if(i == table_->lastRaiser_) return true;
        }
        return false;
    }
}

static int preflopN = 0;
static int flopN = 0;
static int turnN = 0;
static int riverN = 0;


// creates a vector of features that the AI can be fed with
void CardDealer::provideInputForAI() {
    std::vector<double> input;
    Player* player = table_->players_.at(table_->current_);

    // Normalization variables
    // make bigBlind a double for calculation
    // TODO values should be normalised between -1...1 or between 0...1
    // TODO because elsewise a value > 4 would yield ~1 for tanh and then the
    // TODO output of all neurons of the hidden layer would be 1
    // TODO yielding a value ~1 for the output aswell
    // TODO so Normalization is necessary
    // ***********************************+
    // Possible future features:
    // TODO give action restrictions (if agent can only call or raise, don't let him fold...)
    // TODO action on the table so far
    // TODO which betting turn they are on !!!!

    double bigBlind = rules_->blindLevel_.bigBlind_;
    double numBBInGame = rules_->buyIn_ * rules_->totalNumberOfPlayers_ / bigBlind;

    /////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////
    // Effective Hand Strength

    std::vector<Card> holeCards = {player->holeCard1_, player->holeCard2_};

    std::vector<Card> boardCards;

    switch (table_->round_) {
        case R_PRE_FLOP: boardCards = {}; preflopN++; break;
        case R_FLOP: boardCards = {table_->boardCard1_, table_->boardCard2_, table_->boardCard3_};  flopN++; break;
        case R_TURN: boardCards = {table_->boardCard1_, table_->boardCard2_, table_->boardCard3_, table_->boardCard4_}; turnN++; break;
        case R_RIVER: boardCards = {table_->boardCard1_, table_->boardCard2_, table_->boardCard3_, table_->boardCard4_, table_->boardCard5_}; riverN++; break;
        default:boardCards = {};
    }

    // When it comes to the numOpponents there are some tricky cases where I don't yet know how to deal with them!
    // E.g. what if three guys all-in pre and we are on river vs one guy who is all-in since that round?
    // numOpp would be 0 where it is effectively 1
    // On the other hand if we take the getNumActivePlayers we would consider early all-ins as active...
    // But this more pesimistic approach is better for now
    int numOpponents = getNumActivePlayers() - 1; // because we are active aswell
    int numSamples = 1000;
    double winChange;
    if (table_->round_ == R_PRE_FLOP) {
        winChange = getEHSVsNOpponentsPreflop(holeCards, numOpponents);
    } else {
        winChange = getEHSVsNOpponents(holeCards, boardCards, numOpponents, numSamples);
    }
    input.push_back(winChange);


    /////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////

    // Round of the game (PREFLOP, FLOP, TURN, RIVER)
    input.push_back(table_->round_ / 3.0);

    // Betting turns (how many turns of betting for the current round -> max rounds is hard to tell, but probably
    // never more than 20)
    //input.push_back(table_->turn_ / 20.0);

    // Chip count in BB / numBBInGame
    input.push_back(player->stack_ / bigBlind / numBBInGame);

    // Chips in Pot in BB - TODO later change this to the effective number of chips the agent can win
    input.push_back(table_->getPot() / bigBlind / numBBInGame);

    // Chips to call in BB
    input.push_back(table_->getCallAmount() / bigBlind / numBBInGame);

    //TODO check
    // Total number of chips in game in BB
    // after normalisation with this value the feature is reduntant?
    // input.push_back(numBBInGame);

    // Number of opponents - number of agents currently in the hand
    // -1 because ourselves is also an active player
    // TODO should I count currently all-in agents as active or not? - by now not active
    input.push_back((getNumDecidingPlayers() - 1) / 1.0 / rules_->maxNumPlayersPerTable_ );

    // position of hero
    // number of hands remaining until the current player receives the dealer button
    // Normalaized with rules_->maxNumPlayersPerTable_ or with "rules_->maxNumPlayersPerTable_ - 1 " because this is max value
    input.push_back(table_->getPositionOfPlayer(table_->current_) / 1.0 / (rules_->maxNumPlayersPerTable_ - 1));

    // Chip count of all opponents in BB
    // the first entry is always the chip count of the next player left to the current agent
    // What about player which already have folded?... agent should get this pattern because of relative position to dealer

    int startingIndex = table_->current_;
    for (int i = 1; i < table_->players_.size(); i++) {
        int playerIndex = table_->wrap(startingIndex + i);
        double chipCount = table_->players_.at(playerIndex)->stack_ / bigBlind;
        input.push_back(chipCount / numBBInGame);
    }

    // Fill the rest with 0 for empty seats on the table (#maxNumberOfPlayerPerTable features are needed)
    for (long i = table_->players_.size(); i < rules_->maxNumPlayersPerTable_; i++) {
        input.push_back(0);
    }

    // Number of players left in the tournament
    input.push_back(table_->numPlayersLeftInTournament / table_->numTotalPlayers);

    // Opponent model
    // we norm it with / 100.0 because the properties are in %
    int oppStartingIndex = table_->current_;
    for (int i = 1; i < table_->players_.size(); i++) {
        int playerIndex = table_->wrap(oppStartingIndex + i);

        double vpip = table_->players_.at(playerIndex)->VPIP();
        double pfr = table_->players_.at(playerIndex)->PFR();
        double afq = table_->players_.at(playerIndex)->AFQ();

        if (vpip == -1) { vpip = -100; }
        if (pfr == -1) { pfr = -100; }
        if (afq == -1) { afq = -100; }

        input.push_back(vpip / 100.0);
        input.push_back(pfr / 100.0);
        input.push_back(afq / 100.0);
        //std::cout << "OPP-M.:   P.-ID: " << table_->players_.at(playerIndex)->getID() << " - VPIP: " << vpip / 100.0
          //        << " - PFR: " << pfr / 100.0 << " - AFQ: " << afq / 100.0 << std::endl;
    }

    // Fill the rest with 0 for empty seats on the table
    for (long i = table_->players_.size(); i < rules_->maxNumPlayersPerTable_; i++) {
        input.push_back(-1);
        input.push_back(-1);
        input.push_back(-1);
    }

    player->setUpAI(input);
}

void CardDealer::applyAction(const Action& action, long callAmount)
{
    Player *player = table_->players_.at(table_->current_);
    player->lastAction_ = action;

    if(action.command_ == A_FOLD)
    {
        player->folded_ = true;
    }
    else if(action.command_ == A_CHECK)
    {
        //nothing to do
    }
    else if(action.command_ == A_CALL)
    {
        player->placeMoney(callAmount);
    }
    else if(action.command_ == A_RAISE)
    {
        long amount = action.amount_;
        player->placeMoney(amount);
    }
}

//TODO check on CORRECTNESS
void CardDealer::determineWinnerAndSplitPot()
{
    std::vector<long> wager(table_->players_.size());
    std::vector<long> score(table_->players_.size());
    std::vector<bool> folded(table_->players_.size());

    // Evaluate the holdings of all players and safe scores (biggest score is winner)
    // If it is R_PREFLOP and we are here, than everyone except one folded. In this case this guy gets all the chips
    // without evaluation needed
    if (table_->round_ != R_PRE_FLOP) {
        for(size_t i = 0; i < table_->players_.size(); i++)
        {
            Player *pl = table_->players_[i];

            wager[i] = pl->wager_;
            folded[i] = pl->folded_;

            // Use the TPT evaluator
            int cards[7] = {TPTIndex(pl->holeCard1_), TPTIndex(pl->holeCard2_), TPTIndex(table_->boardCard1_), TPTIndex(table_->boardCard2_), TPTIndex(table_->boardCard3_)
                    , TPTIndex(table_->boardCard4_), TPTIndex(table_->boardCard5_)};
            score[i] = TPTEvaluator::lookupHandForNCards(&cards[0], 7);
        }

        //Tell player he won the hand
        std::vector<long> sortedScores = score;
        std::sort(sortedScores.begin(), sortedScores.end(), sScoreGreaterThan);
        long winnerValue = sortedScores[0];

        // When player went to showdown and won the hand the handsWon is incremented as it is at preflop when he wins the hand
        // When player on the otherhand went to showdown but looses the hand the handsLost is incremented as are his chipsTotalLost
        for (int i = 0; i < table_->players_.size(); i++) {
            if (score[i] == winnerValue && !table_->players_.at(i)->hasFolded()) {
                table_->players_[i]->handsWon_++;
            } else if (!table_->players_.at(i)->hasFolded()){
                table_->players_[i]->handsLost_++;
            }
        }

        std::vector<long> wins;

        dividePot(wins, wager, score, folded);

        // Give players money
        for(size_t i = 0; i < wins.size(); i++)
        {
            if(wins[i] == 0 && !table_->players_.at(i)->hasFolded()) {
                //player lost chips at showdown -> save for statistics
                table_->players_[i]->chipsLostTotal_ += table_->players_[i]->wager_;
            } else {
                table_->players_[i]->stack_ += wins[i];
                table_->players_[i]->chipsWonTotal_ += wins[i]; //TODO get rid of corner case where on bets more than any other player has but looses and wins his own chips back that no one covered
            }
        }


    } else {
        // Give the one player that has not folded all the wager chips
        //TODO test this for correctness
        long chipsWon = 0;
        int winningPlayerIndex = -1;
        for(int i = 0; i < table_->players_.size(); i++)
        {
            if(table_->players_.at(i)->hasFolded()) {
                chipsWon += table_->players_.at(i)->wager_;
                table_->players_[i]->handsLost_++;
                table_->players_[i]->chipsLostTotal_ += table_->players_[i]->wager_;

            } else {
                chipsWon += table_->players_.at(i)->wager_;
                winningPlayerIndex = i;
            }
        }
        table_->players_[winningPlayerIndex]->stack_ += chipsWon;
        table_->players_[winningPlayerIndex]->handsWon_++;
        table_->players_[winningPlayerIndex]->chipsWonTotal_ += chipsWon;
    }

    // Reset all wagers
    for(size_t i = 0; i < table_->players_.size(); i++)
    {
        table_->players_[i]->wager_ = 0;
    }

}


/* TODO check on CORRECTNESS -> DOES NOT WORK YET
 * This method is really slow and bad for performance
Calculates how much each player gets from the pot, depending on each players wager, combination score and whether he's folded.
Each std::vector has the size of the amount of players at the table and the index must match the index of players on the table.
wins: how much chips each player gets.
wager: how much each player has wagered this deal.
score: the value of the best 5-card combination of each player, this is an integer that must be greater for a better combination.
folded: whether or not this player has folded (folded players normally don't get any money, but may have bet some)
*/
void CardDealer::dividePot(std::vector<long>& wins, const std::vector<long>& wager, const std::vector<long>& score, const std::vector<bool>& folded)
{
    wins.resize(wager.size()); //how much each player wins
    for(auto &winnings : wins) winnings = 0;

    std::vector<SidePot> sidePots;

    long potsize = 0;

    // creating a helper struct of wagers by players
    std::vector<SPlayer> wagers;
    for(int i = 0; i < wager.size(); i++)
    {
        wagers.push_back(SPlayer(i, wager[i]));
        potsize += wager[i];
    }
    std::sort(wagers.begin(), wagers.end(), sPlayerSmallerThan);  // sorts wager from small to big

    size_t wagerindex = 0; // starting with the smallest wager
    while(potsize > 0)
    {
        if(wagers[wagerindex].value > 0)
        {
            // create a possible sidepot
            //sidePots.resize(sidePots.size() + 1);
            SidePot p = SidePot();
            long wagersize = wagers[wagerindex].value;
            // everyone did after the current wager, did at least wager as much as the current player or more
            // starting with the smallest wager for the smallest possible sidepot, and moving up from there
            // @wagers has to be sorted ascending
            p.chips = (wagers.size() - wagerindex) * wagersize;
            for(size_t i = wagerindex; i < wagers.size(); i++)
            {
                p.players.push_back(wagers[i].index);
                wagers[i].value -= wagersize;
                potsize -= wagersize;
            }
            sidePots.push_back(p);
        }
        wagerindex++;
    }

    for(size_t j = 0; j < sidePots.size(); j++)
    {
        SidePot& p = sidePots[j];
        std::vector<SPlayer> splayers;  // vector of players who are able to earn money

        for(size_t i = 0; i < p.players.size(); i++)
        {
            long combinationValue = score[p.players[i]];

            //folded players can't gain money anymore, give lowest possible value
            if(folded[p.players[i]]) combinationValue = -1;

            splayers.push_back(SPlayer(p.players[i], combinationValue));
        }

        std::sort(splayers.begin(), splayers.end(), sPlayerGreaterThan); // sorted descending (biggest to smallest)

        int num = 1; //num players with same combination value
        while(num < splayers.size() && splayers[num].value == splayers[0].value) num++;

        long potdiv = p.chips / num;
        long potextra = p.chips - potdiv * num; //extra chips due to pot not being exactly divisible by that number

        // split up the pot for winning player(s)
        for(size_t i = 0; i < num; i++) wins[splayers[i].index] += potdiv;
        wins[splayers[0].index] += potextra;

    } //for sidepots
}

// Burns a card from the deck (used before each dealing round)
void CardDealer::burnCard() {
    deck_.next();
}

//applies blinds and antes
void CardDealer::applyForcedBets(const Rules* rules)
{
    Player* sb = table_->players_[table_->getSmallBlindIndex()];
    Player* bb = table_->players_[table_->getBigBlindIndex()];

    long amount_sb = sb->placeMoney(rules->blindLevel_.smallBlind_);  // For log reason
    long amount_bb = bb->placeMoney(rules->blindLevel_.bigBlind_);

    if(rules->blindLevel_.ante_ > 0)
    {
        // Each player (including SB and BB) has to set the ante
        for(auto &player : table_->players_)
        {
            long amount = player->placeMoney(rules->blindLevel_.ante_);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
                            ///FUNCTIONS ON PLAYERS AND INFORMATIONS ON PLAYERS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//gets initial player for a certain betting round (returns negative value if something invalid is encountered)
int CardDealer::getInitialPlayer(Round round)
{
    /*
    With two players:
    Pre-flop: dealer starts
    Post flop: other player starts

    With three or more players:
    Pre-flop: player after big blind starts
    Post flop: player after dealer starts

    */
    if (getNumDecidingPlayers() < 2) {
        if (getNumActivePlayers() > 1) {    // only one player can still decide but there are  > 0 all-in players so showdown is required
            return E_NO_NEED_TO_SETTLE_BETS;
        }
        else return E_LESS_THAN_TWO_PLAYERS;    // stop hand because only one player left in hand
    }
    else if (table_->players_.size() == 2)
    {
        // When only two players remain, special 'head-to-head' or 'heads up' rules are enforced and the blinds are
        // posted differently. In this case, the person with the dealer button posts the small blind, while his/her
        // opponent places the big blind. The dealer acts first before the flop. After the flop, the dealer acts last
        // and continues to do so for the remainder of the hand.
        if (round == R_PRE_FLOP) return wrap(table_->dealer_, table_->players_.size());
        else return wrap(table_->dealer_ + 1, table_->players_.size());
    }
    else    // there are at least 2 players which can decide
    {
        int index;
        if(round == R_PRE_FLOP) index = table_->dealer_ + 3;
        else index = table_->dealer_ + 1;
        index = wrap(index, table_->players_.size());
        index = getCurrentOrNextActivePlayer(index);
        return index;
    }
}

//get amount of players that can still make decisions (stack > 0 and not folded or out)
int CardDealer::getNumDecidingPlayers()
{
    int result = 0;
    for (auto &player : table_->players_) {
        if (player->canDecide()) result++;
    }
    return result;
}

int CardDealer::getCurrentOrNextActivePlayer(int current)
{
    if (table_->players_[current]->canDecide()) return current;
    return (getNextActivePlayer(current));
}

//gets the next non-folded, non-out and non-all-in player. Returns -1 if none (if everyone other than the current player
// is all-in, out or has folded)
int CardDealer::getNextActivePlayer(int current)
{
    int result = current + 1;
    for(;;)
    {
        if(result >= (int)table_->players_.size()) result = 0;      // Manual wrap around
        if(result == current) return E_NO_ACTIVE_PLAYER_AVAILABLE;  // There is no possible active player available
        if(table_->players_[result]->canDecide()) return result;
        result++;
    }
}

int CardDealer::getNumActivePlayers() const
{
    int result = 0;
    for (auto &player : table_->players_) {
        if(!player->hasFolded()) result++;
    }
    return result;
}

//all players who have no money left are kicked out. Dealer is updated to be a non-out player.
void CardDealer::kickOutPlayers()
{
    std::vector<Player*>& playersIn = table_->players_;
    for(int i = 0; i < (int)playersIn.size(); i++)
    {
        bool leave = false;
        if(playersIn[i]->isOut())
        {
            leave = true;
        }

        if(leave)
        {
            table_->playersOut_.push_back(playersIn[i]);
            playersIn.erase(playersIn.begin() + i);
            if(table_->dealer_ > i) table_->dealer_--; // if i == table.dealer, it stays: that makes next player after the one who left the dealer
            if(table_->dealer_ >= (int)playersIn.size()) table_->dealer_ = 0; // if player at the end of array leaves. Dealer wraps around to 0.
            i--;
        }
    }
}