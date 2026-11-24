// Патерн Observer: конкретні підписники
#include "TripObservers.h"

#include <iomanip>
#include <ostream>
#include <sstream>

DriverJournal::DriverJournal(std::ostream* out)
    : out_(out)
{
}

void DriverJournal::update(const TripEvent& event)
{
    std::ostringstream line;
    line << std::fixed << std::setprecision(2);
    line << "Trip #" << event.tripId;
    if (event.type == TripEventType::Created)
    {
        line << " created, estimated fare " << event.fare;
    }
    else
    {
        line << " completed, " << event.distanceKm << " km, fare " << event.fare;
    }

    entries_[event.driverId].push_back(line.str());
    if (out_)
    {
        *out_ << "[Driver journal] Driver " << event.driverId << ": " << line.str() << '\n';
    }
}

std::vector<std::string> DriverJournal::entriesFor(int driverId) const
{
    auto it = entries_.find(driverId);
    return it == entries_.end() ? std::vector<std::string>{} : it->second;
}

void TripStatistics::update(const TripEvent& event)
{
    if (event.type == TripEventType::Created)
    {
        ++createdCount_;
        return;
    }
    ++completedCount_;
    totalRevenue_ += event.fare;
    totalDistanceKm_ += event.distanceKm;
}

double TripStatistics::averageFare() const
{
    return completedCount_ == 0 ? 0.0 : totalRevenue_ / completedCount_;
}
