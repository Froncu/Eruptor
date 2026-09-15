block (PROPAGATE CMAKE_C_COMPILER CMAKE_CXX_COMPILER CMAKE_RC_COMPILER VCPKG_MANIFEST_DIR)
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
      file(GLOB MODULE_MANIFESTS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/modules/*/vcpkg.json")

      set(DEPENDENCIES "[]")
      foreach (MODULE_MANIFEST IN LISTS MODULE_MANIFESTS)
         set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${MODULE_MANIFEST}")
         file(READ "${MODULE_MANIFEST}" MANIFEST)

         string(JSON MEMBER_COUNT LENGTH "${MANIFEST}")
         if (MEMBER_COUNT GREATER 0)
            math(EXPR LAST_MEMBER "${MEMBER_COUNT} - 1")
            foreach (INDEX RANGE "${LAST_MEMBER}")
               string(JSON MEMBER MEMBER "${MANIFEST}" "${INDEX}")
               if (NOT MEMBER STREQUAL "dependencies" AND NOT MEMBER MATCHES "^\\$")
                  message(FATAL_ERROR "${MODULE_MANIFEST}: '${MEMBER}' is not supported in a module manifest, only 'dependencies' is!")
               endif ()
            endforeach ()
         endif ()

         string(JSON MODULE_DEPENDENCIES ERROR_VARIABLE NO_DEPENDENCIES GET "${MANIFEST}" dependencies)
         if (NO_DEPENDENCIES)
            continue()
         endif ()

         string(JSON DEPENDENCY_COUNT LENGTH "${MODULE_DEPENDENCIES}")
         if (DEPENDENCY_COUNT EQUAL 0)
            continue()
         endif ()

         math(EXPR LAST_DEPENDENCY "${DEPENDENCY_COUNT} - 1")
         foreach (INDEX RANGE "${LAST_DEPENDENCY}")
            string(JSON DEPENDENCY GET "${MODULE_DEPENDENCIES}" "${INDEX}")
            string(JSON DEPENDENCY_TYPE TYPE "${MODULE_DEPENDENCIES}" "${INDEX}")
            if (DEPENDENCY_TYPE STREQUAL "STRING")
               set(DEPENDENCY "\"${DEPENDENCY}\"")
            endif ()

            string(JSON APPEND_INDEX LENGTH "${DEPENDENCIES}")
            string(JSON DEPENDENCIES SET "${DEPENDENCIES}" "${APPEND_INDEX}" "${DEPENDENCY}")
         endforeach ()
      endforeach ()

      set(VCPKG_MANIFEST_DIR "${CMAKE_BINARY_DIR}/vcpkg-manifest")
      string(JSON MANIFEST SET "{}" dependencies "${DEPENDENCIES}")

      file(WRITE "${VCPKG_MANIFEST_DIR}/vcpkg.json.tmp" "${MANIFEST}\n")
      file(COPY_FILE "${VCPKG_MANIFEST_DIR}/vcpkg.json.tmp" "${VCPKG_MANIFEST_DIR}/vcpkg.json" ONLY_IF_DIFFERENT)
      file(REMOVE "${VCPKG_MANIFEST_DIR}/vcpkg.json.tmp")
   endif ()
endblock ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/scripts")
include("${CMAKE_CURRENT_LIST_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")