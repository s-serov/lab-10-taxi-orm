// Патерн State: стан поїздки
#include "TripState.h"

void TripState::reject(const std::string& operation) const
{
    throw InvalidTransitionError("Cannot " + operation + " a trip in state " + name());
}

void CreatedState::start(TripContext& context)
{
    context.setState(std::make_unique<InProgressState>());
}

void CreatedState::complete(TripContext&)
{
    reject("complete");
}

void CreatedState::cancel(TripContext& context)
{
    context.setState(std::make_unique<CancelledState>());
}

void InProgressState::start(TripContext&)
{
    reject("start");
}

void InProgressState::complete(TripContext& context)
{
    context.setState(std::make_unique<CompletedState>());
}

void InProgressState::cancel(TripContext& context)
{
    context.setState(std::make_unique<CancelledState>());
}

void CompletedState::start(TripContext&)
{
    reject("start");
}

void CompletedState::complete(TripContext&)
{
    reject("complete");
}

void CompletedState::cancel(TripContext&)
{
    reject("cancel");
}

void CancelledState::start(TripContext&)
{
    reject("start");
}

void CancelledState::complete(TripContext&)
{
    reject("complete");
}

void CancelledState::cancel(TripContext&)
{
    reject("cancel");
}

std::unique_ptr<TripState> createTripState(const std::string& status)
{
    if (status == TripStatus::Created)
        return std::make_unique<CreatedState>();
    if (status == TripStatus::InProgress)
        return std::make_unique<InProgressState>();
    if (status == TripStatus::Completed)
        return std::make_unique<CompletedState>();
    if (status == TripStatus::Cancelled)
        return std::make_unique<CancelledState>();
    throw std::invalid_argument("Unknown trip status: " + status);
}

TripContext::TripContext(Trip trip)
    : trip_(std::move(trip)),
      state_(createTripState(trip_.status))
{
}

void TripContext::start()
{
    state_->start(*this);
}

void TripContext::complete()
{
    state_->complete(*this);
}

void TripContext::cancel()
{
    state_->cancel(*this);
}

void TripContext::setState(std::unique_ptr<TripState> next)
{
    state_ = std::move(next);
    trip_.status = state_->name();
}

std::string TripContext::stateName() const
{
    return state_->name();
}
