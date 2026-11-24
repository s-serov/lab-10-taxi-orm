// Патерн Observer: видавець подій
#include "TripEvents.h"

#include <algorithm>
#include <exception>
#include <iostream>

void TripEventPublisher::subscribe(const std::shared_ptr<ITripObserver>& observer)
{
    if (!observer)
    {
        return;
    }
    const bool alreadySubscribed = std::any_of(
        observers_.begin(), observers_.end(),
        [&](const std::weak_ptr<ITripObserver>& item) { return item.lock() == observer; });
    if (!alreadySubscribed)
    {
        observers_.push_back(observer);
    }
}

void TripEventPublisher::unsubscribe(const std::shared_ptr<ITripObserver>& observer)
{
    std::erase_if(observers_, [&](const std::weak_ptr<ITripObserver>& item)
    {
        auto ptr = item.lock();
        return !ptr || ptr == observer;
    });
}

void TripEventPublisher::publish(const TripEvent& event)
{
    std::erase_if(observers_, [](const std::weak_ptr<ITripObserver>& item)
    {
        return item.expired();
    });

    // Копія списку: підписник може скасувати підписку під час обробки події
    std::vector<std::shared_ptr<ITripObserver>> active;
    for (const auto& item : observers_)
    {
        if (auto observer = item.lock())
        {
            active.push_back(std::move(observer));
        }
    }

    for (const auto& observer : active)
    {
        try
        {
            observer->update(event);
        }
        catch (const std::exception& ex)
        {
            std::cerr << "Observer error: " << ex.what() << '\n';
        }
    }
}

std::size_t TripEventPublisher::subscriberCount() const
{
    return static_cast<std::size_t>(std::count_if(
        observers_.begin(), observers_.end(),
        [](const std::weak_ptr<ITripObserver>& item) { return !item.expired(); }));
}
