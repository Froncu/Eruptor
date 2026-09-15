cmake_minimum_required(VERSION 4.4)
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME MODULE_NAME)
project("${MODULE_NAME}" LANGUAGES CXX)

function (eruptor_module MODULE_TYPE)
   set(INCLUDE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include")
   set(SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/source")

   file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
      "${INCLUDE_DIRECTORY}/*.cpp"
      "${SOURCE_DIRECTORY}/*.cpp")

   set(TARGET_NAME "eruptor_${MODULE_NAME}")
   set(ALIAS_NAME "eru::${MODULE_NAME}")

   if (MODULE_TYPE STREQUAL "EXECUTABLE")
      add_executable("${TARGET_NAME}" ${SOURCE_FILES})
      add_executable("${ALIAS_NAME}" ALIAS "${TARGET_NAME}")

      if (WIN32)
         add_custom_command(TARGET "${TARGET_NAME}" POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different -t "$<TARGET_FILE_DIR:${TARGET_NAME}>" "$<TARGET_RUNTIME_DLLS:${TARGET_NAME}>"
            COMMAND_EXPAND_LISTS VERBATIM)
      endif ()
   elseif (MODULE_TYPE STREQUAL "STATIC" OR MODULE_TYPE STREQUAL "SHARED")
      add_library("${TARGET_NAME}" "${MODULE_TYPE}" ${SOURCE_FILES})
      add_library("${ALIAS_NAME}" ALIAS "${TARGET_NAME}")
   else ()
      message(FATAL_ERROR "module type '${MODULE_TYPE}' is not recognized!")
   endif ()
   
   set_target_properties("${TARGET_NAME}" PROPERTIES EXPORT_NAME "${MODULE_NAME}")

   target_include_directories("${TARGET_NAME}"
      PUBLIC "${INCLUDE_DIRECTORY}"
      PRIVATE "${SOURCE_DIRECTORY}")

   set_target_properties("${TARGET_NAME}" PROPERTIES
      CXX_STANDARD 23
      CXX_STANDARD_REQUIRED TRUE)

   target_compile_options("${TARGET_NAME}" PRIVATE
      "$<$<CXX_COMPILER_FRONTEND_VARIANT:GNU>:-Wall;-Wextra;-Wpedantic;-Werror>"
      "$<$<CXX_COMPILER_FRONTEND_VARIANT:MSVC>:/W4;/WX>"
      "$<$<CXX_COMPILER_ID:Clang>:-Wno-braced-scalar-init;-Wno-unused-template;-Wmissing-prototypes>")

   set(MODULE_TARGET_NAME "${TARGET_NAME}" PARENT_SCOPE)
endfunction () 