// Завдання 4: робота через інтерфейс базового класу
#include "Demos.h"
#include "SQLiteDriverRepository.h"
#include "TripRepository.h"

namespace
{
    void printAll(const IDriverRepository& repository)
    {
        for (const auto& driver : repository.findAll())
        {
            std::cout << driver.id << " | " << driver.name << " | "
                      << driver.licensePlate << " | " << driver.status << '\n';
        }
    }
}

void runRepositoryDemo(Storage& storage)
{
    std::cout << "\n=== REPOSITORY DEMONSTRATION ===\n";

    SQLiteDriverRepository sqliteRepository(storage);
    IDriverRepository& repository = sqliteRepository;
    TripRepository tripRepository(storage);

    std::cout << std::boolalpha;

    // CREATE
    int id = repository.add({0, "Taras Kravets", "Kia Ceed", "KB1111AB", "active"});
    std::cout << "\nCREATE\nInserted driver ID: " << id << '\n';

    // READ
    std::cout << "\nREAD\n";
    auto found = repository.findById(id);
    printDriver(found);
    std::cout << "Driver 9999: ";
    printDriver(repository.findById(9999));

    // UPDATE
    std::cout << "\nUPDATE\n";
    if (found)
    {
        found->status = "offline";
        std::cout << "Updated: " << repository.update(*found) << '\n';
        printDriver(repository.findById(id));
    }
    Driver missing{9999, "Nobody", "None", "000000", "offline"};
    std::cout << "Update of missing driver: " << repository.update(missing) << '\n';

    // READ ALL
    std::cout << "\nREAD ALL\n";
    printAll(repository);

    // Залежна сутність
    std::cout << "\nTRIPS OF DRIVER 1\n";
    for (const auto& trip : tripRepository.findByDriverId(1))
    {
        std::cout << trip.id << " | " << trip.tripDate << " | "
                  << trip.pickupAddress << " -> " << trip.dropoffAddress
                  << " | " << trip.fare << '\n';
    }

    // DELETE
    std::cout << "\nDELETE\n";
    try
    {
        repository.remove(1);
        std::cout << "Unexpected: driver with trips removed\n";
    }
    catch (const std::exception& ex)
    {
        std::cout << "Removing driver 1 rejected: " << ex.what() << '\n';
    }

    std::cout << "Removed new driver: " << repository.remove(id) << '\n';
    std::cout << "Removed again: " << repository.remove(id) << '\n';
    std::cout << "Exists after deletion: "
              << repository.findById(id).has_value() << '\n';
}
