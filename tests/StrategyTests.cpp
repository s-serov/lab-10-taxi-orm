#include <gtest/gtest.h>
#include "FareStrategy.h"

namespace
{
    const FareInput kInput{10.0, 25, 50.0};
}

TEST(Strategy, DistanceFare)
{
    DistanceFareStrategy strategy;
    EXPECT_DOUBLE_EQ(strategy.calculate(kInput), 170.0);
}

TEST(Strategy, TimeFare)
{
    TimeFareStrategy strategy;
    EXPECT_DOUBLE_EQ(strategy.calculate(kInput), 150.0);
}

TEST(Strategy, CombinedFare)
{
    CombinedFareStrategy strategy;
    EXPECT_DOUBLE_EQ(strategy.calculate(kInput), 180.0);
}

TEST(Strategy, CustomTariffs)
{
    EXPECT_DOUBLE_EQ(DistanceFareStrategy(10.0).calculate(kInput), 150.0);
    EXPECT_DOUBLE_EQ(TimeFareStrategy(2.0).calculate(kInput), 100.0);
    EXPECT_DOUBLE_EQ(CombinedFareStrategy(5.0, 1.0).calculate(kInput), 125.0);
}

TEST(Strategy, StrategyNames)
{
    EXPECT_EQ(DistanceFareStrategy().name(), "By distance");
    EXPECT_EQ(TimeFareStrategy().name(), "By time");
    EXPECT_EQ(CombinedFareStrategy().name(), "Combined");
}

TEST(Strategy, CalculatorSwitchesStrategyAtRuntime)
{
    FareCalculator calculator(std::make_unique<DistanceFareStrategy>());
    EXPECT_DOUBLE_EQ(calculator.calculate(kInput), 170.0);
    EXPECT_EQ(calculator.strategyName(), "By distance");

    calculator.setStrategy(std::make_unique<TimeFareStrategy>());
    EXPECT_DOUBLE_EQ(calculator.calculate(kInput), 150.0);
    EXPECT_EQ(calculator.strategyName(), "By time");

    calculator.setStrategy(std::make_unique<CombinedFareStrategy>());
    EXPECT_DOUBLE_EQ(calculator.calculate(kInput), 180.0);
}

TEST(Strategy, ResultIsRoundedToTwoDecimals)
{
    FareCalculator calculator(std::make_unique<DistanceFareStrategy>(12.345));
    EXPECT_DOUBLE_EQ(calculator.calculate({1.0, 0, 0.0}), 12.35);
}

TEST(Strategy, NegativeParametersAreRejected)
{
    FareCalculator calculator(std::make_unique<DistanceFareStrategy>());
    EXPECT_THROW(calculator.calculate({-1.0, 10, 50.0}), std::invalid_argument);
    EXPECT_THROW(calculator.calculate({1.0, -10, 50.0}), std::invalid_argument);
    EXPECT_THROW(calculator.calculate({1.0, 10, -50.0}), std::invalid_argument);
}

TEST(Strategy, NullStrategyIsRejected)
{
    EXPECT_THROW(FareCalculator(nullptr), std::invalid_argument);
    FareCalculator calculator(std::make_unique<TimeFareStrategy>());
    EXPECT_THROW(calculator.setStrategy(nullptr), std::invalid_argument);
    EXPECT_EQ(calculator.strategyName(), "By time");
}
