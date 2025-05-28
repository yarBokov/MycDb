set(LIBSSH_BASE_DIR "${MYC_EXT_LIBS_ROOT}/libssh" CACHE PATH "libssh installation root")
set(LIBSSH_LIB_HINT "${LIBSSH_BASE_DIR}/lib64" CACHE PATH "Library search hint")
set(LIBSSH_INCLUDE_DIRS
    "${LIBSSH_BASE_DIR}/include"
    "${LIBSSH_LIB_HINT}/include"
    CACHE STRING "Include path hints"
)

find_library(LIBSSH_LIBRARY NAMES ssh HINTS ${LIBSSH_LIB_HINT} 
    NO_DEFAULT_PATH DOC "Path to libssh core library")

if (NOT LIBSSH_LIBRARY)
    message(FATAL_ERROR "libssh library not found in ${LIBSSH_LIB_HINT}")
endif()

add_library(ssh::ssh INTERFACE IMPORTED)

target_include_directories(ssh::ssh INTERFACE ${LIBSSH_INCLUDE_DIRS})
target_link_libraries(ssh::ssh
    INTERFACE
        ${LIBSSH_LIBRARY}
        krb5::krb5
        ssl::ssl
        zlib::zlib
        ${CMAKE_DL_LIBS})

message(STATUS "Configured libssh: ${LIBSSH_LIBRARY}")