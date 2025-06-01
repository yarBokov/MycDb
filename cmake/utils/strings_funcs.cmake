# ||>> str_chain <<||
#  IN:
#    ‣ target   — Variable to append strings to.
#    ‣ glue     — Connector between fragments (default: "").
#    ‣ fragments— Data shards to merge (variadic).
#
#  OUT:
#    ‣ Modifies `target` in parent scope.
#
#  EXAMPLE:
#    set(data "") 
#    str_chain(data "::" "A" "B")  # → "A::B"
#
function(str_chain target glue)
    set(build "${${target}}")
    set(link "${glue}")
    if("${build}" STREQUAL "")  # First fragment? No glue.
        set(link "")
    endif()

    set(new_data "")
    string(JOIN "${glue}" new_data ${ARGN})  # Fuse fragments
    if(NOT "${new_data}" STREQUAL "")        # Skip empty
        string(APPEND build "${link}${new_data}")
        set(${target} "${build}" PARENT_SCOPE)  # Upload
    endif()
endfunction()

# ||>> str_weave <<||
#  IN:
#    ‣ target   — Variable to inject into.
#    ‣ fragments— Data shards (auto-spaced).
#
#  NOTE: Uses " " as immutable glue (hardcoded).
#
function(str_weave target)
    str_chain("${target}" " " ${ARGN})  # Delegate with space
    set(${target} "${${target}}" PARENT_SCOPE)  # Sync
endfunction()