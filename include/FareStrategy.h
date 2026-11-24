// Патерн Strategy: розрахунок вартості поїздки
#pragma once

#include <memory>
#include <string>

struct FareInput
{
    double distanceKm = 0.0;
    int durationMin = 0;
    double baseRate = 0.0;
};

class IFareStrategy
{
public:
    virtual ~IFareStrategy() = default;

    virtual double calculate(const FareInput& input) const = 0;
    virtual std::string name() const = 0;
};

// Базова ставка + плата за відстань
class DistanceFareStrategy : public IFareStrategy
{
public:
    explicit DistanceFareStrategy(double perKm = 12.0);

    double calculate(const FareInput& input) const override;
    std::string name() const override { return "By distance"; }

private:
    double perKm_;
};

// Базова ставка + плата за час
class TimeFareStrategy : public IFareStrategy
{
public:
    explicit TimeFareStrategy(double perMinute = 4.0);

    double calculate(const FareInput& input) const override;
    std::string name() const override { return "By time"; }

private:
    double perMinute_;
};

// Базова ставка + плата за відстань і за час
class CombinedFareStrategy : public IFareStrategy
{
public:
    CombinedFareStrategy(double perKm = 8.0, double perMinute = 2.0);

    double calculate(const FareInput& input) const override;
    std::string name() const override { return "Combined"; }

private:
    double perKm_;
    double perMinute_;
};

// Контекст: зберігає поточну стратегію, дає змогу замінювати її під час роботи
class FareCalculator
{
public:
    explicit FareCalculator(std::unique_ptr<IFareStrategy> strategy);

    void setStrategy(std::unique_ptr<IFareStrategy> strategy);
    double calculate(const FareInput& input) const;
    std::string strategyName() const;

private:
    std::unique_ptr<IFareStrategy> strategy_;
};
