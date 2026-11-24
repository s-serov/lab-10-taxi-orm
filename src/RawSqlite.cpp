// Завдання 2: SQLite C API
#include "RawSqlite.h"

#include <stdexcept>
#include <string>

namespace
{
    [[noreturn]] void throwError(sqlite3* db, sqlite3_stmt* stmt)
    {
        std::string message = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw std::runtime_error(message);
    }

    std::string columnText(sqlite3_stmt* stmt, int index)
    {
        const unsigned char* text = sqlite3_column_text(stmt, index);
        return text ? reinterpret_cast<const char*>(text) : "";
    }
}

void rawCreateSchema(sqlite3* db)
{
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS Driver (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            carModel TEXT NOT NULL,
            licensePlate TEXT NOT NULL,
            status TEXT NOT NULL
        );
    )";

    char* errorMessage = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errorMessage) != SQLITE_OK)
    {
        std::string message = errorMessage ? errorMessage : "Unknown error";
        sqlite3_free(errorMessage);
        throw std::runtime_error(message);
    }
}

int rawInsertDriver(sqlite3* db, const Driver& driver)
{
    const char* sql = R"(
        INSERT INTO Driver (name, carModel, licensePlate, status)
        VALUES (?, ?, ?, ?);
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        throwError(db, stmt);
    }

    int rc = sqlite3_bind_text(stmt, 1, driver.name.c_str(), -1, SQLITE_TRANSIENT);
    if (rc == SQLITE_OK)
        rc = sqlite3_bind_text(stmt, 2, driver.carModel.c_str(), -1, SQLITE_TRANSIENT);
    if (rc == SQLITE_OK)
        rc = sqlite3_bind_text(stmt, 3, driver.licensePlate.c_str(), -1, SQLITE_TRANSIENT);
    if (rc == SQLITE_OK)
        rc = sqlite3_bind_text(stmt, 4, driver.status.c_str(), -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK)
    {
        throwError(db, stmt);
    }

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        throwError(db, stmt);
    }

    int id = static_cast<int>(sqlite3_last_insert_rowid(db));
    sqlite3_finalize(stmt);
    return id;
}

std::optional<Driver> rawFindDriverById(sqlite3* db, int id)
{
    const char* sql = R"(
        SELECT id, name, carModel, licensePlate, status
        FROM Driver
        WHERE id = ?;
    )";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
    {
        throwError(db, stmt);
    }

    if (sqlite3_bind_int(stmt, 1, id) != SQLITE_OK)
    {
        throwError(db, stmt);
    }

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW)
    {
        Driver driver;
        driver.id = sqlite3_column_int(stmt, 0);
        driver.name = columnText(stmt, 1);
        driver.carModel = columnText(stmt, 2);
        driver.licensePlate = columnText(stmt, 3);
        driver.status = columnText(stmt, 4);
        sqlite3_finalize(stmt);
        return driver;
    }

    if (rc != SQLITE_DONE)
    {
        throwError(db, stmt);
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}
