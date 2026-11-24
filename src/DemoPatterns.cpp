// Лабораторна 10: демонстрація патернів State, Strategy, Observer та Facade
#include "Demos.h"

#include <filesystem>
#include <iomanip>
#include <memory>

#include "FareStrategy.h"
#include "SQLiteDriverRepository.h"
#include "SQLiteTransactionRunner.h"
#include "TestData.h"
#include "TripObservers.h"
#include "TripRepository.h"
#include "TripService.h"
#include "TripStatusHistoryRepository.h"

namespace
{
    void printTrip(const TripService& service, int tripId)
    {
        auto trip = service.findTrip(tripId);
        if (!trip)
        {
            std::cout << "Trip not found\n";
            return;
        }
        std::cout << "Trip #" << trip->id
                  << " | " << trip->distanceKm << " km"
                  << " | " << trip->durationMin << " min"
                  << " | fare " << trip->fare
                  << " | status " << trip->status << '\n';
    }

    void printHistory(const TripService& service, int tripId)
    {
        std::cout << "History of trip #" << tripId << ":\n";
        for (const auto& entry : service.historyOf(tripId))
        {
            std::cout << "  " << entry.changedAt << " | "
                      << (entry.oldStatus.empty() ? "-" : entry.oldStatus)
                      << " -> " << entry.newStatus << '\n';
        }
    }

    template <typename Operation>
    void attempt(const std::string& title, Operation operation)
    {
        try
        {
            operation();
            std::cout << title << ": done\n";
        }
        catch (const std::exception& ex)
        {
            std::cout << title << ": rejected (" << ex.what() << ")\n";
        }
    }

    void printStatistics(const TripStatistics& statistics)
    {
        std::cout << "Statistics: created " << statistics.createdCount()
                  << ", completed " << statistics.completedCount()
                  << ", revenue " << statistics.totalRevenue()
                  << ", distance " << statistics.totalDistanceKm() << " km"
                  << ", average fare " << statistics.averageFare() << '\n';
    }

    NewTripRequest sampleTrip(int driverId, double distanceKm, int durationMin)
    {
        return {driverId, "2026-11-23", "Podil", "Obolon", distanceKm, durationMin};
    }
}

void runPatternsDemo()
{
    std::cout << std::fixed << std::setprecision(2);

    const std::string databaseFile = "taxi_patterns.db";
    std::filesystem::remove(databaseFile);

    auto storage = createStorage(databaseFile);
    configureStorage(storage);
    seedTestData(storage);

    SQLiteDriverRepository drivers(storage);
    TripRepository trips(storage);
    TripStatusHistoryRepository histories(storage);
    SQLiteTransactionRunner transactions(storage);
    TripEventPublisher publisher;

    TripService service(drivers, trips, histories, transactions, publisher,
                        std::make_unique<DistanceFareStrategy>(), 50.0);
    const int driverId = 1;

    // ---------------- Strategy ----------------
    std::cout << "\n=== STRATEGY: FARE CALCULATION ===\n";
    const FareInput input{12.0, 30, 50.0};
    FareCalculator calculator(std::make_unique<DistanceFareStrategy>());
    std::cout << "Input: 12 km, 30 min, base rate 50\n";
    std::cout << calculator.strategyName() << ": " << calculator.calculate(input) << '\n';
    calculator.setStrategy(std::make_unique<TimeFareStrategy>());
    std::cout << calculator.strategyName() << ": " << calculator.calculate(input) << '\n';
    calculator.setStrategy(std::make_unique<CombinedFareStrategy>());
    std::cout << calculator.strategyName() << ": " << calculator.calculate(input) << '\n';

    std::cout << "\nStrategy switched at runtime in the service:\n";
    std::cout << "Current strategy: " << service.fareStrategyName() << '\n';
    printTrip(service, service.createTrip(sampleTrip(driverId, 12.0, 30)));
    service.setFareStrategy(std::make_unique<TimeFareStrategy>());
    std::cout << "Current strategy: " << service.fareStrategyName() << '\n';
    printTrip(service, service.createTrip(sampleTrip(driverId, 12.0, 30)));
    service.setFareStrategy(std::make_unique<CombinedFareStrategy>());
    std::cout << "Current strategy: " << service.fareStrategyName() << '\n';
    printTrip(service, service.createTrip(sampleTrip(driverId, 12.0, 30)));

    // ---------------- State ----------------
    std::cout << "\n=== STATE: TRIP LIFECYCLE ===\n";
    const int completedId = service.createTrip(sampleTrip(driverId, 8.0, 20));
    printTrip(service, completedId);
    attempt("Complete before start", [&] { service.completeTrip(completedId, 20); });
    attempt("Start", [&] { service.startTrip(completedId); });
    attempt("Start again", [&] { service.startTrip(completedId); });
    attempt("Complete", [&] { service.completeTrip(completedId, 24); });
    attempt("Cancel completed trip", [&] { service.cancelTrip(completedId); });
    printTrip(service, completedId);
    printHistory(service, completedId);

    std::cout << "\nCancelled trip:\n";
    const int cancelledId = service.createTrip(sampleTrip(driverId, 5.0, 12));
    attempt("Cancel", [&] { service.cancelTrip(cancelledId); });
    attempt("Start cancelled trip", [&] { service.startTrip(cancelledId); });
    printHistory(service, cancelledId);

    // ---------------- Observer ----------------
    std::cout << "\n=== OBSERVER: EVENTS AND SUBSCRIBERS ===\n";
    auto journal = std::make_shared<DriverJournal>(&std::cout);
    auto statistics = std::make_shared<TripStatistics>();
    publisher.subscribe(journal);
    publisher.subscribe(statistics);
    std::cout << "Subscribers: " << publisher.subscriberCount() << '\n';

    const int firstId = service.createTrip(sampleTrip(driverId, 10.0, 25));
    service.startTrip(firstId);
    service.completeTrip(firstId, 28);
    printStatistics(*statistics);

    std::cout << "\nStatistics unsubscribed:\n";
    publisher.unsubscribe(statistics);
    std::cout << "Subscribers: " << publisher.subscriberCount() << '\n';
    const int secondId = service.createTrip(sampleTrip(driverId, 6.0, 15));
    service.startTrip(secondId);
    service.completeTrip(secondId, 16);
    printStatistics(*statistics);

    std::cout << "\nEntries of driver " << driverId << " in the journal: "
              << journal->entriesFor(driverId).size() << '\n';
}
