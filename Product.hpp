#ifndef PRODUCT_HPP
#define PRODUCT_HPP

#include <stack>
#include "TS.hpp"
#include "NBA.hpp"

std::shared_ptr<TS> ProductTSWithNBA(std::shared_ptr<TS> ts, std::shared_ptr<NBA> nba);

#endif 