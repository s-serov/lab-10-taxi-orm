// Завдання 2
#pragma once

#include <optional>
#include <sqlite3.h>
#include "Driver.h"

void rawCreateSchema(sqlite3* db);
int rawInsertDriver(sqlite3* db, const Driver& driver);
std::optional<Driver> rawFindDriverById(sqlite3* db, int id);
