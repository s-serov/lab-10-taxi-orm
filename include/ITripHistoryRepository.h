#pragma once

#include <vector>
#include "TripStatusHistory.h"

class ITripHistoryRepository
{
public:
    virtual ~ITripHistoryRepository() = default;

    virtual int add(const TripStatusHistory& entry) = 0;
    virtual std::vector<TripStatusHistory> findByTripId(int tripId) const = 0;
};
