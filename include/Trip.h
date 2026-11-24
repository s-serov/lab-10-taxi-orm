// Завдання 3: сутність поїздки (розширено полями тривалості та статусу)
#pragma once

#include <string>

namespace TripStatus
{
    inline constexpr const char* Created = "Created";
    inline constexpr const char* InProgress = "InProgress";
    inline constexpr const char* Completed = "Completed";
    inline constexpr const char* Cancelled = "Cancelled";
}

struct Trip
{
    int id = 0;
    int driverId = 0;
    std::string tripDate;
    std::string pickupAddress;
    std::string dropoffAddress;
    double distanceKm = 0.0;
    double fare = 0.0;
    int durationMin = 0;
    std::string status = TripStatus::Created;
};
