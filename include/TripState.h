// Патерн State: стан поїздки
#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include "Trip.h"

class InvalidTransitionError : public std::logic_error
{
public:
    using std::logic_error::logic_error;
};

class TripContext;

class TripState
{
public:
    virtual ~TripState() = default;

    virtual void start(TripContext& context) = 0;
    virtual void complete(TripContext& context) = 0;
    virtual void cancel(TripContext& context) = 0;
    virtual std::string name() const = 0;

protected:
    [[noreturn]] void reject(const std::string& operation) const;
};

class CreatedState : public TripState
{
public:
    void start(TripContext& context) override;
    void complete(TripContext& context) override;
    void cancel(TripContext& context) override;
    std::string name() const override { return TripStatus::Created; }
};

class InProgressState : public TripState
{
public:
    void start(TripContext& context) override;
    void complete(TripContext& context) override;
    void cancel(TripContext& context) override;
    std::string name() const override { return TripStatus::InProgress; }
};

class CompletedState : public TripState
{
public:
    void start(TripContext& context) override;
    void complete(TripContext& context) override;
    void cancel(TripContext& context) override;
    std::string name() const override { return TripStatus::Completed; }
};

class CancelledState : public TripState
{
public:
    void start(TripContext& context) override;
    void complete(TripContext& context) override;
    void cancel(TripContext& context) override;
    std::string name() const override { return TripStatus::Cancelled; }
};

// Відновлює об'єкт стану за значенням, збереженим у базі даних
std::unique_ptr<TripState> createTripState(const std::string& status);

// Контекст: поїздка та її поточний стан
class TripContext
{
public:
    explicit TripContext(Trip trip);

    void start();
    void complete();
    void cancel();

    void setState(std::unique_ptr<TripState> next);
    std::string stateName() const;

    const Trip& trip() const { return trip_; }
    Trip& trip() { return trip_; }

private:
    Trip trip_;
    std::unique_ptr<TripState> state_;
};
