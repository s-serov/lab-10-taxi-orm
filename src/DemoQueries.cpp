// Завдання 5
#include "Demos.h"

#include <memory>

namespace
{
    double valueOr(const std::unique_ptr<double>& value, double fallback)
    {
        return value ? *value : fallback;
    }
}

void runQueryDemo(Storage& storage)
{
    using namespace sqlite_orm;

    std::cout << "\n=== ORM QUERY DEMONSTRATION ===\n";

    // 1. Фільтрація
    std::cout << "\n1.1. Active drivers:\n";
    auto activeDrivers = storage.get_all<Driver>(
        where(c(&Driver::status) == "active")
    );
    for (const auto& driver : activeDrivers)
    {
        std::cout << driver.id << " | " << driver.name << " | "
                  << driver.status << '\n';
    }

    std::cout << "\n1.2. Long expensive trips (distance > 10 km, fare > 250):\n";
    auto longTrips = storage.get_all<Trip>(
        where(and_(c(&Trip::distanceKm) > 10.0, c(&Trip::fare) > 250.0))
    );
    for (const auto& trip : longTrips)
    {
        std::cout << trip.id << " | " << trip.tripDate << " | "
                  << trip.distanceKm << " km | " << trip.fare << '\n';
    }

    // 2. Сортування та обмеження
    std::cout << "\n2. Top 3 most expensive trips:\n";
    auto topTrips = storage.get_all<Trip>(
        order_by(&Trip::fare).desc(),
        limit(3)
    );
    for (const auto& trip : topTrips)
    {
        std::cout << trip.id << " | " << trip.tripDate << " | "
                  << trip.pickupAddress << " -> " << trip.dropoffAddress
                  << " | " << trip.fare << '\n';
    }

    // 3. Агрегація
    std::cout << "\n3. Aggregates:\n";
    std::cout << "Trip count: " << storage.count<Trip>() << '\n';
    std::cout << "Total fare: "
              << valueOr(storage.sum(&Trip::fare), 0.0) << '\n';
    std::cout << "Average fare: " << storage.avg(&Trip::fare) << '\n';

    std::cout << "\n3.1. Trips and revenue by driver (GROUP BY):\n";
    auto byDriver = storage.select(
        columns(&Trip::driverId, count(&Trip::id), sum(&Trip::fare)),
        group_by(&Trip::driverId)
    );
    for (const auto& [driverId, trips, total] : byDriver)
    {
        std::cout << "Driver ID: " << driverId
                  << " | Trips: " << trips
                  << " | Revenue: " << valueOr(total, 0.0) << '\n';
    }

    // 4. INNER JOIN
    std::cout << "\n4.1. Trip history with driver data:\n";
    auto history = storage.select(
        columns(
            &Driver::name,
            &Driver::licensePlate,
            &Trip::tripDate,
            &Trip::pickupAddress,
            &Trip::dropoffAddress,
            &Trip::fare
        ),
        inner_join<Trip>(on(c(&Driver::id) == &Trip::driverId)),
        order_by(&Trip::id)
    );
    for (const auto& [name, plate, date, from, to, fare] : history)
    {
        std::cout << name << " | " << plate << " | " << date << " | "
                  << from << " -> " << to << " | " << fare << '\n';
    }

    std::cout << "\n4.2. Trips of active drivers with fare over 300:\n";
    auto activeExpensive = storage.select(
        columns(&Driver::name, &Trip::tripDate, &Trip::fare),
        inner_join<Trip>(on(c(&Driver::id) == &Trip::driverId)),
        where(and_(c(&Driver::status) == "active", c(&Trip::fare) > 300.0)),
        order_by(&Trip::fare).desc()
    );
    for (const auto& [name, date, fare] : activeExpensive)
    {
        std::cout << name << " | " << date << " | " << fare << '\n';
    }
}
