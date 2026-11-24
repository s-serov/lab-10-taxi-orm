#include <gtest/gtest.h>
#include "Fakes.h"
#include "TripObservers.h"
#include "TripService.h"

namespace
{
    // Сервіс із підставними репозиторіями, без реальної бази даних
    struct Env
    {
        FakeDatabase db;
        FakeDriverRepository drivers{db};
        FakeTripRepository trips{db};
        FakeHistoryRepository histories{db};
        FakeTransactionRunner transactions{db};
        TripEventPublisher publisher;
        TripService service{drivers, trips, histories, transactions, publisher,
                            std::make_unique<DistanceFareStrategy>(), 50.0};
        std::shared_ptr<RecordingObserver> recorder = std::make_shared<RecordingObserver>();
        int driverId = 0;

        Env()
        {
            driverId = drivers.add({0, "Ivan Petrenko", "Toyota Camry", "AA1234KK", "active"});
            publisher.subscribe(recorder);
        }

        int createTrip()
        {
            return service.createTrip({driverId, "2026-11-20", "Podil", "Obolon", 10.0, 25});
        }
    };
}

TEST(TripService, CreateTripStoresTripAndInitialHistory)
{
    Env env;
    const int id = env.createTrip();

    auto trip = env.service.findTrip(id);
    ASSERT_TRUE(trip.has_value());
    EXPECT_EQ(trip->status, "Created");
    EXPECT_EQ(trip->driverId, env.driverId);

    auto history = env.service.historyOf(id);
    ASSERT_EQ(history.size(), 1u);
    EXPECT_EQ(history[0].oldStatus, "");
    EXPECT_EQ(history[0].newStatus, "Created");
    EXPECT_FALSE(history[0].changedAt.empty());
}

TEST(TripService, CreateTripUsesCurrentFareStrategy)
{
    Env env;
    const int first = env.createTrip();
    env.service.setFareStrategy(std::make_unique<TimeFareStrategy>());
    const int second = env.createTrip();
    env.service.setFareStrategy(std::make_unique<CombinedFareStrategy>());
    const int third = env.createTrip();

    EXPECT_DOUBLE_EQ(env.service.findTrip(first)->fare, 170.0);
    EXPECT_DOUBLE_EQ(env.service.findTrip(second)->fare, 150.0);
    EXPECT_DOUBLE_EQ(env.service.findTrip(third)->fare, 180.0);
    EXPECT_EQ(env.service.fareStrategyName(), "Combined");
}

TEST(TripService, CreateTripPublishesCreatedEvent)
{
    Env env;
    const int id = env.createTrip();

    ASSERT_EQ(env.recorder->events.size(), 1u);
    EXPECT_TRUE(env.recorder->events[0].type == TripEventType::Created);
    EXPECT_EQ(env.recorder->events[0].tripId, id);
    EXPECT_EQ(env.recorder->events[0].driverId, env.driverId);
}

TEST(TripService, UnknownDriverIsRejected)
{
    Env env;
    EXPECT_THROW(env.service.createTrip({999, "2026-11-20", "A", "B", 5.0, 10}),
                 std::invalid_argument);
    EXPECT_TRUE(env.db.trips.empty());
    EXPECT_TRUE(env.recorder->events.empty());
}

TEST(TripService, FullLifecycleIsRecordedInHistory)
{
    Env env;
    const int id = env.createTrip();
    env.service.startTrip(id);
    env.service.completeTrip(id, 30);

    EXPECT_EQ(env.service.findTrip(id)->status, "Completed");
    auto history = env.service.historyOf(id);
    ASSERT_EQ(history.size(), 3u);
    EXPECT_EQ(history[1].oldStatus, "Created");
    EXPECT_EQ(history[1].newStatus, "InProgress");
    EXPECT_EQ(history[2].oldStatus, "InProgress");
    EXPECT_EQ(history[2].newStatus, "Completed");
}

TEST(TripService, CompleteRecalculatesFareWithActualDuration)
{
    Env env;
    env.service.setFareStrategy(std::make_unique<CombinedFareStrategy>());
    const int id = env.createTrip();
    EXPECT_DOUBLE_EQ(env.service.findTrip(id)->fare, 180.0);

    env.service.startTrip(id);
    env.service.completeTrip(id, 40);

    auto trip = env.service.findTrip(id);
    EXPECT_EQ(trip->durationMin, 40);
    EXPECT_DOUBLE_EQ(trip->fare, 210.0);
}

