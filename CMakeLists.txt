cmake_minimum_required(VERSION 3.14)
########################################## SET  COMPILERS ##########################################
set(CMAKE_C_COMPILER "gcc") # {gcc, clang, mpicc, icc}
set(CMAKE_CXX_COMPILER "g++") # {g++, clang++, mpicxx, icpc}
project(LKMC)
set(CMAKE_CXX_STANDARD 20)
############################################ SET  PATHS ############################################
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
############################################ SET  FLAGS ############################################
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} "
        CACHE INTERNAL "")
#set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} \
-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wpointer-arith -Wold-style-cast \
-Wpedantic  -Wcast-qual -Wunused -Woverloaded-virtual -Winline \
-Wunreachable-code -Wcast-align -Weffc++ -Wmisleading-indentation \
-Wredundant-decls -pedantic-errors -Wnull-dereference -Wdouble-promotion \
-Wconversion -Wfatal-errors -Wextra-semi -O0 -g3 -finline-functions"
        CACHE INTERNAL "")
IF(NOT TEST)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wundef"
           CACHE INTERNAL "")
ENDIF()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall -Wextra"
        CACHE INTERNAL "")

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}"
            CACHE INTERNAL "")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}"
            CACHE INTERNAL "")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}"
            CACHE INTERNAL "")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}"
            CACHE INTERNAL "")
else ()
    message(WARNING "Please check which compiler you are using")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -D_GLIBCXX_PARALLEL"
            CACHE INTERNAL "")
endif ()
message(STATUS "Detecting ${CMAKE_CXX_COMPILER_ID} compiler")
message(STATUS "Current build type is ${CMAKE_BUILD_TYPE}")
############################################ BUILD LIBS ############################################
#add_executable(${PROJECT_NAME} LKMC_Main.cpp)
#add_subdirectory(lkmc/cfg)
#add_subdirectory(lkmc/gen)
#add_subdirectory(lkmc/ansys)
#add_subdirectory(lkmc/kmc)
#target_link_libraries(${PROJECT_NAME} PUBLIC cfg gen ansys kmc)
############################################ BUILD SRCS ############################################
file(GLOB PROJECT_SRC
        ${PROJECT_SOURCE_DIR}/lkmc/cfg/src/*.cpp
        ${PROJECT_SOURCE_DIR}/lkmc/pred/src/*.cpp
        ${PROJECT_SOURCE_DIR}/lkmc/kmc/src/*.cpp
        ${PROJECT_SOURCE_DIR}/lkmc/ansys/src/*.cpp
        ${PROJECT_SOURCE_DIR}/lkmc/api/src/*.cpp
        )
set(PROJECT_INCLUDE
        ${PROJECT_SOURCE_DIR}/lkmc/cfg/include
        ${PROJECT_SOURCE_DIR}/lkmc/pred/include
        ${PROJECT_SOURCE_DIR}/lkmc/kmc/include
        ${PROJECT_SOURCE_DIR}/lkmc/ansys/include
        ${PROJECT_SOURCE_DIR}/lkmc/api/include
        )

add_executable(${PROJECT_NAME} LKMC_Main.cpp ${PROJECT_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_INCLUDE})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "lkmc.exe")
#################################### BOOST #####################################
#set(Boost_NO_SYSTEM_PATHS TRUE)
#if (Boost_NO_SYSTEM_PATHS)
#    set(BOOST_ROOT "/opt/apps/intel18/impi18_0/boost-mpi/1.68")
#    set(BOOST_INCLUDE_DIRS "${BOOST_ROOT}/include")
#    set(BOOST_LIBRARY_DIRS "${BOOST_ROOT}/lib")
#endif (Boost_NO_SYSTEM_PATHS)
# Stampede2 doesn't have boost-mpi. Use self-compiled version
#SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH}
#        "/home1/07075/tg863748/library/boost/include")
#SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH}
#        "/home1/07075/tg863748/library/boost/lib")
#find_package(Boost COMPONENTS mpi serialization REQUIRED)
find_package(Boost REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Boost_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Boost_LIBRARIES})
############################################### MPI ################################################
find_package(MPI REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC MPI::MPI_CXX)
############################################## OPENMP ##############################################
find_package(OpenMP REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC OpenMP::OpenMP_CXX)
############################################### JSON ###############################################
# Typically you don't care so much for a third party library's tests to be
# run from your own project's code.
#set(JSON_BuildTests OFF CACHE INTERNAL "")
# If you only include this third party in PRIVATE source files, you do not
# need to install it when your main project gets installed.
# set(JSON_Install OFF CACHE INTERNAL "")
# add_subdirectory(external/json)
include(FetchContent)
FetchContent_Declare(
        json
        GIT_REPOSITORY
        https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
        GIT_TAG v3.10.5)
FetchContent_GetProperties(json)
if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
############################################ ARMADILLO #############################################
#find_package(ARMADILLO REQUIRED)
#target_include_directories(${PROJECT_NAME} PUBLIC
#        ${ARMADILLO_INCLUDE_DIRS})
#target_link_libraries(${PROJECT_NAME} PUBLIC
#        ${ARMADILLO_LIBRARIES})
############################################## EIGEN ###############################################
#find_package(EIGEN3 REQUIRED)
#target_include_directories(${PROJECT_NAME} PUBLIC
#        ${EIGEN3_INCLUDE_DIRS})
############################################ TENSORFLOW ############################################
#find_package(TensorFlow REQUIRED)
#target_include_directories(${PROJECT_NAME} PUBLIC
#        ${TensorFlow_INCLUDE_DIRS})
#target_link_libraries(${PROJECT_NAME} PUBLIC
#        ${TensorFlow_LIBRARIES})
############################################ GOOGLETEST ############################################
#set(TEST "on")
IF(TEST)
    message(STATUS "TEST mode on")
    include(FetchContent)
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest/
            GIT_TAG release-1.11.0)
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    include(GoogleTest)
#    macro(package_add_test TESTNAME)
#        add_executable(${TESTNAME} ${ARGN})
#        target_link_libraries(${TESTNAME} gtest gmock gtest_main)
#        gtest_discover_tests(${TESTNAME}
#                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
#                PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/test"
#                )
#        set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
#    endmacro()

#    macro(package_add_test_with_libraries TESTNAME FILES LIBRARIES)
#        add_executable(${TESTNAME} ${FILES})
#        target_link_libraries(${TESTNAME} gtest gmock gtest_main ${LIBRARIES})
#        gtest_discover_tests(${TESTNAME}
#                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/test
#                PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/test"
#                )
#        set_target_properties(${TESTNAME} PROPERTIES FOLDER tests)
#    endmacro()
    set(TEST_NAME my_test)
    add_executable(${TEST_NAME} LKMC_Gtest.cpp ${PROJECT_SRC})
    set_target_properties(${TEST_NAME} PROPERTIES
            OUTPUT_NAME "${TEST_NAME}.exe")
    target_include_directories(${TEST_NAME} PUBLIC
            ${PROJECT_INCLUDE}
            ${Boost_INCLUDE_DIR})

    target_link_libraries(${TEST_NAME} gtest gmock gtest_main )
    gtest_discover_tests(${TEST_NAME})
ELSE()
    message(STATUS "TEST mode off")
ENDIF()


