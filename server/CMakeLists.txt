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
        common
        Boost::system
        Boost::thread
)

target_include_directories(messenger_server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/common
)

# Для Windows потрібні додаткові бібліотеки
if(WIN32)
    target_link_libraries(messenger_server PRIVATE ws2_32 wsock32)
endif()