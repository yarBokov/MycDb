function(add_lib sources target_name target_alias)
    if(STATIC)
        set(lib_type STATIC)
    else()
        set(lib_type SHARED)
    endif()

    add_library(${target_name} ${lib_type} ${sources})

    if (NOT "${target_alias}" STREQUAL "")
        add_library(${target_alias} ALIAS ${target_name})
    endif()

    target_include_directories(${target_name} PUBLIC "${PROJECT_SOURCE_DIR}/src")
endfunction()