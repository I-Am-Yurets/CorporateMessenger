find_package(Qt6 REQUIRED COMPONENTS Core Network)

set(CMAKE_AUTOMOC ON)

add_executable(messenger_server
        main.cpp
        Server.h
        Server.cpp
        ClientSession.h
        ClientSession.cpp
        Database.h
        Database.cpp
)

target_link_libraries(messenger_server PRIVATE
        Qt6::Core
        Qt6::Network
)

target_include_directories(messenger_server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)