cmake_minimum_required(VERSION 3.21)
project(TaxiORM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SqliteOrm CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# Ядро: моделі, ORM, репозиторії та патерни State, Strategy, Observer, Facade
add_library(taxi_core STATIC
    src/RawSqlite.cpp
    src/SQLiteDriverRepository.cpp
    src/TripRepository.cpp
    src/TripStatusHistoryRepository.cpp
    src/TestData.cpp
    src/TripState.cpp
    src/FareStrategy.cpp
    src/TripEvents.cpp
    src/TripObservers.cpp
    src/TripService.cpp
)
target_include_directories(taxi_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(taxi_core PUBLIC
    sqlite_orm::sqlite_orm
    unofficial::sqlite3::sqlite3
)

add_executable(TaxiORM
    src/main.cpp
    src/DemoCompare.cpp
    src/DemoRepository.cpp
    src/DemoQueries.cpp
    src/DemoTransactions.cpp
    src/DemoPatterns.cpp
)
target_link_libraries(TaxiORM PRIVATE taxi_core)

# Модульні тести
enable_testing()
add_executable(taxi_tests
    tests/StateTests.cpp
    tests/StrategyTests.cpp
    tests/ObserverTests.cpp
    tests/TripServiceTests.cpp
    tests/SqliteIntegrationTests.cpp
)
target_link_libraries(taxi_tests PRIVATE taxi_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(taxi_tests DISCOVERY_MODE PRE_TEST)
