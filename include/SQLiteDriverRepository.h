// Завдання 4
#pragma once

#include "Database.h"
#include "IDriverRepository.h"

class SQLiteDriverRepository : public IDriverRepository
{
public:
    explicit SQLiteDriverRepository(Storage& storage);

    int add(const Driver& driver) override;
    std::optional<Driver> findById(int id) const override;
    std::vector<Driver> findAll() const override;
    bool update(const Driver& driver) override;
    bool remove(int id) override;

private:
    Storage& storage_;
};
