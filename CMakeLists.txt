cmake_minimum_required(VERSION 3.16)

project(TWR-U-Pu-Cylinder-2D LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Dependencies
find_package(OpenMP REQUIRED)
find_package(HDF5 COMPONENTS CXX REQUIRED)

# 1. Compiler-Specific Flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Aggressive optimization and architecture-specific tuning
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -ffast-math")
    # Wall/Wextra help catch logical errors in your physics formulas
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

# 2. Define the Target
add_executable(${PROJECT_NAME}
    src/main.cpp
    include/geometry/Cell.h
    include/geometry/Cylinder.h
    include/geometry/Grid.h
    include/physics/Neutron.h
    include/physics/Nuclide.h
    include/physics/Precursor.h
    include/containers/Field.h
    include/solver/Simulation.h
    include/physics/Fuel.h
)

# 3. Enable Link-Time Optimization (LTO)
# FIX: Moved this AFTER add_executable so the property can be applied to the target
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT error)
if(ipo_supported)
    set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
    message(STATUS "LTO/IPO is enabled for ${PROJECT_NAME}")
endif()

# 4. Include Paths and Linking
target_include_directories(${PROJECT_NAME}
    PRIVATE include
    PRIVATE ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE OpenMP::OpenMP_CXX
    PRIVATE ${HDF5_CXX_LIBRARIES}
)

# 5. Installation Logic
include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
