list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/scripts")
include(eruptor_module)

block (PROPAGATE CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_RC_COMPILER)
   set(LLVM_REQUIRED_VERSION 23.1.0)

   if ("$ENV{LLVM_ROOT}" STREQUAL "")
      message(FATAL_ERROR "LLVM_ROOT is not set. LLVM ${LLVM_REQUIRED_VERSION} is required!")
   endif ()

   cmake_path(CONVERT "$ENV{LLVM_ROOT}" TO_CMAKE_PATH_LIST LLVM_ROOT NORMALIZE)

   if (NOT IS_DIRECTORY "${LLVM_ROOT}")
      message(FATAL_ERROR "LLVM_ROOT is set to '${LLVM_ROOT}', but that directory does not exist!")
   endif ()

   set(LLVM_BIN "${LLVM_ROOT}/bin")

   if (CMAKE_HOST_WIN32)
      set(CMAKE_C_COMPILER "${LLVM_BIN}/clang-cl.exe")
      set(CMAKE_CXX_COMPILER "${LLVM_BIN}/clang-cl.exe")
      set(LLVM_CONFIG "${LLVM_BIN}/llvm-config.exe")

      if (NOT DEFINED VCPKG_TARGET_ARCHITECTURE OR NOT DEFINED VCPKG_CRT_LINKAGE)
         set(CMAKE_RC_COMPILER "${LLVM_BIN}/llvm-rc.exe")
      endif ()
   else ()
      set(CMAKE_C_COMPILER "${LLVM_BIN}/clang")
      set(CMAKE_CXX_COMPILER "${LLVM_BIN}/clang++")
      set(LLVM_CONFIG "${LLVM_BIN}/llvm-config")
   endif ()

   execute_process(COMMAND "${LLVM_CONFIG}" --version OUTPUT_VARIABLE LLVM_INSTALLED_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
   if (NOT LLVM_INSTALLED_VERSION STREQUAL LLVM_REQUIRED_VERSION)
      message(FATAL_ERROR "LLVM ${LLVM_REQUIRED_VERSION} is required, but ${LLVM_INSTALLED_VERSION} was found at ${LLVM_ROOT}!")
   endif ()

   get_property(IN_TRY_COMPILE GLOBAL PROPERTY IN_TRY_COMPILE)
   if (NOT IN_TRY_COMPILE)
      file(GLOB MODULE_MANIFESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/../modules/*/vcpkg.json")
      set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${MODULE_MANIFESTS})

      set(DEPENDENCIES)
      foreach (MODULE_MANIFEST IN LISTS MODULE_MANIFESTS)
         file(READ "${MODULE_MANIFEST}" MANIFEST)
         string(JSON MODULE_DEPENDENCIES ERROR_VARIABLE NO_DEPENDENCIES GET "${MANIFEST}" dependencies)
         string(REGEX REPLACE "^\\[(.*)\\]$" "\\1" MODULE_DEPENDENCIES "${MODULE_DEPENDENCIES}")
         string(STRIP "${MODULE_DEPENDENCIES}" MODULE_DEPENDENCIES)
         if (NOT NO_DEPENDENCIES AND NOT MODULE_DEPENDENCIES STREQUAL "")
            list(APPEND DEPENDENCIES "${MODULE_DEPENDENCIES}")
         endif ()
      endforeach ()

      list(JOIN DEPENDENCIES "," DEPENDENCIES)
      file(CONFIGURE OUTPUT "${VCPKG_MANIFEST_DIR}/vcpkg.json" CONTENT "{ \"dependencies\": [${DEPENDENCIES}] }\n" @ONLY)
   endif ()
endblock ()