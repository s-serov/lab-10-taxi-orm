// Завдання 1-6 (лаб. 8) та демонстрація патернів (лаб. 10)
#include <filesystem>
#include <iostream>
#include <string>

#include "Database.h"
#include "Demos.h"
#include "TestData.h"

namespace
{
    void runLab8Demos()
    {
        // Завдання 2
        runCompareDemo();

        // Завдання 3
        const std::string databaseFile = "taxi.db";
        std::filesystem::remove(databaseFile);

        auto storage = createStorage(databaseFile);
        configureStorage(storage);

        std::cout << "\n=== SCHEMA AND TEST DATA ===\n";
        printSchemaInfo(storage);
        seedTestData(storage);

        // Завдання 4
        runRepositoryDemo(storage);

        // Завдання 5
        runQueryDemo(storage);

        // Завдання 6
        runTransactionDemo(storage);
        runWithoutTransactionDemo();
    }
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc > 1 && std::string(argv[1]) == "lab8")
        {
            runLab8Demos();
        }
        else
        {
            runPatternsDemo();
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Application error: " << ex.what() << '\n';
        return 1;
    }

    return 0;
}
