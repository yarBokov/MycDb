set(SSL_BASE_DIR "${LIBS_DIR}/openssl")
set(SSL_LIB_HINT "${SSL_BASE_DIR}/lib64")
set(SSL_INCLUDE_DIR "${SSL_BASE_DIR}/lib64/include")

find_library(SSL_LIBRARIES NAMES ssl crypto ${SSL_LIB_HINT})

if (NOT SSL_LIBRARIES)
    message(FATAL_ERROR "openssl library not found in ${FMT_LIB_HINT}")
endif()

add_library(ssl::ssl INTERFACE IMPORTED)

target_include_directories(ssl::ssl INTERFACE ${SSL_INCLUDE_DIR})
target_link_libraries(ssl::ssl INTERFACE ${SSL_LIBRARIES} ${CMAKE_DL_LIBS})

message(STATUS "Configured ssl and ssl-crypto libraries: ${SSL_LIBRARIES}")