// Завдання 4
#include "SQLiteDriverRepository.h"

SQLiteDriverRepository::SQLiteDriverRepository(Storage& storage)
    : storage_(storage)
{
}

int SQLiteDriverRepository::add(const Driver& driver)
{
    return storage_.insert(driver);
}

std::optional<Driver> SQLiteDriverRepository::findById(int id) const
{
    return storage_.get_optional<Driver>(id);
}

std::vector<Driver> SQLiteDriverRepository::findAll() const
{
    return storage_.get_all<Driver>();
}

bool SQLiteDriverRepository::update(const Driver& driver)
{
    if (!storage_.get_optional<Driver>(driver.id))
    {
        return false;
    }
    storage_.update(driver);
    return true;
}

bool SQLiteDriverRepository::remove(int id)
{
    if (!storage_.get_optional<Driver>(id))
    {
        return false;
    }
    storage_.remove<Driver>(id);
    return true;
}
