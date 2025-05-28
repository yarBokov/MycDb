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