// Патерн Facade: єдина точка роботи з поїздками
#include "TripService.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace
{
    std::string currentTimestamp()
    {
        const std::time_t now = std::time(nullptr);
        std::tm local{};
#ifdef _WIN32
        localtime_s(&local, &now);
#else
        localtime_r(&now, &local);
#endif
        std::ostringstream out;
        out << std::put_time(&local, "%Y-%m-%d %H:%M:%S");
        return out.str();
    }
}

TripService::TripService(IDriverRepository& drivers,
                         ITripRepository& trips,
                         ITripHistoryRepository& histories,
                         ITransactionRunner& transactions,
                         TripEventPublisher& publisher,
                         std::unique_ptr<IFareStrategy> fareStrategy,
                         double baseRate)
    : drivers_(drivers),
      trips_(trips),
      histories_(histories),
      transactions_(transactions),
      publisher_(publisher),
      calculator_(std::move(fareStrategy)),
      baseRate_(baseRate)
{
}

int TripService::createTrip(const NewTripRequest& request)
{
    if (!drivers_.findById(request.driverId))
    {
        throw std::invalid_argument("Driver not found");
    }

    Trip trip;
    trip.driverId = request.driverId;
    trip.tripDate = request.tripDate;
    trip.pickupAddress = request.pickupAddress;
    trip.dropoffAddress = request.dropoffAddress;
    trip.distanceKm = request.distanceKm;
    trip.durationMin = request.estimatedDurationMin;
    trip.fare = calculator_.calculate({request.distanceKm, request.estimatedDurationMin, baseRate_});
    trip.status = TripStatus::Created;

    int tripId = 0;
    transactions_.run([&]()
    {
        tripId = trips_.add(trip);
        histories_.add({0, tripId, "", TripStatus::Created, currentTimestamp()});
    });

    publisher_.publish({TripEventType::Created, tripId, trip.driverId, trip.distanceKm, trip.fare});
    return tripId;
}

void TripService::startTrip(int tripId)
{
    transition(tripId, [](TripContext& context) { context.start(); });
}

void TripService::completeTrip(int tripId, int actualDurationMin)
{
    if (actualDurationMin < 0)
    {
        throw std::invalid_argument("Duration must not be negative");
    }

    const Trip completed = transition(tripId, [&](TripContext& context)
    {
        context.complete();
        context.trip().durationMin = actualDurationMin;
        context.trip().fare = calculator_.calculate(
            {context.trip().distanceKm, actualDurationMin, baseRate_});
    });

    // Подія надсилається лише після успішного завершення транзакції
    publisher_.publish({TripEventType::Completed, completed.id, completed.driverId,
                        completed.distanceKm, completed.fare});
}

void TripService::cancelTrip(int tripId)
{
    transition(tripId, [](TripContext& context) { context.cancel(); });
}

void TripService::setFareStrategy(std::unique_ptr<IFareStrategy> strategy)
{
    calculator_.setStrategy(std::move(strategy));
}

std::string TripService::fareStrategyName() const
{
    return calculator_.strategyName();
}

std::optional<Trip> TripService::findTrip(int tripId) const
{
    return trips_.findById(tripId);
}

std::vector<TripStatusHistory> TripService::historyOf(int tripId) const
{
    return histories_.findByTripId(tripId);
}

Trip TripService::transition(int tripId, const std::function<void(TripContext&)>& operation)
{
    auto stored = trips_.findById(tripId);
    if (!stored)
    {
        throw std::invalid_argument("Trip not found");
    }

    TripContext context(*stored);
    const std::string oldStatus = context.stateName();

    // Недопустимий перехід викликає виняток до будь-яких змін у БД
    operation(context);

    transactions_.run([&]()
    {
        if (!trips_.update(context.trip()))
        {
            throw std::runtime_error("Trip update failed");
        }
        histories_.add({0, tripId, oldStatus, context.stateName(), currentTimestamp()});
    });

    return context.trip();
}
