cmake_minimum_required(VERSION 3.9)
project(dstar)

include(ExternalProject)

set(CMAKE_CXX_STANDARD 11)

# ARGSX LIBRARY
set(DEP_ARGSX "argsx")
set(LIB_ARGSX "ArgsX.a")
# SPARK LIBRARY
set(DEP_SPARK "spark")
set(LIB_SPARK "Spark.a")

# ARGSX LIBRARY
ExternalProject_Add(${DEP_ARGSX} PREFIX "${PROJECT_SOURCE_DIR}/lib/argsx/" SOURCE_DIR "${PROJECT_SOURCE_DIR}/lib/argsx/" INSTALL_COMMAND "")
ExternalProject_Get_Property(${DEP_ARGSX} source_dir)
include_directories("${source_dir}/include/")
link_directories("${source_dir}/bin/")

# SPARK LIBRARY
ExternalProject_Add(${DEP_SPARK} PREFIX "${PROJECT_SOURCE_DIR}/lib/spark/" SOURCE_DIR "${PROJECT_SOURCE_DIR}/lib/spark/" INSTALL_COMMAND "")
ExternalProject_Get_Property(${DEP_SPARK} source_dir)
include_directories("${source_dir}/include/")
link_directories("${source_dir}/bin/")

include_directories(include)
set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")

add_executable(dstar src/main.cpp src/Core.cpp src/actions/Flood.cpp src/actions/Starvation.cpp src/DhcpPool.cpp include/DhcpPool.h src/DhcpSocket.cpp include/DhcpSocket.h)
add_dependencies(dstar ${DEP_ARGSX})
add_dependencies(dstar ${DEP_SPARK})
target_link_libraries(dstar pthread)
target_link_libraries(dstar rt)
target_link_libraries(dstar ${LIB_ARGSX})
target_link_libraries(dstar ${LIB_SPARK})