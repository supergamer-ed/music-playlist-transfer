cmake_minimum_required(VERSION 3.6)
project(music-transfer)

set(CMAKE_CXX_STANDARD 11)
set(MY_DIRS ${CMAKE_SOURCE_DIR}/music-playlist-transfer)

add_library(added_libs STATIC spotifyFUNCS.cpp youtubeFUNCS.cpp)
target_include_directories(added_libs PUBLIC ${MY_DIRS}
                                             ${MY_DIRS}/libs
                                             ${MY_DIRS}/libs/nlohmann)

find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    message("Openssl was found")
    include_directories(${OPENSSL_LIBRARIES})
    target_link_libraries(added_libs PUBLIC OpenSSL::Crypto OpenSSL::SSL)
else()
    message(FATAL_ERROR "Openssl is no where to be found")
endif()

find_package(CURL REQUIRED)
if(CURL_FOUND)
    message("CURL found")
    include_directories(${CURL_INCLUDE_DIRS})
    target_link_libraries(added_libs PUBLIC CURL::libcurl)
else()
    message(FATAL_ERROR "CURL not found")
endif()

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} added_libs)