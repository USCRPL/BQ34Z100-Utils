set(MAIN_SOURCES
    SOCTestSuite.h
    SOCTestSuite.cpp)

set(CHEMID_MEASURER_SOURCES
	ChemIDMeasurer.cpp
	ChemIDMeasurer.h)

# compile main test code
add_executable(soc-test ${MAIN_SOURCES})
target_include_directories(soc-test PUBLIC .)
target_link_libraries(soc-test BQ34Z100 mbed-os)
mbed_set_post_build(soc-test)

add_executable(chem-id-measurer ${CHEMID_MEASURER_SOURCES})
target_link_libraries(chem-id-measurer BQ34Z100 mbed-os)
mbed_set_post_build(chem-id-measurer)
