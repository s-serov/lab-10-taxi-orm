// Патерн Facade: єдина точка роботи з поїздками
#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "FareStrategy.h"
#include "IDriverRepository.h"
#include "ITransactionRunner.h"
#include "ITripHistoryRepository.h"
#include "ITripRepository.h"
#include "TripEvents.h"
#include "TripState.h"

struct NewTripRequest
{
    int driverId = 0;
    std::string tripDate;
    std::string pickupAddress;
    std::string dropoffAddress;
    double distanceKm = 0.0;
    int estimatedDurationMin = 0;
};

class TripService
{
public:
    TripService(IDriverRepository& drivers,
                ITripRepository& trips,
                ITripHistoryRepository& histories,
                ITransactionRunner& transactions,
                TripEventPublisher& publisher,
                std::unique_ptr<IFareStrategy> fareStrategy,
                double baseRate = 50.0);

    int createTrip(const NewTripRequest& request);
    void startTrip(int tripId);
    void completeTrip(int tripId, int actualDurationMin);
    void cancelTrip(int tripId);

    void setFareStrategy(std::unique_ptr<IFareStrategy> strategy);
    std::string fareStrategyName() const;

    std::optional<Trip> findTrip(int tripId) const;
    std::vector<TripStatusHistory> historyOf(int tripId) const;

private:
    Trip transition(int tripId, const std::function<void(TripContext&)>& operation);

    IDriverRepository& drivers_;
    ITripRepository& trips_;
    ITripHistoryRepository& histories_;
    ITransactionRunner& transactions_;
    TripEventPublisher& publisher_;
    FareCalculator calculator_;
    double baseRate_;
};
