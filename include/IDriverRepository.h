// Завдання 4
#pragma once

#include <optional>
#include <vector>
#include "Driver.h"

class IDriverRepository
{
public:
    virtual ~IDriverRepository() = default;

    virtual int add(const Driver& driver) = 0;
    virtual std::optional<Driver> findById(int id) const = 0;
    virtual std::vector<Driver> findAll() const = 0;
    virtual bool update(const Driver& driver) = 0;
    virtual bool remove(int id) = 0;
};
