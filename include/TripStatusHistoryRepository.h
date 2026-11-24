#pragma once

#include <vector>
#include "Database.h"
#include "ITripHistoryRepository.h"

class TripStatusHistoryRepository : public ITripHistoryRepository
{
public:
    explicit TripStatusHistoryRepository(Storage& storage);

    int add(const TripStatusHistory& entry) override;
    std::vector<TripStatusHistory> findByTripId(int tripId) const override;

private:
    Storage& storage_;
};
