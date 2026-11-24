// Завдання 4
#pragma once

#include <optional>
#include <vector>
#include "Database.h"
#include "ITripRepository.h"

class TripRepository : public ITripRepository
{
public:
    explicit TripRepository(Storage& storage);

    int add(const Trip& trip) override;
    std::optional<Trip> findById(int id) const override;
    std::vector<Trip> findAll() const override;
    std::vector<Trip> findByDriverId(int driverId) const override;
    bool update(const Trip& trip) override;

private:
    Storage& storage_;
};
