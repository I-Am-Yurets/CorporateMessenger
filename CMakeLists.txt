cmake_minimum_required(VERSION 3.16)
project(CorporateMessenger VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Знайти необхідні пакети
find_package(Boost 1.70 REQUIRED COMPONENTS system thread)
find_package(Qt6 COMPONENTS Core Widgets Network QUIET)

if (NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets Network)
endif()

# Включити автоматичну обробку MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Додати підкаталоги
add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)