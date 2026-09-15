include(eruptor_module)
eruptor_module(EXECUTABLE)

find_package(LibXml2 CONFIG REQUIRED)
find_package(ZLIB CONFIG REQUIRED)
find_package(zstd CONFIG REQUIRED)
find_package(LLVM CONFIG REQUIRED)
cmake_path(GET LLVM_CMAKE_DIR PARENT_PATH LLVM_CMAKE_PARENT_DIR)
set(Clang_DIR "${LLVM_CMAKE_PARENT_DIR}/clang")
find_package(Clang CONFIG REQUIRED NO_DEFAULT_PATH)

target_include_directories("${MODULE_TARGET_NAME}" SYSTEM
   PRIVATE ${LLVM_INCLUDE_DIRS}
   PRIVATE ${CLANG_INCLUDE_DIRS})

set_target_properties("${MODULE_TARGET_NAME}" PROPERTIES "MSVC_RUNTIME_LIBRARY" "MultiThreaded")

separate_arguments(LLVM_DEFINITIONS NATIVE_COMMAND "${LLVM_DEFINITIONS}")
target_compile_definitions("${MODULE_TARGET_NAME}" PRIVATE ${LLVM_DEFINITIONS})

target_link_libraries("${MODULE_TARGET_NAME}"
   PRIVATE clangTooling
   PRIVATE clangASTMatchers
   PRIVATE clangBasic)