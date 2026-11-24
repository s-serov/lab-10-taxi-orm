#pragma once

#include <functional>
#include "Database.h"
#include "ITransactionRunner.h"

class SQLiteTransactionRunner : public ITransactionRunner
{
public:
    explicit SQLiteTransactionRunner(Storage& storage)
        : storage_(storage)
    {
    }

    void run(const std::function<void()>& work) override
    {
        storage_.transaction([&]()
        {
            work();
            return true;
        });
    }

private:
    Storage& storage_;
};
