function(set_project_defaults TARGET_NAME)
   set_target_properties(${TARGET_NAME} PROPERTIES
      CXX_STANDARD 23
      CXX_STANDARD_REQUIRED TRUE)

   target_compile_options(${TARGET_NAME}
      PRIVATE $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic -Werror>
      PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>)
endfunction()