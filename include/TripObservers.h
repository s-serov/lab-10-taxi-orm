// Патерн Observer: конкретні підписники
#pragma once

#include <iosfwd>
#include <map>
#include <string>
#include <vector>
#include "TripEvents.h"

// Журнал водія: записує події поїздок по кожному водію
class DriverJournal : public ITripObserver
{
public:
    explicit DriverJournal(std::ostream* out = nullptr);

    void update(const TripEvent& event) override;
    std::vector<std::string> entriesFor(int driverId) const;

private:
    std::ostream* out_;
    std::map<int, std::vector<std::string>> entries_;
};

// Статистика поїздок: лічильники та підсумки
class TripStatistics : public ITripObserver
{
public:
    void update(const TripEvent& event) override;

    int createdCount() const { return createdCount_; }
    int completedCount() const { return completedCount_; }
    double totalRevenue() const { return totalRevenue_; }
    double totalDistanceKm() const { return totalDistanceKm_; }
    double averageFare() const;

private:
    int createdCount_ = 0;
    int completedCount_ = 0;
    double totalRevenue_ = 0.0;
    double totalDistanceKm_ = 0.0;
};
