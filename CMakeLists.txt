cmake_minimum_required(VERSION 3.14)

#set(CMAKE_C_STANDARD 98)
#set(CMAKE_C_STANDARD_REQUIRED True)
#set(CMAKE_C_COMPILER "gcc")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_COMPILER "g++")

set(TARGET_KALK kalk)
set(EXECUTABLE_NAME kalk)
set(EXECUTABLE_KALK ${EXECUTABLE_NAME}.out)

project(Kalk VERSION 1.0.0)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wconversion -Wpedantic -fconcepts -Wno-psabi")
elseif(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

add_library(${TARGET_KALK} STATIC)
set_target_properties(${TARGET_KALK} PROPERTIES LINKER_LANGUAGE CXX)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
FIND_PACKAGE(Boost 1.65 REQUIRED COMPONENTS system filesystem program_options date_time)
if(Boost_FOUND)
  target_include_directories(${TARGET_KALK} PUBLIC ${Boost_INCLUDE_DIRS})
  target_link_libraries(${TARGET_KALK} ${Boost_LIBRARIES})
endif()

set(LIBRARY_TEXT text)
set(LIBRARY_MATH math)

# Traverse directories
add_subdirectory(src)
add_subdirectory(ext/lib-text-cpp)
add_subdirectory(ext/lib-math-cpp)

target_link_libraries(${TARGET_KALK} ${LIBRARY_TEXT} ${LIBRARY_MATH} gmp gmpxx mpfr readline)

get_target_property(SOURCES ${TARGET_KALK} SOURCES)
add_executable(${EXECUTABLE_KALK} main.cpp ${SOURCES})
target_compile_definitions(${TARGET_KALK} PUBLIC BOOST_DATE_TIME_POSIX_TIME_STD_CONFIG PROJECT_NAME="${PROJECT_NAME}" PROJECT_VERSION="${PROJECT_VERSION}" PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR} PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR} PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH} PROJECT_EXECUTABLE="${EXECUTABLE_NAME}")
target_include_directories(${TARGET_KALK} PUBLIC src ext/lib-text-cpp/src ext/lib-math-cpp/src)
target_link_libraries(${EXECUTABLE_KALK} ${TARGET_KALK})
