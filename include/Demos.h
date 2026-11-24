// Завдання 2-6 (лаб. 8), патерни (лаб. 10)
#pragma once

#include <iostream>
#include <optional>
#include "Database.h"
#include "Driver.h"

inline void printDriver(const std::optional<Driver>& driver)
{
    if (!driver)
    {
        std::cout << "Driver not found\n";
        return;
    }
    std::cout << "ID: " << driver->id
              << " | Name: " << driver->name
              << " | Car: " << driver->carModel
              << " | Plate: " << driver->licensePlate
              << " | Status: " << driver->status << '\n';
}

void runCompareDemo();
void runRepositoryDemo(Storage& storage);
void runQueryDemo(Storage& storage);
void runTransactionDemo(Storage& storage);
void runWithoutTransactionDemo();
void runPatternsDemo();
