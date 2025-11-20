cmake_minimum_required(VERSION 3.16)
project(indi_wmh_nema_focuser CXX C)

# INDI version requirement
set(INDI_MIN_VERSION "2.1.6")

# Find required packages
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/")
find_package(INDI ${INDI_MIN_VERSION} REQUIRED)
find_package(Threads REQUIRED)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

# Include directories
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${INDI_INCLUDE_DIR})

# Source files
set(WMH_NEMA_FOCUSER_SOURCES
    wmh_nema_focuser.cpp
)

# Create the driver executable
add_executable(indi_wmh_nema_focuser ${WMH_NEMA_FOCUSER_SOURCES})

# Link libraries
target_link_libraries(indi_wmh_nema_focuser
    ${INDI_LIBRARIES}
    lgpio
    Threads::Threads
)

# Installation
install(TARGETS indi_wmh_nema_focuser
    RUNTIME DESTINATION bin
)

# Install driver XML file
install(FILES indi_wmh_nema_focuser.xml
    DESTINATION ${INDI_DATA_DIR}
)

# Optionally install udev rules
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/99-wmh-nema-focuser.rules")
    install(FILES 99-wmh-nema-focuser.rules
        DESTINATION /etc/udev/rules.d
    )
endif()
