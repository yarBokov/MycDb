# ||>> decode_module_path <<||
# [IN]
#   • MOD_DIR - Корневая директория модуля
#   • [FLAGS]
#     - VERBOSE - Подробный вывод
#   • [CONFIG]
#     - PREFIX - Префикс проекта (по умолчанию ${PROJECT_NAME})
#   • [OUTPUTS]
#     - LIB_NAME - Имя библиотеки (без префикса)
#     - TARGET_NAME - Имя цели CMake
#     - TARGET_ALIAS - Алиас цели
#
# [OUT]
#   Заполняет указанные выходные переменные в родительской области видимости
function(decode_module_path MOD_DIR)
    set(verbose_message_prefix "[DECODE_MODULE_PATH]: ")

    set(opt_args VERBOSE USE_PARENT)
    set(single_args  
        LIB_NAME REL_PATH BASE_REL_PATH 
        BASE_PREFIX
        PROJ_PREFIX SRC_ROOT
        TARGET_NAME TARGET_ALIAS)

    cmake_parse_arguments(DMP "${opt_args}" "${single_args}" "" ${ARGN})

    # Конфигурация по умолчанию
    set(src_root "${PROJECT_SOURCE_DIR}/src")
    set(proj_prefix "${PROJECT_NAME}")
    
    # Переопределение конфигурации
    if(DMP_SRC_ROOT)
        set(src_root "${DMP_SRC_ROOT}")
    endif()
    if(DMP_PROJ_PREFIX)
        set(proj_prefix "${DMP_PREFIX}")
    endif()

    # Валидация пути
    string(FIND "${MOD_DIR}" "${src_root}/" root_pos)
    if(root_pos EQUAL -1)
        message(WARNING "${verbose_message_prefix} Invalid path: ${src_root} not in ${MOD_DIR}")
        return()
    endif()

    set(mod_prefix "${proj_prefix}")
    convert_to_target_name("${mod_prefix}" MOD_PREFIX)

    # Извлечение компонентов пути
    string(LENGTH "${src_root}/" root_len)
    string(SUBSTRING "${MOD_DIR}" ${root_len} -1 dir_suffix)
    string(PREPEND dir_suffix "${mod_prefix}/")

    string(FIND "${dir_suffix}" "/" prefix_index)
    string(SUBSTRING "${dir_suffix}" ${prefix_index} -1 rel_path)
    string(SUBSTRING "${rel_path}" 1 -1 rel_path)
    string(FIND "${rel_path}" "/" suffix_index REVERSE)
    string(SUBSTRING "${rel_path}" 0 ${suffix_index} base_rel_path)
    convert_to_target_name("${rel_path}" lib_name)

    string(FIND "${lib_name}" "-" base_prefix_index)
    string(SUBSTRING "${lib_name}" 0 base_prefix_index base_prefix)

    set(target_name "${mod_prefix}-${lib_name}")
    set(target_alias "${mod_prefix}::${lib_name}")

    # Вывод отладочной информации
    if(DMP_VERBOSE)
        message(STATUS "${verbose_message_prefix} src: ${src_root}")
        message(STATUS "${verbose_message_prefix} module: ${MOD_DIR}")
        message(STATUS "${verbose_message_prefix} prefix: ${mod_prefix}")
        message(STATUS "${verbose_message_prefix} Lib Name: ${lib_name}")
        message(STATUS "${verbose_message_prefix} Relative Path: ${rel_path}")
        message(STATUS "${verbose_message_prefix} Base Relative Path: ${base_rel_path}")
        message(STATUS "${verbose_message_prefix} prefix: ${base_prefix}")
        message(STATUS "${verbose_message_prefix} Target Name: ${target_name}")
        message(STATUS "${verbose_message_prefix} Target Alias: ${target_alias}")
    endif()

    if (DEFINED DMP_LIB_NAME)
        set(${DMP_LIB_NAME} "${lib_name}" PARENT_SCOPE)
    endif()
    
    if (DEFINED DMP_TARGET_NAME)
        set(${DMP_TARGET_NAME} "${target_name}" PARENT_SCOPE)
    endif()
    
    if (DEFINED DMP_TARGET_ALIAS)
        set(${DMP_TARGET_ALIAS} "${target_alias}" PARENT_SCOPE)
    endif()

    if (DEFINED DMP_BASE_PREFIX)
        set(${DMP_BASE_PREFIX} "${base_prefix}" PARENT_SCOPE)
    endif()
    
    if (DEFINED DMP_REL_PATH)
        set(${DMP_REL_PATH} "${rel_path}" PARENT_SCOPE)
    endif()
    
    if (DEFINED DMP_BASE_REL_PATH)
        set(${DMP_BASE_REL_PATH} "${base_rel_path}" PARENT_SCOPE)
    endif()

endfunction()

function(get_local_sources OUT_VAR)
    set(options ALL SOURCES HEADERS)
    cmake_parse_arguments(ARG "${options}" "" "" ${ARGN})
    
    if(NOT ARG_SOURCES AND NOT ARG_HEADERS)
        set(ARG_ALL TRUE)
    endif()

    set(BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

    set(RESULT_FILES)
    
    if(ARG_ALL OR ARG_SOURCES)
        file(GLOB_RECURSE CPP_FILES "${BASE_DIR}/*.cpp")
        list(FILTER CPP_FILES EXCLUDE REGEX "/tests")
        list(APPEND RESULT_FILES ${CPP_FILES})
    endif()

    if(ARG_ALL OR ARG_HEADERS)
        file(GLOB_RECURSE H_FILES
            "${BASE_DIR}/*.h"
            "${BASE_DIR}/*.hpp"
        )
        list(FILTER H_FILES EXCLUDE REGEX "/tests")
        list(APPEND RESULT_FILES ${H_FILES})
    endif()

    set(${OUT_VAR} ${RESULT_FILES} PARENT_SCOPE)
endfunction()