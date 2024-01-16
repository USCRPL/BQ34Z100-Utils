cmake_minimum_required(VERSION 3.19)
cmake_policy(VERSION 3.19)

# Initialize Mbed OS build system. 
# Note: This block must be before the include of app.cmake
set(MBED_APP_JSON_PATH mbed_app.json5)

include(mbed-os/tools/cmake/app.cmake)
add_subdirectory(mbed-os)

project(BQ34Z100G1-Utils)

add_subdirectory(BQ34Z100G1-Driver)
add_subdirectory(src)

mbed_finalize_build()