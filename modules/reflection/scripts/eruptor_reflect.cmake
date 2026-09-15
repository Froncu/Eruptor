function (eruptor_reflect TARGET_NAME)
   if (NOT TARGET "${TARGET_NAME}")
      message(FATAL_ERROR "cannot reflect '${TARGET_NAME}' because it is not a target!")
   endif ()

   get_target_property(ALIASED_TARGET "${TARGET_NAME}" ALIASED_TARGET)
   if (ALIASED_TARGET)
      set(TARGET_NAME "${ALIASED_TARGET}")
   endif ()

   set_property(TARGET "${TARGET_NAME}" PROPERTY EXPORT_COMPILE_COMMANDS ON)
   add_custom_command(TARGET "${TARGET_NAME}" PRE_LINK
      COMMAND "$<TARGET_FILE:${${PROJECT_NAME}_NAMESPACE}::reflection>"
      WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
      COMMENT "Running Eruptor reflection on ${TARGET_NAME}" VERBATIM)
endfunction ()