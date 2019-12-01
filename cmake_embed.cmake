include(ExternalProject)
ExternalProject_Add(embedder-proj
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/embedder
    INSTALL_COMMAND ""
    BUILD_ALWAYS TRUE
)
ExternalProject_Get_property(embedder-proj BINARY_DIR)
add_executable(embedder IMPORTED)
set_target_properties(embedder PROPERTIES IMPORTED_LOCATION ${BINARY_DIR}/embedder)
add_dependencies(embedder embedder-proj)

# TODO: Make multi file version of this
function(embed_file target filename)
    set(flags)
    set(onevaropts "LIBNAME" "SYMNAME")
    set(multivaropts)
    cmake_parse_arguments(EMBF "${flags}" "${onevaropts}" "${multivaropts}" ${ARGN})
    if(EMBF_LIBNAME)
        set(LIBNAME ${EMBF_LIBNAME})
    else()
        get_filename_component(LIBNAME ${filename} NAME_WLE)
        string(MAKE_C_IDENTIFIER ${LIBNAME} LIBNAME)
    endif()
    if(EMBF_SYMNAME)
        set(SYMNAME ${EMBF_SYMNAME})
    else()
        get_filename_component(SYMNAME ${filename} NAME) 
        string(MAKE_C_IDENTIFIER ${SYMNAME} SYMNAME)
    endif()

    get_filename_component(src_path ${filename} ABSOLUTE)

    set(work_DIR ${CMAKE_CURRENT_BINARY_DIR}/embed/${LIBNAME})
    make_directory(${work_DIR})
    add_custom_command(
        OUTPUT ${work_DIR}/${LIBNAME}.c ${work_DIR}/${LIBNAME}.h
        COMMAND embedder ${LIBNAME} ${src_path} ${SYMNAME}
        WORKING_DIRECTORY ${work_DIR}
        DEPENDS ${src_path} embedder

    )
    add_library(${LIBNAME} STATIC ${work_DIR}/${LIBNAME}.c)
    target_include_directories(${LIBNAME} PUBLIC ${work_DIR})
    target_link_libraries(${target} PUBLIC ${LIBNAME})    


endfunction()
