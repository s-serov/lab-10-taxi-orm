#include <gtest/gtest.h>
#include "TripState.h"

namespace
{
    Trip tripWithStatus(const std::string& status)
    {
        Trip trip;
        trip.id = 1;
        trip.status = status;
        return trip;
    }
}

TEST(State, CreatedToInProgress)
{
    TripContext context(tripWithStatus(TripStatus::Created));
    context.start();
    EXPECT_EQ(context.stateName(), "InProgress");
    EXPECT_EQ(context.trip().status, "InProgress");
}

TEST(State, InProgressToCompleted)
{
    TripContext context(tripWithStatus(TripStatus::InProgress));
    context.complete();
    EXPECT_EQ(context.stateName(), "Completed");
}

TEST(State, CreatedToCancelled)
{
    TripContext context(tripWithStatus(TripStatus::Created));
    context.cancel();
    EXPECT_EQ(context.stateName(), "Cancelled");
}

TEST(State, InProgressToCancelled)
{
    TripContext context(tripWithStatus(TripStatus::InProgress));
    context.cancel();
    EXPECT_EQ(context.stateName(), "Cancelled");
}

TEST(State, CreatedCannotBeCompleted)
{
    TripContext context(tripWithStatus(TripStatus::Created));
    EXPECT_THROW(context.complete(), InvalidTransitionError);
    EXPECT_EQ(context.stateName(), "Created");
    EXPECT_EQ(context.trip().status, "Created");
}

TEST(State, InProgressCannotBeStartedAgain)
{
    TripContext context(tripWithStatus(TripStatus::InProgress));
    EXPECT_THROW(context.start(), InvalidTransitionError);
    EXPECT_EQ(context.stateName(), "InProgress");
}

TEST(State, CompletedIsFinal)
{
    TripContext context(tripWithStatus(TripStatus::Completed));
    EXPECT_THROW(context.start(), InvalidTransitionError);
    EXPECT_THROW(context.complete(), InvalidTransitionError);
    EXPECT_THROW(context.cancel(), InvalidTransitionError);
    EXPECT_EQ(context.stateName(), "Completed");
}

TEST(State, CancelledIsFinal)
{
    TripContext context(tripWithStatus(TripStatus::Cancelled));
    EXPECT_THROW(context.start(), InvalidTransitionError);
    EXPECT_THROW(context.complete(), InvalidTransitionError);
    EXPECT_THROW(context.cancel(), InvalidTransitionError);
    EXPECT_EQ(context.stateName(), "Cancelled");
}

TEST(State, StateIsRestoredFromStoredStatus)
{
    EXPECT_EQ(createTripState("Created")->name(), "Created");
    EXPECT_EQ(createTripState("InProgress")->name(), "InProgress");
    EXPECT_EQ(createTripState("Completed")->name(), "Completed");
    EXPECT_EQ(createTripState("Cancelled")->name(), "Cancelled");
}

TEST(State, UnknownStatusIsRejected)
{
    EXPECT_THROW(createTripState("Unknown"), std::invalid_argument);
    EXPECT_THROW(TripContext(tripWithStatus("Broken")), std::invalid_argument);
}
