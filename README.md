# TaxiORM

Облік поїздок таксі (Driver 1:N Trip). Лабораторна робота №8 (ORM, Repository) та
лабораторна робота №10 (поведінкові патерни State, Strategy, Observer, додатковий патерн Facade).

## Збирання і запуск

Потрібні: Visual Studio з C++, CMake, vcpkg (змінна VCPKG_ROOT).

    cmake --preset default
    cmake --build build
    build\TaxiORM.exe          # демонстрація патернів (лаб. 10)
    build\TaxiORM.exe lab8     # демонстрація лабораторної №8

## Модульні тести

    build\taxi_tests.exe
    ctest --test-dir build --output-on-failure
