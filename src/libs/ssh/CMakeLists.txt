# decode_module_path(
#   ${CMAKE_CURRENT_LIST_DIR}
#   LIB_NAME      lib_name
#   BASE_PREFIX   base_prefix
#   TARGET_NAME   target_name
#   TARGET_ALIAS  target_alias)

set(sources_list
    exceptions.h
    sessions.h
    ssh_client.h)

add_lib(${sources_list} ${target_name} ${target_alias})

target_link_libraries(${target_name}
  PRIVATE
    fmt::fmt
  PUBLIC
    ssh::ssh)

set_output_target(${target_name} ${lib_name})
