// Патерн Strategy: розрахунок вартості поїздки
#include "FareStrategy.h"

#include <cmath>
#include <stdexcept>

DistanceFareStrategy::DistanceFareStrategy(double perKm)
    : perKm_(perKm)
{
}

double DistanceFareStrategy::calculate(const FareInput& input) const
{
    return input.baseRate + input.distanceKm * perKm_;
}

TimeFareStrategy::TimeFareStrategy(double perMinute)
    : perMinute_(perMinute)
{
}

double TimeFareStrategy::calculate(const FareInput& input) const
{
    return input.baseRate + input.durationMin * perMinute_;
}

CombinedFareStrategy::CombinedFareStrategy(double perKm, double perMinute)
    : perKm_(perKm), perMinute_(perMinute)
{
}

double CombinedFareStrategy::calculate(const FareInput& input) const
{
    return input.baseRate + input.distanceKm * perKm_ + input.durationMin * perMinute_;
}

FareCalculator::FareCalculator(std::unique_ptr<IFareStrategy> strategy)
{
    setStrategy(std::move(strategy));
}

void FareCalculator::setStrategy(std::unique_ptr<IFareStrategy> strategy)
{
    if (!strategy)
    {
        throw std::invalid_argument("Fare strategy must not be null");
    }
    strategy_ = std::move(strategy);
}

double FareCalculator::calculate(const FareInput& input) const
{
    if (input.distanceKm < 0.0 || input.durationMin < 0 || input.baseRate < 0.0)
    {
        throw std::invalid_argument("Fare parameters must not be negative");
    }
    return std::round(strategy_->calculate(input) * 100.0) / 100.0;
}

std::string FareCalculator::strategyName() const
{
    return strategy_->name();
}
