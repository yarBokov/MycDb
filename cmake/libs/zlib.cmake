set(ZLIB_BASE_DIR "${MYC_EXT_LIBS_ROOT}/zlib" CACHE PATH "Base directory for zlib installation")
set(ZLIB_LIB_HINT "${ZLIB_BASE_DIR}/lib64" CACHE PATH "Library search path hint")
set(ZLIB_INCLUDE_DIR "${ZLIB_BASE_DIR}" CACHE PATH "Include path hint")

find_library(ZLIB_LIBRARY NAMES z HINTS ${ZLIB_LIB_HINT} NO_DEFAULT_PATH DOC "Path to zlib core library")

if (NOT ZLIB_LIBRARY)
    message(FATAL_ERROR "zlib library not found in ${ZLIB_LIB_HINT}")
endif()

add_library(zlib::zlib INTERFACE IMPORTED)

target_include_directories(zlib::zlib INTERFACE ${ZLIB_INCLUDE_DIR})
target_link_libraries(zlib::zlib INTERFACE ${ZLIB_LIBRARY})

message(STATUS "Configured zlib library: ${ZLIB_LIBRARY}")