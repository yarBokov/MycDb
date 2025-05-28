set(FMT_BASE_DIR "${MYC_EXT_LIBS_ROOT}/fmt" CACHE PATH "Base directory for {fmt} installation")
set(FMT_LIB_HINT "${FMT_BASE_DIR}/lib64" CACHE PATH "Library search path hint")
set(FMT_INCLUDE_DIR "${FMT_BASE_DIR}/include" CACHE PATH "Include path hint")

find_library(FMT_LIBRARY NAMES fmt HINTS ${FMT_LIB_HINT} NO_DEFAULT_PATH DOC "Path to {fmt} core library")

if (NOT FMT_LIBRARY)
    message(FATAL_ERROR "{fmt} library not found in ${FMT_LIB_HINT}")
endif()

add_library(fmt::fmt INTERFACE IMPORTED)

target_include_directories(fmt::fmt INTERFACE ${FMT_INCLUDE_DIR})
target_link_libraries(fmt::fmt INTERFACE ${FMT_LIBRARY})

message(STATUS "Configured {fmt} library: ${FMT_LIBRARY}")