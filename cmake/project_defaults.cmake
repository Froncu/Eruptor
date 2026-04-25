function(set_project_defaults TARGET_NAME)
   set_target_properties(${TARGET_NAME} PROPERTIES
      CXX_STANDARD 23
      CXX_STANDARD_REQUIRED TRUE
      CXX_CLANG_TIDY clang-tidy --extra-arg=-fno-ms-extensions --extra-arg=-fno-ms-compatibility)
endfunction()