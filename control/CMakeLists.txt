cmake_minimum_required(VERSION 3.10)
project("MainControl" LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Locate GTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Find and include bson library
find_package(PkgConfig REQUIRED)
pkg_check_modules(BSON REQUIRED libbson-1.0)
include_directories(${BSON_INCLUDE_DIRS})

# Collect source files
file(GLOB SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_SOURCE_DIR}/src/main.cpp")

# Path to local communication directory

file(GLOB SOURCES_COMMUNICATION "../communication/src/*.cpp")
file(GLOB HEADERS_COMMUNICATION "../communication/include/*.h")
file(GLOB SOCKETS_COMMUNICATIONS "../communication/sockets/*.*" )

file(GLOB PARSER "../parser_json/src/*.*")

file(GLOB LOGGER "../logger/*.*")

# Main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${PARSER} ${SOURCES_COMMUNICATION} ${HEADERS_COMMUNICATION} ${SOCKETS_COMMUNICATIONS} ${LOGGER}   src/main.cpp )
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} PRIVATE ${BSON_LIBRARIES} )

# Test executable, including additional source files
file(GLOB TEST_SOURCES "test/*.cpp")
add_executable(RunTests ${SOURCES} ${PARSER} ${SOURCES_COMMUNICATION} ${HEADERS_COMMUNICATION} ${SOCKETS_COMMUNICATIONS} ${LOGGER} ${TEST_SOURCES}  )
target_include_directories(RunTests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(RunTests PRIVATE ${BSON_LIBRARIES} ${GTEST_LIBRARIES} pthread)


include(../hsm-client/use_hsm_client.cmake)
find_library(HSM_CLIENT_LIB hsm_client_lib PATHS ${HSM_CLIENT_LIB_PATH})
if(HSM_CLIENT_LIB)
    message(STATUS "Found hsm_client_lib: ${HSM_CLIENT_LIB}")
    target_link_libraries(${PROJECT_NAME} PRIVATE ${HSM_CLIENT_LIB} ${HSM_CLIENT_LIB_DEPS})
    target_link_libraries(RunTests PRIVATE ${HSM_CLIENT_LIB} ${HSM_CLIENT_LIB_DEPS})
    target_include_directories(${PROJECT_NAME} PRIVATE ${HSM_CLIENT_INCLUDE_DIRS})
    target_include_directories(RunTests PRIVATE ${HSM_CLIENT_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Could not find hsm_client_lib at ${HSM_CLIENT_LIB_PATH}")
endif()