// Завдання 3: початковий набір даних (5 водіїв, 15 поїздок)
#include "TestData.h"

#include <iostream>
#include <vector>

void seedTestData(Storage& storage)
{
    std::vector<Driver> drivers = {
        {0, "Ivan Petrenko",    "Toyota Camry",   "AA1234KK", "active"},
        {0, "Olena Koval",      "Skoda Octavia",  "KA5678BH", "active"},
        {0, "Marko Shevchenko", "Hyundai Sonata", "AX9012CE", "offline"},
        {0, "Andrii Bondar",    "Renault Logan",  "BC3456AA", "active"},
        {0, "Natalia Melnyk",   "Nissan Leaf",    "KI7890MM", "suspended"}
    };

    std::vector<int> ids;
    for (const auto& driver : drivers)
    {
        ids.push_back(storage.insert(driver));
    }

    std::vector<Trip> trips = {
        {0, ids[0], "2026-10-01", "Khreshchatyk 1",   "Airport Boryspil", 32.5, 650.0, 75, TripStatus::Completed},
        {0, ids[0], "2026-10-12", "Podil",            "Obolon",            9.8, 210.0, 23, TripStatus::Completed},
        {0, ids[0], "2026-10-25", "Lukianivska",      "Central Station",   5.2, 120.0, 12, TripStatus::Completed},

        {0, ids[1], "2026-10-03", "Obolon",           "Sviatoshyn",       14.1, 300.0, 32, TripStatus::Completed},
        {0, ids[1], "2026-10-15", "Airport Zhuliany", "Khreshchatyk 1",   11.0, 260.0, 25, TripStatus::Completed},
        {0, ids[1], "2026-10-28", "Darnytsia",        "Pozniaky",          7.4, 150.0, 17, TripStatus::Completed},

        {0, ids[2], "2026-09-20", "Teremky",          "Vokzalna",         16.3, 340.0, 37, TripStatus::Completed},
        {0, ids[2], "2026-10-02", "Libidska",         "Kontraktova",       4.1,  95.0, 9, TripStatus::Completed},
        {0, ids[2], "2026-10-18", "Shuliavka",        "Osokorky",         19.7, 410.0, 45, TripStatus::Completed},

        {0, ids[3], "2026-09-30", "Troieshchyna",     "Livoberezhna",      8.9, 180.0, 20, TripStatus::Completed},
        {0, ids[3], "2026-10-10", "Central Station",  "Airport Boryspil", 35.0, 700.0, 80, TripStatus::Completed},
        {0, ids[3], "2026-11-01", "Pecherska",        "Podil",             6.3, 140.0, 14, TripStatus::Completed},

        {0, ids[4], "2026-08-15", "Vynohradar",       "Minska",           10.5, 230.0, 24, TripStatus::Completed},
        {0, ids[4], "2026-09-05", "Lisova",           "Pozniaky",         12.2, 270.0, 28, TripStatus::Completed},
        {0, ids[4], "2026-10-22", "Kharkivska",       "Obolon",           21.0, 455.0, 48, TripStatus::Completed}
    };

    for (const auto& trip : trips)
    {
        storage.insert(trip);
    }

    std::cout << "Test database prepared\n";
    std::cout << "Driver records: " << storage.count<Driver>() << '\n';
    std::cout << "Trip records: " << storage.count<Trip>() << '\n';
}

void printSchemaInfo(Storage& storage)
{
    std::cout << std::boolalpha;
    std::cout << "Table Driver exists: " << storage.table_exists("Driver") << '\n';
    std::cout << "Table Trip exists: " << storage.table_exists("Trip") << '\n';

    // Перевірка: вставка поїздки з неіснуючим водієм у транзакції, яка скасовується
    bool enforced = false;
    storage.transaction([&]()
    {
        try
        {
            storage.insert(Trip{0, -1, "2026-01-01", "A", "B", 1.0, 1.0});
        }
        catch (const std::exception&)
        {
            enforced = true;
        }
        return false;
    });
    std::cout << "Foreign keys enforced: " << enforced << '\n';
}
