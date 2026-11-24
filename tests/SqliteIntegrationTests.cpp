#include <gtest/gtest.h>

#include "Database.h"
#include "SQLiteDriverRepository.h"
#include "SQLiteTransactionRunner.h"
#include "TripRepository.h"
#include "TripService.h"
#include "TripStatusHistoryRepository.h"

namespace
{
    // Реальна база SQLite у пам'яті: перевіряє узгодження патернів з ORM та Repository
    struct SqliteEnv
    {
        Storage storage;
        SQLiteDriverRepository drivers;
        TripRepository trips;
        TripStatusHistoryRepository histories;
        SQLiteTransactionRunner transactions;
        TripEventPublisher publisher;
        TripService service;
        int driverId = 0;

        SqliteEnv()
            : storage(createStorage(":memory:")),
              drivers(storage),
              trips(storage),
              histories(storage),
              transactions(storage),
              service(drivers, trips, histories, transactions, publisher,
                      std::make_unique<DistanceFareStrategy>(), 50.0)
        {
            configureStorage(storage);
            driverId = drivers.add({0, "Ivan Petrenko", "Toyota Camry", "AA1234KK", "active"});
        }

        int createTrip()
        {
            return service.createTrip({driverId, "2026-11-20", "Podil", "Obolon", 10.0, 25});
        }
    };

    class FailingHistoryRepository : public ITripHistoryRepository
    {
    public:
        int add(const TripStatusHistory&) override
        {
            throw std::runtime_error("History write failed");
        }
        std::vector<TripStatusHistory> findByTripId(int) const override { return {}; }
    };
}

TEST(SqliteIntegration, LifecycleIsPersistedWithHistory)
{
    SqliteEnv env;
    const int id = env.createTrip();
    env.service.startTrip(id);
    env.service.completeTrip(id, 30);

    auto trip = env.trips.findById(id);
    ASSERT_TRUE(trip.has_value());
    EXPECT_EQ(trip->status, "Completed");
    EXPECT_EQ(trip->durationMin, 30);

    auto history = env.histories.findByTripId(id);
    ASSERT_EQ(history.size(), 3u);
    EXPECT_EQ(history[0].newStatus, "Created");
    EXPECT_EQ(history[1].newStatus, "InProgress");
    EXPECT_EQ(history[2].newStatus, "Completed");
}

TEST(SqliteIntegration, InvalidTransitionLeavesDatabaseUntouched)
{
    SqliteEnv env;
    const int id = env.createTrip();
    env.service.cancelTrip(id);

    EXPECT_THROW(env.service.startTrip(id), InvalidTransitionError);

    EXPECT_EQ(env.trips.findById(id)->status, "Cancelled");
    EXPECT_EQ(env.histories.findByTripId(id).size(), 2u);
}

TEST(SqliteIntegration, StatusAndHistoryAreChangedAtomically)
{
    SqliteEnv env;
    const int id = env.createTrip();

    FailingHistoryRepository failingHistory;
    TripService brokenService(env.drivers, env.trips, failingHistory, env.transactions,
                              env.publisher, std::make_unique<DistanceFareStrategy>());

    EXPECT_THROW(brokenService.startTrip(id), std::runtime_error);

    EXPECT_EQ(env.trips.findById(id)->status, "Created");
    EXPECT_EQ(env.histories.findByTripId(id).size(), 1u);
}

TEST(SqliteIntegration, HistoryRequiresExistingTrip)
{
    SqliteEnv env;
    EXPECT_THROW(env.histories.add({0, 999, "", "Created", "2026-11-20 10:00:00"}),
                 std::exception);
}
