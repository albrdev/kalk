cmake_minimum_required(VERSION 3.14)

#set(CMAKE_C_STANDARD 98)
#set(CMAKE_C_STANDARD_REQUIRED True)
#set(CMAKE_C_COMPILER "gcc")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER "g++")

set(TARGET_MAIN object-kalk)
set(EXECUTABLE_MAIN kalk)

project(Kalk VERSION 1.0.0)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -Wpedantic")
elseif(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

add_library(${TARGET_MAIN} STATIC)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
FIND_PACKAGE(Boost 1.71 REQUIRED COMPONENTS system filesystem program_options)
if(Boost_FOUND)
  target_include_directories(${TARGET_MAIN} PUBLIC ${Boost_INCLUDE_DIRS})
  target_link_libraries(${TARGET_MAIN} ${Boost_LIBRARIES})
endif()

set(LIBRARY_COMMON common)
set(LIBRARY_TEXT text)

# Traverse directories
add_subdirectory(src)
add_subdirectory(ext/lib-common-cpp)
add_subdirectory(ext/lib-text-cpp)

get_target_property(SOURCES ${TARGET_MAIN} SOURCES)
add_executable(${EXECUTABLE_MAIN} main.cpp ${SOURCES})
target_compile_definitions(${TARGET_MAIN} PUBLIC PROJECT_NAME="${PROJECT_NAME}" PROJECT_VERSION="${PROJECT_VERSION}" PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR} PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH} PROJECT_EXECUTABLE="${EXECUTABLE_MAIN}")
target_include_directories(${TARGET_MAIN} PUBLIC src ext/lib-common-cpp/src ext/lib-text-cpp/src)
target_link_libraries(${EXECUTABLE_MAIN} ${TARGET_MAIN} ${LIBRARY_COMMON} ${LIBRARY_TEXT} gmpxx gmp mpfr readline)

