cmake_minimum_required(VERSION 3.16)
project(CorporateMessengerSimple VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# === СЕРВЕР ===
add_executable(server server.cpp)

if(WIN32)
    target_link_libraries(server PRIVATE ws2_32 wsock32)
endif()

# === КЛІЄНТ ===

# Qt (ВАЖЛИВО: порядок має значення!)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Для Windows - вказати шлях до UIC
if(WIN32)
    set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/client)
endif()

find_package(Qt6 COMPONENTS Core Widgets Network QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Widgets Network)
endif()

# Явно вказати всі файли
set(CLIENT_SOURCES
        client/main.cpp
        client/MainWindow.cpp
)

set(CLIENT_HEADERS
        client/MainWindow.h
)

set(CLIENT_UI
        client/MainWindow.ui
)

add_executable(client
        ${CLIENT_SOURCES}
        ${CLIENT_HEADERS}
        ${CLIENT_UI}
)

if(Qt6_FOUND)
    target_link_libraries(client PRIVATE Qt6::Core Qt6::Widgets Qt6::Network)
    set(QT_VERSION_MAJOR 6)
else()
    target_link_libraries(client PRIVATE Qt5::Core Qt5::Widgets Qt5::Network)
    set(QT_VERSION_MAJOR 5)
endif()

# Додати include директорії для згенерованих файлів
target_include_directories(client PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/client_autogen/include
)

# Для Windows GUI (без консолі)
if(WIN32)
    set_target_properties(client PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# === ВИВІД ===
message(STATUS "==================================")
message(STATUS "Corporate Messenger - Simple")
message(STATUS "==================================")
if(Qt6_FOUND)
    message(STATUS "Qt version: ${Qt6_VERSION}")
else()
    message(STATUS "Qt version: ${Qt5_VERSION}")
endif()
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build directory: ${CMAKE_BINARY_DIR}")
message(STATUS "==================================")