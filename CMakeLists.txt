cmake_minimum_required(VERSION 3.11)
set(CMAKE_BUILD_DIRECTORY build)

project(httpServer)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
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
        src/server/request.cpp
        src/server/request_handler.hpp
        src/server/request_handler.cpp
        src/server/request_parser.hpp
        src/server/request_parser.cpp
        src/server/server.hpp
        src/server/server.cpp
        src/server/context.hpp
        src/server/context.cpp
        )
target_include_directories(httpServer PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(httpServer ${Boost_LIBRARIES})
target_link_libraries(httpServer pqxx)
target_link_libraries(httpServer pq)

project(init)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/handlers)
add_executable(init
        src/init/main.cpp)
target_link_libraries(init pqxx)
target_link_libraries(init pq)
