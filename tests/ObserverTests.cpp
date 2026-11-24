#include <gtest/gtest.h>
#include "Fakes.h"
#include "TripEvents.h"
#include "TripObservers.h"

namespace
{
    const TripEvent kCreated{TripEventType::Created, 1, 7, 10.0, 170.0};
    const TripEvent kCompleted{TripEventType::Completed, 1, 7, 10.0, 180.0};

    // Підписник, який скасовує власну підписку під час обробки події
    class SelfUnsubscribingObserver : public ITripObserver
    {
    public:
        explicit SelfUnsubscribingObserver(TripEventPublisher& publisher) : publisher_(publisher) {}
        void bind(const std::shared_ptr<ITripObserver>& self) { self_ = self; }
        void update(const TripEvent&) override
        {
            ++calls;
            publisher_.unsubscribe(self_.lock());
        }
        int calls = 0;

    private:
        TripEventPublisher& publisher_;
        std::weak_ptr<ITripObserver> self_;
    };
}

TEST(Observer, AllSubscribersReceiveEvent)
{
    TripEventPublisher publisher;
    auto first = std::make_shared<RecordingObserver>();
    auto second = std::make_shared<RecordingObserver>();
    publisher.subscribe(first);
    publisher.subscribe(second);

    publisher.publish(kCreated);

    EXPECT_EQ(first->events.size(), 1u);
    EXPECT_EQ(second->events.size(), 1u);
    EXPECT_EQ(first->events[0].tripId, 1);
}

TEST(Observer, UnsubscribedObserverReceivesNothing)
{
    TripEventPublisher publisher;
    auto first = std::make_shared<RecordingObserver>();
    auto second = std::make_shared<RecordingObserver>();
    publisher.subscribe(first);
    publisher.subscribe(second);

    publisher.publish(kCreated);
    publisher.unsubscribe(second);
    publisher.publish(kCompleted);

    EXPECT_EQ(first->events.size(), 2u);
    EXPECT_EQ(second->events.size(), 1u);
}

TEST(Observer, DuplicateSubscriptionIsIgnored)
{
    TripEventPublisher publisher;
    auto observer = std::make_shared<RecordingObserver>();
    publisher.subscribe(observer);
    publisher.subscribe(observer);

    publisher.publish(kCreated);

    EXPECT_EQ(observer->events.size(), 1u);
    EXPECT_EQ(publisher.subscriberCount(), 1u);
}

TEST(Observer, DestroyedSubscriberIsSkipped)
{
    TripEventPublisher publisher;
    auto alive = std::make_shared<RecordingObserver>();
    publisher.subscribe(alive);
    {
        auto temporary = std::make_shared<RecordingObserver>();
        publisher.subscribe(temporary);
        EXPECT_EQ(publisher.subscriberCount(), 2u);
    }

    EXPECT_EQ(publisher.subscriberCount(), 1u);
    publisher.publish(kCreated);
    EXPECT_EQ(alive->events.size(), 1u);
}

TEST(Observer, FailingSubscriberDoesNotBlockOthers)
{
    TripEventPublisher publisher;
    auto failing = std::make_shared<ThrowingObserver>();
    auto healthy = std::make_shared<RecordingObserver>();
    publisher.subscribe(failing);
    publisher.subscribe(healthy);

    EXPECT_NO_THROW(publisher.publish(kCreated));
    EXPECT_EQ(healthy->events.size(), 1u);
}

TEST(Observer, SubscriberMayUnsubscribeDuringNotification)
{
    TripEventPublisher publisher;
    auto selfRemoving = std::make_shared<SelfUnsubscribingObserver>(publisher);
    selfRemoving->bind(selfRemoving);
    auto other = std::make_shared<RecordingObserver>();
    publisher.subscribe(selfRemoving);
    publisher.subscribe(other);

    publisher.publish(kCreated);
    publisher.publish(kCompleted);

    EXPECT_EQ(selfRemoving->calls, 1);
    EXPECT_EQ(other->events.size(), 2u);
}

TEST(Observer, StatisticsCountsEvents)
{
    TripStatistics statistics;
    statistics.update(kCreated);
    statistics.update(kCreated);
    statistics.update(kCompleted);

    EXPECT_EQ(statistics.createdCount(), 2);
    EXPECT_EQ(statistics.completedCount(), 1);
    EXPECT_DOUBLE_EQ(statistics.totalRevenue(), 180.0);
    EXPECT_DOUBLE_EQ(statistics.totalDistanceKm(), 10.0);
}

TEST(Observer, StatisticsAverageFare)
{
    TripStatistics statistics;
    EXPECT_DOUBLE_EQ(statistics.averageFare(), 0.0);
    statistics.update({TripEventType::Completed, 1, 7, 5.0, 100.0});
    statistics.update({TripEventType::Completed, 2, 7, 5.0, 200.0});
    EXPECT_DOUBLE_EQ(statistics.averageFare(), 150.0);
}

TEST(Observer, JournalRecordsEntriesPerDriver)
{
    DriverJournal journal;
    journal.update(kCreated);
    journal.update(kCompleted);
    journal.update({TripEventType::Created, 2, 9, 3.0, 90.0});

    EXPECT_EQ(journal.entriesFor(7).size(), 2u);
    EXPECT_EQ(journal.entriesFor(9).size(), 1u);
    EXPECT_TRUE(journal.entriesFor(100).empty());
}
