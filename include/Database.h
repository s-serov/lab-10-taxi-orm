// Завдання 3
#pragma once

#include <stdexcept>
#include <string>
#include <sqlite_orm/sqlite_orm.h>

#include "Driver.h"
#include "Trip.h"
#include "TripStatusHistory.h"

inline auto createStorage(const std::string& filename)
{
    using namespace sqlite_orm;
    return make_storage(
        filename,
        make_table(
            "Driver",
            make_column("id", &Driver::id, primary_key().autoincrement()),
            make_column("name", &Driver::name, not_null()),
            make_column("carModel", &Driver::carModel, not_null()),
            make_column("licensePlate", &Driver::licensePlate, not_null()),
            make_column("status", &Driver::status, not_null())
        ),
        make_table(
            "Trip",
            make_column("id", &Trip::id, primary_key().autoincrement()),
            make_column("driverId", &Trip::driverId, not_null()),
            make_column("tripDate", &Trip::tripDate, not_null()),
            make_column("pickupAddress", &Trip::pickupAddress, not_null()),
            make_column("dropoffAddress", &Trip::dropoffAddress, not_null()),
            make_column("distanceKm", &Trip::distanceKm, not_null()),
            make_column("fare", &Trip::fare, not_null()),
            make_column("durationMin", &Trip::durationMin, not_null()),
            make_column("status", &Trip::status, not_null()),
            foreign_key(&Trip::driverId).references(&Driver::id)
        ),
        make_table(
            "TripStatusHistory",
            make_column("id", &TripStatusHistory::id, primary_key().autoincrement()),
            make_column("tripId", &TripStatusHistory::tripId, not_null()),
            make_column("oldStatus", &TripStatusHistory::oldStatus, not_null()),
            make_column("newStatus", &TripStatusHistory::newStatus, not_null()),
            make_column("changedAt", &TripStatusHistory::changedAt, not_null()),
            foreign_key(&TripStatusHistory::tripId).references(&Trip::id)
        )
    );
}

using Storage = decltype(createStorage(""));

// Вмикає перевірку зовнішніх ключів, утримує з'єднання та синхронізує схему
inline void configureStorage(Storage& storage)
{
    storage.on_open = [](sqlite3* db)
    {
        if (sqlite3_exec(db, "PRAGMA foreign_keys = ON;",
                         nullptr, nullptr, nullptr) != SQLITE_OK)
        {
            throw std::runtime_error("Cannot enable foreign keys");
        }
    };
    storage.open_forever();
    storage.sync_schema();
}
