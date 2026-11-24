// Патерн Observer: події поїздок
#pragma once

#include <cstddef>
#include <memory>
#include <vector>

enum class TripEventType
{
    Created,
    Completed
};

struct TripEvent
{
    TripEventType type;
    int tripId;
    int driverId;
    double distanceKm;
    double fare;
};

class ITripObserver
{
public:
    virtual ~ITripObserver() = default;

    virtual void update(const TripEvent& event) = 0;
};

// Видавець: не володіє підписниками, зберігає слабкі посилання
class TripEventPublisher
{
public:
    void subscribe(const std::shared_ptr<ITripObserver>& observer);
    void unsubscribe(const std::shared_ptr<ITripObserver>& observer);
    void publish(const TripEvent& event);
    std::size_t subscriberCount() const;

private:
    std::vector<std::weak_ptr<ITripObserver>> observers_;
};