TEST(TripService, CompletePublishesCompletedEvent)
{
    Env env;
    const int id = env.createTrip();
    env.service.startTrip(id);
    env.service.completeTrip(id, 25);

    ASSERT_EQ(env.recorder->events.size(), 2u);
    EXPECT_TRUE(env.recorder->events[1].type == TripEventType::Completed);
    EXPECT_EQ(env.recorder->events[1].tripId, id);
    EXPECT_DOUBLE_EQ(env.recorder->events[1].fare, 170.0);
}

TEST(TripService, InvalidTransitionChangesNothing)
{
    Env env;
    const int id = env.createTrip();

    EXPECT_THROW(env.service.completeTrip(id, 20), InvalidTransitionError);

    EXPECT_EQ(env.service.findTrip(id)->status, "Created");
    EXPECT_EQ(env.service.historyOf(id).size(), 1u);
    EXPECT_EQ(env.recorder->events.size(), 1u);
}

TEST(TripService, CancelFromCreatedAndFinalStates)
{
    Env env;
    const int id = env.createTrip();
    env.service.cancelTrip(id);

    EXPECT_EQ(env.service.findTrip(id)->status, "Cancelled");
    EXPECT_THROW(env.service.startTrip(id), InvalidTransitionError);
    EXPECT_THROW(env.service.cancelTrip(id), InvalidTransitionError);
    EXPECT_EQ(env.service.historyOf(id).size(), 2u);
}

TEST(TripService, CompletedTripCannotBeCancelled)
{
    Env env;
    const int id = env.createTrip();
    env.service.startTrip(id);
    env.service.completeTrip(id, 25);

    EXPECT_THROW(env.service.cancelTrip(id), InvalidTransitionError);
    EXPECT_EQ(env.service.findTrip(id)->status, "Completed");
}

TEST(TripService, FailedHistoryWriteRollsBackStatusChange)
{
    Env env;
    const int id = env.createTrip();

    env.histories.failOnAdd = true;
    EXPECT_THROW(env.service.startTrip(id), std::runtime_error);

    EXPECT_EQ(env.service.findTrip(id)->status, "Created");
    EXPECT_EQ(env.db.history.size(), 1u);
}

TEST(TripService, FailedPersistencePublishesNoEvent)
{
    Env env;
    const int id = env.createTrip();
    env.service.startTrip(id);
    env.recorder->events.clear();

    env.histories.failOnAdd = true;
    EXPECT_THROW(env.service.completeTrip(id, 25), std::runtime_error);

    EXPECT_TRUE(env.recorder->events.empty());
    EXPECT_EQ(env.service.findTrip(id)->status, "InProgress");
}

TEST(TripService, UnknownTripIsRejected)
{
    Env env;
    EXPECT_THROW(env.service.startTrip(123), std::invalid_argument);
    EXPECT_THROW(env.service.completeTrip(123, 10), std::invalid_argument);
    EXPECT_THROW(env.service.cancelTrip(123), std::invalid_argument);
}

TEST(TripService, NegativeDurationIsRejected)
{
    Env env;
    const int id = env.createTrip();
    env.service.startTrip(id);
    EXPECT_THROW(env.service.completeTrip(id, -5), std::invalid_argument);
    EXPECT_EQ(env.service.findTrip(id)->status, "InProgress");
}

TEST(TripService, ObserversReceiveEventsFromService)
{
    Env env;
    auto statistics = std::make_shared<TripStatistics>();
    auto journal = std::make_shared<DriverJournal>();
    env.publisher.subscribe(statistics);
    env.publisher.subscribe(journal);

    const int id = env.createTrip();
    env.service.startTrip(id);
    env.service.completeTrip(id, 25);

    EXPECT_EQ(statistics->createdCount(), 1);
    EXPECT_EQ(statistics->completedCount(), 1);
    EXPECT_DOUBLE_EQ(statistics->totalRevenue(), 170.0);
    EXPECT_EQ(journal->entriesFor(env.driverId).size(), 2u);

    env.publisher.unsubscribe(statistics);
    env.createTrip();
    EXPECT_EQ(statistics->createdCount(), 1);
    EXPECT_EQ(journal->entriesFor(env.driverId).size(), 3u);
}
