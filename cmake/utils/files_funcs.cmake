function(set_output_target target name)
  set(prop_name "mycdb-${name}" PARENT_SCOPE)

  set_target_properties(${target} PROPERTIES OUTPUT_NAME ${prop_name})

  #strip_file(${target})
endfunction()