# set(KRB5_PATH ${LIBS_DIR}/krb5)
# set(KRB5_INCLUDE_DIRS
#     ${KRB5_PATH}/src/include
#     ${KRB5_PATH}/src/lib
#     ${KRB5_PATH}/lib64/include)
# set(KRB5_LIBRARY_DIR ${KRB5_PATH}/lib64)

# find_library(KRB5_LIBRARY krb5 HINTS ${KRB5_LIBRARY_DIR})
# find_library(KRB5_KDB5_LIBRARY kdb5 HINTS ${KRB5_LIBRARY_DIR})
# find_library(KRB5_DB2_LIBRARY krb5_db2 HINTS ${KRB5_LIBRARY_DIR})
# find_library(KRB5_GSSAPI_LIBRARY gssapi_krb5 HINTS ${KRB5_LIBRARY_DIR})
# find_library(KRB5_KADM5SRV_MIT_LIBRARY kadm5srv_mit HINTS ${KRB5_LIBRARY_DIR})
# find_library(KRB5_GSSRPC_LIBRARY gssrpc HINTS ${KRB5_LIBRARY_DIR})
# find_library(KRB5_COM_ERR_LIBRARY com_err HINTS ${KRB5_LIBRARY_DIR})
# find_library(KRB5_K5CRYPTO_LIBRARY k5crypto HINTS ${KRB5_LIBRARY_DIR})
# find_library(KRB5_SUPPORT_LIBRARY krb5support HINTS ${KRB5_LIBRARY_DIR})

# add_library(krb5::krb5 INTERFACE IMPORTED)
# target_include_directories(krb5::krb5 INTERFACE ${KRB5_INCLUDE_DIRS})
# target_link_libraries(krb5::krb5
#   INTERFACE
#     -Wl,--start-group
#     ${KRB5_LIBRARY}
#     ${KRB5_KDB5_LIBRARY}
#     ${KRB5_DB2_LIBRARY}
#     ${KRB5_GSSAPI_LIBRARY}
#     ${KRB5_KADM5SRV_MIT_LIBRARY}
#     ${KRB5_GSSRPC_LIBRARY}
#     ${KRB5_COM_ERR_LIBRARY}
#     ${KRB5_K5CRYPTO_LIBRARY}
#     ${KRB5_SUPPORT_LIBRARY}
#     resolv
#     -Wl,--end-group)

set(KRB5_BASE_DIR ${LIBS_DIR}/krb5)
set(KBR5_LIB_HINT ${KRB5_BASE_DIR}/lib64)
set(KRB5_INCLUDE_DIRS
  ${KRB5_BASE_DIR}/src/include
  ${KRB5_BASE_DIR}/src/lib
  ${KRB5_BASE_DIR}/lib64/include)

find_library(KRB5_LIBRARIES NAMES 
    krb5 kdb5 krb5_db2 gssapi_krb5 kadm5srv_mit krb5support gssrpc HINTS ${KBR5_LIB_HINT})

if (NOT KRB5_LIBRARIES)
    message(FATAL_ERROR "Kerberos libraries not found in ${KBR5_LIB_HINT}")
endif()

add_library(krb5::krb5 INTERFACE IMPORTED)

target_include_directories(krb5::krb5 INTERFACE ${KRB5_INCLUDE_DIRS})
target_link_libraries(krb5::krb5 INTERFACE KRB5_LIBRARIES)

message(STATUS "Configured Kerberos MIT V5: ${KRB5_LIBRARIES}")