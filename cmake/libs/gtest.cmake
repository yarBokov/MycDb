set(GTEST_BASE_DIR "${LIBS_DIR}/googletest")
set(GTEST_LIB_HINT "${GTEST_BASE_DIR}/lib64")

set(GTEST_SOURCE_DIR ${GTEST_BASE_DIR}/googletest)
set(GTEST_INCLUDE_DIRS
  ${GTEST_SOURCE_DIR}
  ${GTEST_SOURCE_DIR}/include)

# set(GMOCK_SOURCE_DIR ${GTEST_PATH}/googlemock)
# set(GMOCK_INCLUDE_DIRS
#   ${GMOCK_SOURCE_DIR}
#   ${GMOCK_SOURCE_DIR}/include)

find_library(GTEST_LIBRARY gtest HINTS ${GTEST_LIB_HINT} NO_DEFAULT_PATH DOC "Path to {googletest} core library")
find_library(GTEST_MAIN_LIB gtest_main HINTS ${GTEST_LIB_HINT} NO_DEFAULT_PATH DOC "Path to {googletest} main library")
# find_library(GMOCK_LIBRARY gtest HINTS ${GTEST_LIB_HINT})
# find_library(GMOCK_MAIN_LIB gtest HINTS ${GTEST_LIB_HINT})

add_library(gtest::gtest INTERFACE IMPORTED)
target_include_directories(gtest::gtest
    INTERFACE
    ${GTEST_INCLUDE_DIRS})
#    ${GMOCK_INCLUDE_DIRS})

target_link_libraries(gtest::gtest
    INTERFACE
        ${GTEST_LIBRARY}
        ${GTEST_MAIN_LIB})
        # ${GMOCK_LIBRARY}
        # ${GMOCK_MAIN_LIB})

message(STATUS "Configured {googletest} library: ${GTEST_LIBRARY}")