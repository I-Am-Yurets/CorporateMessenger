cmake_minimum_required(VERSION 3.20)
project(corporate_messenger)

# 1. Обов'язково вмикаємо інструменти Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 2. Шукаємо Qt (спочатку 6, якщо ні - 5)
find_package(Qt6 COMPONENTS Core Widgets Network QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Widgets Network REQUIRED)
endif()

# 3. Шукаємо Boost
find_package(Boost REQUIRED COMPONENTS system thread)

# 4. Створення виконуваного файлу
add_executable(messenger_client
        main.cpp
        MainWindow.h MainWindow.cpp MainWindow.ui
        LoginDialog.h LoginDialog.ui
        RegisterDialog.h RegisterDialog.cpp RegisterDialog.ui
        NetworkClient.h NetworkClient.cpp
        LoginDialog.cpp
)

# 5. Підключення бібліотек
if (Qt6_FOUND)
    target_link_libraries(messenger_client PRIVATE
            common
            Qt6::Core Qt6::Widgets Qt6::Network
            Boost::system Boost::thread
    )
else()
    target_link_libraries(messenger_client PRIVATE
            common
            Qt5::Core Qt5::Widgets Qt5::Network
            Boost::system Boost::thread
    )
endif()

# 6. Шляхи до заголовків
target_include_directories(messenger_client PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/common
        ${CMAKE_CURRENT_BINARY_DIR}
        # Шлях до автогенерованих файлів (залежить від імені target)
        ${CMAKE_CURRENT_BINARY_DIR}/messenger_client_autogen/include
)

if(WIN32)
    target_link_libraries(messenger_client PRIVATE ws2_32)
endif()
