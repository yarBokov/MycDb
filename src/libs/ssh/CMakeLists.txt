decode_module_path(
  ${CMAKE_CURRENT_LIST_DIR}
  LIB_NAME      lib_name
  TARGET_NAME   target_name
  TARGET_ALIAS  target_alias)

get_local_sources(sources_list)

add_library(${target_name} STATIC ${sources_list})

if (NOT "${target_alias}" STREQUAL "")
    add_library(${target_alias} ALIAS ${target_name})
endif()

target_include_directories(${target_name} PUBLIC "${PROJECT_SOURCE_DIR}/src")

target_link_libraries(${target_name}
  PRIVATE
    fmt::fmt
  PUBLIC
    ssh::ssh)

set_output_target(${target_name} ${lib_name})