#include "TripStatusHistoryRepository.h"

TripStatusHistoryRepository::TripStatusHistoryRepository(Storage& storage)
    : storage_(storage)
{
}

int TripStatusHistoryRepository::add(const TripStatusHistory& entry)
{
    return storage_.insert(entry);
}

std::vector<TripStatusHistory> TripStatusHistoryRepository::findByTripId(int tripId) const
{
    using namespace sqlite_orm;
    return storage_.get_all<TripStatusHistory>(
        where(c(&TripStatusHistory::tripId) == tripId),
        order_by(&TripStatusHistory::id).asc()
    );
}
