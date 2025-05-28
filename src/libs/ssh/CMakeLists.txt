

if(STATIC)
    SET(type STATIC)
else()
    set(type SHARED)
endif()

set(source_code exceptions.h sessions.h ssh_client.h)

add_library(${} ${type} ${source_code})

target_include_directories(  PUBLIC ${PROJECT_SOURCE_DIR}/src)

target_link_directories(${} 
                        PRIVATE fmt::fmt
                        PUBLIC ssh::ssh)



