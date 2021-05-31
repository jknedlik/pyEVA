cmake_minimum_required(VERSION 3.15...3.19)

project(pyneva VERSION "0.0.1")
include (FindPackageHandleStandardArgs)

if(SKBUILD)
  # Scikit-Build does not add your site-packages to the search path automatically,
  # so we need to add it _or_ the pybind11 specific directory here.
  execute_process(
    COMMAND
      "${PYTHON_EXECUTABLE}" -c
      "import pybind11; print(pybind11.get_cmake_dir())"
    OUTPUT_VARIABLE _tmp_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE COMMAND_ECHO STDOUT)
  list(APPEND CMAKE_PREFIX_PATH "${_tmp_dir}")
endif()
# set variables to find geneva
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")
add_compile_options(-std=c++17)
message("setting includes to ${GENEVA_INCLUDE_DIR}")
find_package(Geneva)
find_package(Boost COMPONENTS program_options serialization unit_test_framework REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS}) 
# add example to run via geneva
add_subdirectory(examples)
if(PYBUILD)
# Now we can find pybind11
find_package(pybind11 CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS}) 
INCLUDE_DIRECTORIES(${GENEVA_INCLUDE_DIR}) 
message("link against: ${GENEVA_LIBRARIES}")
LINK_LIBRARIES(${GENEVA_LIBS} ${Boost_LIBRARIES})
pybind11_add_module(pyneva MODULE pysrc/main.cpp)

target_compile_definitions(pyneva PRIVATE VERSION_INFO=${PROJECT_VERSION})

install(TARGETS pyneva DESTINATION .)
endif()
