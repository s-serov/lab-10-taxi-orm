#pragma once

#include <optional>
#include <vector>
#include "Trip.h"

class ITripRepository
{
public:
    virtual ~ITripRepository() = default;

    virtual int add(const Trip& trip) = 0;
    virtual std::optional<Trip> findById(int id) const = 0;
    virtual std::vector<Trip> findAll() const = 0;
    virtual std::vector<Trip> findByDriverId(int driverId) const = 0;
    virtual bool update(const Trip& trip) = 0;
};
