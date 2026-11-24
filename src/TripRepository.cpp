// Завдання 4
#include "TripRepository.h"

TripRepository::TripRepository(Storage& storage)
    : storage_(storage)
{
}

int TripRepository::add(const Trip& trip)
{
    return storage_.insert(trip);
}

std::optional<Trip> TripRepository::findById(int id) const
{
    return storage_.get_optional<Trip>(id);
}

std::vector<Trip> TripRepository::findAll() const
{
    return storage_.get_all<Trip>();
}

std::vector<Trip> TripRepository::findByDriverId(int driverId) const
{
    using namespace sqlite_orm;
    return storage_.get_all<Trip>(
        where(c(&Trip::driverId) == driverId),
        order_by(&Trip::tripDate).asc()
    );
}

bool TripRepository::update(const Trip& trip)
{
    if (!storage_.get_optional<Trip>(trip.id))
    {
        return false;
    }
    storage_.update(trip);
    return true;
}
