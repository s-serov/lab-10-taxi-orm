#pragma once

#include <map>
#include <memory>
#include <stdexcept>
#include <vector>

#include "IDriverRepository.h"
#include "ITransactionRunner.h"
#include "ITripHistoryRepository.h"
#include "ITripRepository.h"
#include "TripEvents.h"

struct FakeDatabase
{
    std::map<int, Driver> drivers;
    std::map<int, Trip> trips;
    std::vector<TripStatusHistory> history;
    int nextTripId = 1;
    int nextHistoryId = 1;
};

class FakeDriverRepository : public IDriverRepository
{
public:
    explicit FakeDriverRepository(FakeDatabase& db) : db_(db) {}

    int add(const Driver& driver) override
    {
        Driver stored = driver;
        stored.id = static_cast<int>(db_.drivers.size()) + 1;
        db_.drivers[stored.id] = stored;
        return stored.id;
    }
    std::optional<Driver> findById(int id) const override
    {
        auto it = db_.drivers.find(id);
        return it == db_.drivers.end() ? std::nullopt : std::optional<Driver>(it->second);
    }
    std::vector<Driver> findAll() const override
    {
        std::vector<Driver> result;
        for (const auto& [id, driver] : db_.drivers) result.push_back(driver);
        return result;
    }
    bool update(const Driver& driver) override
    {
        if (!db_.drivers.count(driver.id)) return false;
        db_.drivers[driver.id] = driver;
        return true;
    }
    bool remove(int id) override { return db_.drivers.erase(id) > 0; }

private:
    FakeDatabase& db_;
};

class FakeTripRepository : public ITripRepository
{
public:
    explicit FakeTripRepository(FakeDatabase& db) : db_(db) {}

    int add(const Trip& trip) override
    {
        Trip stored = trip;
        stored.id = db_.nextTripId++;
        db_.trips[stored.id] = stored;
        return stored.id;
    }
    std::optional<Trip> findById(int id) const override
    {
        auto it = db_.trips.find(id);
        return it == db_.trips.end() ? std::nullopt : std::optional<Trip>(it->second);
    }
    std::vector<Trip> findAll() const override
    {
        std::vector<Trip> result;
        for (const auto& [id, trip] : db_.trips) result.push_back(trip);
        return result;
    }
    std::vector<Trip> findByDriverId(int driverId) const override
    {
        std::vector<Trip> result;
        for (const auto& [id, trip] : db_.trips)
            if (trip.driverId == driverId) result.push_back(trip);
        return result;
    }
    bool update(const Trip& trip) override
    {
        if (!db_.trips.count(trip.id)) return false;
        db_.trips[trip.id] = trip;
        return true;
    }

private:
    FakeDatabase& db_;
};

class FakeHistoryRepository : public ITripHistoryRepository
{
public:
    explicit FakeHistoryRepository(FakeDatabase& db) : db_(db) {}

    int add(const TripStatusHistory& entry) override
    {
        if (failOnAdd) throw std::runtime_error("History write failed");
        TripStatusHistory stored = entry;
        stored.id = db_.nextHistoryId++;
        db_.history.push_back(stored);
        return stored.id;
    }
    std::vector<TripStatusHistory> findByTripId(int tripId) const override
    {
        std::vector<TripStatusHistory> result;
        for (const auto& entry : db_.history)
            if (entry.tripId == tripId) result.push_back(entry);
        return result;
    }

    bool failOnAdd = false;

private:
    FakeDatabase& db_;
};

// Імітує транзакцію: при винятку відновлює знімок даних
class FakeTransactionRunner : public ITransactionRunner
{
public:
    explicit FakeTransactionRunner(FakeDatabase& db) : db_(db) {}

    void run(const std::function<void()>& work) override
    {
        const FakeDatabase snapshot = db_;
        try
        {
            work();
        }
        catch (...)
        {
            db_ = snapshot;
            throw;
        }
    }

private:
    FakeDatabase& db_;
};

class RecordingObserver : public ITripObserver
{
public:
    void update(const TripEvent& event) override { events.push_back(event); }
    std::vector<TripEvent> events;
};

class ThrowingObserver : public ITripObserver
{
public:
    void update(const TripEvent&) override { throw std::runtime_error("observer failure"); }
};
