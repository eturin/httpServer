cmake_minimum_required(VERSION 3.11)
project(httpServer)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
find_package(Boost REQUIRED COMPONENTS thread)
add_executable(httpServer
        main.cpp
        src/server/connection.cpp
        src/server/connection.hpp
        src/server/header.hpp
        src/server/mime_types.hpp
        src/server/mime_types.cpp
        src/server/reply.hpp
        src/server/reply.cpp
        src/server/request.hpp
        src/server/request_handler.hpp
        src/server/request_handler.cpp
        src/server/request_parser.hpp
        src/server/request_parser.cpp
        src/server/server.hpp
        src/server/server.cpp
)
target_include_directories(httpServer PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(httpServer ${Boost_LIBRARIES})
