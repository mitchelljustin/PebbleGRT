cmake_minimum_required(VERSION 2.8.4)
project(PebbleGRT)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    src/js/lib.js
    src/js/api.js
    src/js/stops.js
    src/js/main.js
    src/main.c
    src/nearby_buses.c
    src/nearby_stops.c
    src/bus_detail.c
    src/route_picker.c
    src/stop_info.c
    appinfo.json
    )

add_executable(PebbleGRT ${SOURCE_FILES})