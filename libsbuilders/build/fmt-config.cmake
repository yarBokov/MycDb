get_filename_component(_lib_name "${CMAKE_CURRENT_LIST_FILE}" NAME_WE)
string(REGEX REPLACE "(^Find|^find-|-config$|Config$)" "" _lib_name "${_lib_name}")
set(_target_name "${_lib_name}::${_lib_name}")

find_path(${_lib_name}_INCLUDE_DIR "format.h")
find_library(${_lib_name}_LIBRARY NAMES "${_lib_name}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(${_lib_name}
  REQUIRED_VARS ${_lib_name}_LIBRARY ${_lib_name}_INCLUDE_DIR)

if(${_lib_name}_FOUND AND NOT TARGET ${_target_name})
  add_library(${_target_name} UNKNOWN IMPORTED)
  set_target_properties(${_target_name} PROPERTIES
    IMPORTED_LOCATION "${${_lib_name}_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${${_lib_name}_INCLUDE_DIR}")
endif()

mark_as_advanced(${_lib_name}_LIBRARY ${_lib_name}_INCLUDE_DIR)