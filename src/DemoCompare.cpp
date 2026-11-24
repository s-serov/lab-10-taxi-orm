// Завдання 2: порівняння SQLite C API та sqlite_orm
#include "Demos.h"
#include "RawSqlite.h"

#include <filesystem>
#include <stdexcept>
#include <string>

void runCompareDemo()
{
    std::cout << "\n=== SQLITE C API vs ORM ===\n";

    Driver driver{0, "Ivan Petrenko", "Toyota Camry", "AA1234KK", "active"};

    // --- SQLite C API ---
    const std::string rawFile = "taxi_raw.db";
    std::filesystem::remove(rawFile);

    sqlite3* db = nullptr;
    if (sqlite3_open(rawFile.c_str(), &db) != SQLITE_OK)
    {
        std::string message = sqlite3_errmsg(db);
        sqlite3_close(db);
        throw std::runtime_error(message);
    }

    try
    {
        rawCreateSchema(db);
        int id = rawInsertDriver(db, driver);
        std::cout << "\n[C API] Inserted driver ID: " << id << '\n';
        printDriver(rawFindDriverById(db, id));
    }
    catch (...)
    {
        sqlite3_close(db);
        throw;
    }
    sqlite3_close(db);

    // --- sqlite_orm ---
    const std::string ormFile = "taxi_orm.db";
    std::filesystem::remove(ormFile);

    auto storage = createStorage(ormFile);
    configureStorage(storage);

    int id = storage.insert(driver);
    std::cout << "\n[ORM] Inserted driver ID: " << id << '\n';
    printDriver(storage.get_optional<Driver>(id));
}
