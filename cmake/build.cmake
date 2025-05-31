macro(initialize_path_system)
    set(BASE_DIR ${CMAKE_SOURCE_DIR} PARENT_SCOPE)

    if(DEFINED ENV{LIBS_DIR} AND EXISTS "$ENV{LIBS_DIR}")
        set(MYC_EXT_LIBS_ROOT "$ENV{LIBS_DIR}" CACHE PATH 
            "External dependencies location" FORCE)
    else()
        set(MYC_EXT_LIBS_ROOT "${BASE_DIR}/../aux_libs" CACHE PATH 
            "Default external dependencies location" FORCE)
    endif()
    
    # Generated files configuration
    set(MYC_GEN_DIR "${CMAKE_BINARY_DIR}/src" PARENT_SCOPE)
    
    # Output directories configuration
    set(MYC_DIST_DIR "${BASE_DIR}/out" PARENT_SCOPE)
    set(MYC_PACKAGE_DIR "${MYC_DIST_DIR}/pkg" PARENT_SCOPE)
    
    # Artifacts distribution
    set(MYC_ARCHIVE_OUTPUT "${MYC_DIST_DIR}/arch" PARENT_SCOPE)
    set(MYC_LIBRARY_OUTPUT "${MYC_DIST_DIR}/lib" PARENT_SCOPE)
    set(MYC_RUNTIME_OUTPUT "${MYC_DIST_DIR}/bin" PARENT_SCOPE)
    set(MYC_TEST_OUTPUT "${MYC_DIST_DIR}/tests" PARENT_SCOPE)
endmacro()

macro(create_output_directories_configuration)
    # CMake output directories
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MYC_ARCHIVE_OUTPUT} PARENT_SCOPE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MYC_LIBRARY_OUTPUT} PARENT_SCOPE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MYC_RUNTIME_OUTPUT} PARENT_SCOPE)
    
    # Make these directories
    file(MAKE_DIRECTORY ${MYC_ARCHIVE_OUTPUT})
    file(MAKE_DIRECTORY ${MYC_LIBRARY_OUTPUT})
    file(MAKE_DIRECTORY ${MYC_RUNTIME_OUTPUT})
    file(MAKE_DIRECTORY ${MYC_TEST_OUTPUT})
endmacro()

macro(set_definitions)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_SKIP_RPATH ON)

    # definitions for myc db services
    set(MYC_SERVICE_TIMEOUT 60)
    set(MYC_SERVICE_FILE_LIMIT "8192:262144")

    set(def_warnings
        -Wall
        -Wcast-align
        -Wdouble-promotion
        -Wextra
        -Wformat
        -Wformat-security
        -Wextra-semi
        -Wlogical-op
        -Wmisleading-indentation
        -Wnon-virtual-dtor
        -Wuseless-cast
        -Wconversion
        -Wunused
        -Wstack-protector
        -Winline
        -Wshadow
        -Wfloat-equal
        -Wold-style-cast)

    set(def_errors -Werror=return-type)
    set(def_no_errors -Wno-error=missing-declarations)

    set(def_flags
        -fexceptions
        -fpie)

    set(def_security
        -Werror=format-security
        -fPIC
        -fstack-clash-protection
        -fstack-protector-strong
        -ftrapv
        -pie)

    set(all_defs
        ${def_flags}
        ${def_warnings}
        ${def_errors}
        ${def_no_errors}
        ${def_security})

    set(cxx_debug_flags ${all_defs} -O0 -g3 -ggdb)

    str_weave(CMAKE_CXX_FLAGS_DEBUG ${cxx_debug_flags})
    str_weave(CMAKE_CXX_FLAGS_RELWITHDEBINFO ${all_defs})

    exec_program(uname ARGS -r OUTPUT_VARIABLE CMAKE_SYSTEM_VERSION)

    find_program(CCACHE "ccache")
    if(CCACHE)
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
    endif(CCACHE)

endmacro()

macro(myc_setup_include_paths)
    include_directories(
        ${MYC_GEN_DIR}
    )
endmacro()

initialize_path_system()

create_output_directories_configuration()

set_definitions()

myc_setup_include_paths()

if(NOT EXISTS "${MYC_EXT_LIBS_ROOT}")
    message(WARNING "Third-party libraries directory not found: ${MYC_EXT_LIBS_ROOT}")
endif()