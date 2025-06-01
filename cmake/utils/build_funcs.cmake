# ||>> decode_module_path <<||
# [IN]
#   • MOD_DIR - Корневая директория модуля
#   • [FLAGS]
#     - VERBOSE - Подробный вывод
#     - USE_PARENT - Использовать родительскую директорию (N уровней вверх)
#   • [CONFIG]
#     - PREFIX - Префикс проекта (по умолчанию ${PROJECT_NAME})
#     - SRC_ROOT - Корень исходников (по умолчанию ${PROJECT_SOURCE_DIR}/src)
#   • [OUTPUTS]
#     - LIB_NAME - Имя библиотеки (без префикса)
#     - LIB_PATH - Относительный путь библиотеки
#     - TARGET - Имя цели CMake
#     - ALIAS - Алиас цели
#     - INCLUDE_DIR - Суффикс для include директории
#
# [OUT]
#   Заполняет указанные выходные переменные в родительской области видимости
function(decode_module_path MOD_DIR)
    set(opt_args VERBOSE USE_PARENT)
    set(single_args 
        PREFIX SRC_ROOT 
        LIB_NAME LIB_PATH BASE_LIB_PATH 
        BASE_PREFIX BASE_SUFFIX 
        TARGET ALIAS INCLUDE_DIR)
    cmake_parse_arguments(DMP "${opt_args}" "${single_args}" "" ${ARGN})

    # Конфигурация по умолчанию
    set(src_root "${PROJECT_SOURCE_DIR}/src")
    set(proj_prefix "${PROJECT_NAME}")
    
    # Переопределение конфигурации
    if(DMP_SRC_ROOT)
        set(src_root "${DMP_SRC_ROOT}")
    endif()
    if(DMP_PREFIX)
        set(proj_prefix "${DMP_PREFIX}")
    endif()

    # Обработка родительских директорий
    set(actual_dir "${MOD_DIR}")
    if(DMP_USE_PARENT)
        math(EXPR levels "${DMP_USE_PARENT}+0")  # Преобразование в число
        foreach(i RANGE 1 ${levels})
            get_filename_component(actual_dir "${actual_dir}" DIRECTORY)
        endforeach()
        
        if(DMP_VERBOSE)
            message(STATUS "[DMP] Parent dirs: ${levels} → ${actual_dir}")
        endif()
    endif()

    # Валидация пути
    string(FIND "${actual_dir}" "${src_root}/" root_pos)
    if(root_pos EQUAL -1)
        message(WARNING "[DMP] Invalid path: ${src_root} not in ${actual_dir}")
        return()
    endif()

    # Извлечение компонентов пути
    string(LENGTH "${src_root}/" root_len)
    string(SUBSTRING "${actual_dir}" ${root_len} -1 rel_path)
    
    # Генерация имен
    string(REPLACE "/" "-" lib_name "${rel_path}")
    string(FIND "${lib_name}" "-" first_dash)
    
    set(base_prefix "${lib_name}")
    set(base_suffix "")
    if(NOT first_dash EQUAL -1)
        string(SUBSTRING "${lib_name}" 0 ${first_dash} base_prefix)
        math(EXPR suffix_start "${first_dash}+1")
        string(SUBSTRING "${lib_name}" ${suffix_start} -1 base_suffix)
    endif()

    # Формирование выходных значений
    set(full_target "${proj_prefix}-${lib_name}")
    set(target_alias "${proj_prefix}::${lib_name}")
    set(include_suffix "${proj_prefix}/${rel_path}")

    # Вывод отладочной информации
    if(DMP_VERBOSE)
        message(STATUS "[DMP] Module components:")
        message(STATUS "  • Library: ${lib_name}")
        message(STATUS "  • Target: ${full_target}")
        message(STATUS "  • Alias: ${target_alias}")
        message(STATUS "  • Includes: ${include_suffix}")
    endif()

    # Установка выходных переменных
    foreach(var IN ITEMS 
        LIB_NAME LIB_PATH BASE_LIB_PATH 
        BASE_PREFIX BASE_SUFFIX 
        TARGET ALIAS INCLUDE_DIR)
        if(DMP_${var})
            set(${DMP_${var}} "${${var}}" PARENT_SCOPE)
        endif()
    endforeach()
endfunction()

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