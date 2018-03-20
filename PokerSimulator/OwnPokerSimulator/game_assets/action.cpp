//
// Created by Fabian Moik on 11.12.17.
//

#include "action.h"

Action::Action(Command command, long amount, int betType)
        : command_(command)
        , amount_(amount)
        , betType_(betType)
{
}

Action::Action()
        : command_(A_FOLD)
        , amount_(0)
        , betType_(B_UNKNOWN_BET)
{
}

/*
    Is the action allowed by the game of no-limit Texas Hold'em?
    It is not allowed if:
    -you need to move more chips to the table than you have in your stack to perform this action (unless you go all-in)
    -it's a raise action but the amount of chips raised is smaller than the highest raise so far this deal (unless you go all-in)
        -when a player moves all-in for less then the full raise amount, the betting is not reopened again to the initial raiser
         after other players call. (utg bets 50, utg+1 is all-in for 77, utg+2 calls, back to utgv -> he can only call or fold))

        //TODO
        -min raise amount of utg+2 would be in this case 50 plus the extra amount of the all in (27 -> 77-50)
         so utg+2 would net to raise to at least 127
    -it's a check action while the call amount is higher than 0

    stack: stack the player currently has (excludes his wager)
    wager: chips the player has contributed to the pot this deal so far
    highestWager: the amount of wager of the player with highest wager (needed to know amount required to call)
    highestRaise: the highest raise amount so far during this deal (the amount raised above the call amount) (needed to exclude small raises)
*/
bool Action::isValidAction(long stack, long wager, long highestWager, long highestRaise, long extra)
{
    long callAmount = highestWager - wager;

    switch(command_)
    {
        case A_FOLD:
        {
            return true;
        }
        case A_CHECK:
        {
            return callAmount == 0;
        }
        case A_CALL:
        {
            if(callAmount == 0) return false; //you should use check in this case. Otherwise player statistics get messed up. So, disallowed!
            return stack > 0; //it's always valid for the rest, as long as your stack isn't empty. If call amount is bigger than your stack, you go all-in, it's still a valid call.
        }
        case A_RAISE:
        {
            long raiseAmount = amount_ - callAmount; // amount is the total amount you wager (call amount + raise amount)

            //if (raiseAmount >= table_->lastRaiseAmount_ + extra) // in order to count as a normal raise the amount has to be twice as big as the latest raise amount...
            //TODO check if correct with extra - should be correct
            return (amount_ <= stack && raiseAmount >= highestRaise + extra)
                   || isValidAllInAction(stack, wager, highestWager, highestRaise);
        }
    }

    return false;
}

/*
   Is this an all-in action, and, is it valid?
   It is not considered valid if it's a raise action and the amount of chips is larger than your stack. It
   must be exactly equal to be an all-in action.
*/
bool Action::isValidAllInAction(long stack, long wager, long highestWager, long highestRaise)
{
    (void)highestRaise;

    long callAmount = highestWager - wager;

    switch(command_)
    {
        case A_FOLD:
        {
            return false;
        }
        case A_CHECK:
        {
            return false;
        }
        case A_CALL:
        {
            return callAmount >= stack;
        }
        case A_RAISE:
        {
            // There are still two cases, either your stack is greater than than twice the lastRaiseAmount + call amount
            // then it is a normal raise and should be treated as such
            // Second case is, if you have less than the needed raiseAmount, then it should be treated as a call with extra
            // the extra amount needs to be added on top of further raises of other players
            // both cases are checked outside of this function
            return amount_ > callAmount && amount_ == stack; //must be exact. If higher, it's an invalid action.
        }
        default: return false;
    }
}

