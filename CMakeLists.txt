# define the project name
PROJECT(indi_serial_shutter_release C CXX)
cmake_minimum_required(VERSION 3.0)

include(GNUInstallDirs)

# add our cmake modules folder
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake_modules/")

# FIND OUR REQUIRED PACKAGES
find_package(INDI REQUIRED)
find_package(fmt REQUIRED)


# these will be used to set the version number in config.h and our driver's xml file
set(INDI_SERIAL_SHUTTER_RELEASE_VERSION_MAJOR 0)
set(INDI_SERIAL_SHUTTER_RELEASE_VERSION_MINOR 1)

# do the replacements in the config and xml files
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/indi_serial_shutter_release.xml.cmake ${CMAKE_CURRENT_BINARY_DIR}/indi_serial_shutter_release.xml )

# set our include directories
include_directories( ${CMAKE_CURRENT_BINARY_DIR})
include_directories( ${CMAKE_CURRENT_SOURCE_DIR})
include_directories( ${INDI_INCLUDE_DIR})
include_directories( ${EV_INCLUDE_DIR})

include(CMakeCommon)

########### driver ###########
set(indi_serial_shutter_release_SRCS
   ${CMAKE_CURRENT_SOURCE_DIR}/driver.cpp
   )

# tell cmake to build our executable and link it to these libraries
add_executable(indi_serial_shutter_release ${indi_serial_shutter_release_SRCS})
target_link_libraries(indi_serial_shutter_release ${INDI_LIBRARIES} ${GSL_LIBRARIES} ${ZLIB_LIBRARIES} fmt::fmt)

# tell cmake where to install our executable and where to put the driver's xml file
install(TARGETS indi_serial_shutter_release RUNTIME DESTINATION bin )
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/indi_serial_shutter_release.xml DESTINATION ${INDI_DATA_DIR})
