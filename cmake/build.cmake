macro(initialize_path_system)
    set(BASE_DIR ${CMAKE_SOURCE_DIR})

    if(NOT EXISTS "$ENV{LIBS_DIR}") 
        set(LIBS_DIR "${BASE_DIR}/../aux_libs" CACHE PATH "External dependencies location")
    else()
        set(LIBS_DIR "$ENV{LIBS_DIR}")
    endif()
    
    # Generated files configuration
    set(MYC_GEN_DIR "${CMAKE_BINARY_DIR}/src")
    
    # Output directories configuration
    set(MYC_DIST_DIR "${BASE_DIR}/out")
    set(MYC_PACKAGE_DIR "${MYC_DIST_DIR}/pkg")
    
    # Artifacts distribution
    set(MYC_ARCHIVE_OUTPUT "${MYC_DIST_DIR}/arch")
    set(MYC_LIBRARY_OUTPUT "${MYC_DIST_DIR}/lib")
    set(MYC_RUNTIME_OUTPUT "${MYC_DIST_DIR}/bin")
    set(MYC_TEST_OUTPUT "${MYC_DIST_DIR}/tests")
endmacro()

macro(create_output_directories_configuration)
    # CMake output directories
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${MYC_ARCHIVE_OUTPUT})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${MYC_LIBRARY_OUTPUT})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${MYC_RUNTIME_OUTPUT})
    
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
        -Wno-unused-parameter
        -Wno-reorder
        -Wno-deprecated-copy
        -Wstack-protector
        -Winline
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

set_definitions()

initialize_path_system()

create_output_directories_configuration()

myc_setup_include_paths()

if(NOT EXISTS "${LIBS_DIR}")
    message(WARNING "Third-party libraries directory not found: ${LIBS_DIR}")
endif()