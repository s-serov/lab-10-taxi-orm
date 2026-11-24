// Завдання 6
#include "Demos.h"

#include <filesystem>

namespace
{
    void printCounts(const char* title, Storage& storage)
    {
        std::cout << title << '\n';
        std::cout << "Drivers: " << storage.count<Driver>() << '\n';
        std::cout << "Trips: " << storage.count<Trip>() << '\n';
    }

    bool driverExists(Storage& storage, const std::string& plate)
    {
        using namespace sqlite_orm;
        return !storage.get_all<Driver>(
            where(c(&Driver::licensePlate) == plate)
        ).empty();
    }
}

void runTransactionDemo(Storage& storage)
{
    std::cout << "\n=== TRANSACTION DEMONSTRATION ===\n";
    std::cout << std::boolalpha;

    // Сценарій 1: COMMIT
    std::cout << "\n--- Scenario 1: success (COMMIT) ---\n";
    printCounts("Before:", storage);

    bool committed = storage.transaction([&]()
    {
        int driverId = storage.insert(
            Driver{0, "Oleh Savchuk", "Kia Rio", "KC2222AB", "active"});
        storage.insert(
            Trip{0, driverId, "2026-11-05", "Airport Zhuliany", "Central Station", 18.5, 420.0});
        return true;
    });

    std::cout << "Committed: " << committed << '\n';
    printCounts("After:", storage);
    std::cout << "Driver exists: " << driverExists(storage, "KC2222AB") << '\n';

    // Сценарій 2: ROLLBACK через виняток (порушення FOREIGN KEY)
    std::cout << "\n--- Scenario 2: error in second operation (ROLLBACK) ---\n";
    printCounts("Before:", storage);

    try
    {
        storage.transaction([&]()
        {
            int driverId = storage.insert(
                Driver{0, "Rollback Test", "Ford Focus", "KD3333AB", "active"});
            std::cout << "Driver inserted inside transaction, ID: " << driverId << '\n';

            storage.insert(
                Trip{0, -1, "2026-11-06", "Podil", "Obolon", 9.0, 200.0});
            return true;
        });
    }
    catch (const std::exception& ex)
    {
        std::cout << "Transaction failed: " << ex.what() << '\n';
    }

    printCounts("After:", storage);
    std::cout << "Driver exists: " << driverExists(storage, "KD3333AB") << '\n';

    // Сценарій 3: ROLLBACK через return false
    std::cout << "\n--- Scenario 3: function returned false (ROLLBACK) ---\n";
    printCounts("Before:", storage);

    bool result = storage.transaction([&]()
    {
        int driverId = storage.insert(
            Driver{0, "Cancelled Driver", "Opel Astra", "KE4444AB", "active"});
        storage.insert(
            Trip{0, driverId, "2026-11-06", "Lisova", "Pozniaky", 12.0, 250.0});
        return false;
    });

    std::cout << "Committed: " << result << '\n';
    printCounts("After:", storage);
    std::cout << "Driver exists: " << driverExists(storage, "KE4444AB") << '\n';
}

void runWithoutTransactionDemo()
{
    std::cout << "\n=== WITHOUT TRANSACTION ===\n";

    const std::string databaseFile = "taxi_no_transaction.db";
    std::filesystem::remove(databaseFile);

    auto storage = createStorage(databaseFile);
    configureStorage(storage);

    printCounts("Before:", storage);

    try
    {
        int driverId = storage.insert(
            Driver{0, "Rollback Test", "Ford Focus", "KD3333AB", "active"});
        std::cout << "Driver inserted, ID: " << driverId << '\n';

        storage.insert(
            Trip{0, -1, "2026-11-06", "Podil", "Obolon", 9.0, 200.0});
    }
    catch (const std::exception& ex)
    {
        std::cout << "Operation failed: " << ex.what() << '\n';
    }

    printCounts("After:", storage);
    std::cout << "Driver remained in database: "
              << std::boolalpha << driverExists(storage, "KD3333AB") << '\n';
}
