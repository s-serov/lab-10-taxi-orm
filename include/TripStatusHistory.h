// Історія змін стану поїздки
#pragma once

#include <string>

struct TripStatusHistory
{
    int id = 0;
    int tripId = 0;
    std::string oldStatus;
    std::string newStatus;
    std::string changedAt;
};
