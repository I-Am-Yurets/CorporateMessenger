# 1. Налаштування vcpkg (має бути на самому початку)
set(VCPKG_APPLOCAL_DEPS ON)
cmake_minimum_required(VERSION 3.20)
project(corporate_messenger)

# Щоб прибрати попередження про FindBoost, яке ми бачили в логах
cmake_policy(SET CMP0167 NEW)

# 2. Обов'язково вмикаємо інструменти Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 3. Шукаємо бібліотеки
find_package(Qt6 COMPONENTS Core Widgets Network REQUIRED) # Ви вже знаєте, що у вас Qt 6.9.3
find_package(Boost REQUIRED COMPONENTS system thread)      # Версія 1.90.0 у вас вже встановлена

# 4. Створення виконуваного файлу
add_executable(messenger_client
        main.cpp
        MainWindow.h MainWindow.cpp MainWindow.ui
        LoginDialog.h LoginDialog.cpp LoginDialog.ui
        RegisterDialog.h RegisterDialog.cpp RegisterDialog.ui
        NetworkClient.h NetworkClient.cpp
)

# 5. Підключення бібліотек
target_link_libraries(messenger_client PRIVATE
        common
        Qt6::Core Qt6::Widgets Qt6::Network
        Boost::system Boost::thread
)

# 6. Шляхи до заголовків
target_include_directories(messenger_client PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/common
        ${CMAKE_CURRENT_BINARY_DIR}
)

# 7. Специфічні налаштування для Windows
if(WIN32)
    # Потрібно для роботи Boost.Asio
    target_compile_definitions(messenger_client PRIVATE _WIN32_WINNT=0x0A00)
    # Лінкування системної бібліотеки сокетів
    target_link_libraries(messenger_client PRIVATE ws2_32)
endif()