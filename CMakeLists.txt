cmake_minimum_required(VERSION 3.15)
project(MyProject)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/api/include
)

add_subdirectory(src)
add_subdirectory(api)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE core_lib api_handlers)
add_executable(departure_board examples/departure_board.cpp)
target_link_libraries(departure_board PRIVATE core_lib)

include(CTest)
if(BUILD_TESTING)
    add_executable(searchhandler_tests tests/searchhandler_tests.cpp)
    target_link_libraries(searchhandler_tests PRIVATE core_lib api_handlers)
    add_test(NAME searchhandler_tests COMMAND searchhandler_tests)
    add_test(NAME departure_example COMMAND MyProject
        ${CMAKE_SOURCE_DIR}/examples/enschede-departures.json
        board.departures.dortmund.destination)
    set_tests_properties(departure_example PROPERTIES
        PASS_REGULAR_EXPRESSION "Dortmund Hbf")
    add_test(NAME departure_board_example COMMAND departure_board
        ${CMAKE_SOURCE_DIR}/examples/enschede-departures.json)
    set_tests_properties(departure_board_example PROPERTIES
        PASS_REGULAR_EXPRESSION "07:56 +Dortmund Hbf +RB51 +10:06")
endif()
