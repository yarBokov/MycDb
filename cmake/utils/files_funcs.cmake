function(get_common_name name common_name)
  set(${common_name} "mycdb-${name}" PARENT_SCOPE)
endfunction()

function(set_output_target target name)
  get_common_name(${name} prop_name)
  
  set_target_properties(${target} PROPERTIES OUTPUT_NAME ${prop_name} LINKER_LANGUAGE CXX)
endfunction()

function(convert_to_target_name string_to_convert out)
  set(out_string "${string_to_convert}")
  string(TOLOWER "${out_string}" out_string)
  string(REGEX REPLACE "[^a-z0-9\\:]" "-" out_string "${out_string}")
  set(${out} "${out_string}" PARENT_SCOPE)
endfunction()