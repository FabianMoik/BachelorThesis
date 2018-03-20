//
// Created by Corny on 05.11.2017.
//

#include "table.h"
#include "cardDealer.h"
#include "../util.h"

Table::Table()
        : isRunning_(false)
        , dealer_(0)
        , current_(1)
        , round_(R_PRE_FLOP)
        , turn_(0)
{
}

Table::~Table()
{
    delete cardDealer_;
}

void Table::setID(int id) {
    id_ = id;
}

int Table::getID() {
    return id_;
}


void Table::seatPlayer(Player* player) {
    players_.push_back(player);
}

void Table::assignCardDealer(CardDealer* dealer) {
    cardDealer_ = dealer;
}

// TABLE INFORMATION FUNCTIONS


int Table::wrap(int index) const
{
    return ::wrap(index, players_.size());
}

int Table::getSmallBlindIndex() const
{
    if(players_.size() == 2)
    {
        return dealer_;
    }
    else
    {
        return wrap(dealer_ + 1);
    }
}

int Table::getBigBlindIndex() const
{
    if(players_.size() == 2)
    {
        return wrap(dealer_ + 1);
    }
    else
    {
        return wrap(dealer_ + 2);
    }
}

int Table::getHighestWager() const
{
    int result = 0;
    for (auto &player : players_)
    {
        if(player->wager_ > result) result = player->wager_;

    }
    return result;
}

int Table::getCallAmount() const
{
    int result = getHighestWager() - players_[current_]->wager_;

    if(players_[current_]->stack_ < result) result = players_[current_]->stack_;

    return result;
}

int Table::getPot() const
{
    int result = 0;
    for(auto &player: players_)
    {
        result += player->wager_;
    }
    return result;
}

int Table::getPositionOfPlayer(int index) const
{
    return wrap(index - dealer_);
}
