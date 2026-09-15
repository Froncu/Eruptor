include(eruptor_module)
eruptor_module(EXECUTABLE)

target_link_libraries("${MODULE_TARGET_NAME}" PRIVATE eru::entry)