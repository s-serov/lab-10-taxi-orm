#pragma once

#include <functional>

// Виконує групу операцій атомарно: виняток усередині work скасовує всі зміни
class ITransactionRunner
{
public:
    virtual ~ITransactionRunner() = default;

    virtual void run(const std::function<void()>& work) = 0;
};
