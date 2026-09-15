include(eruptor_module)
eruptor_module(STATIC)

target_link_libraries("${MODULE_TARGET_NAME}" PUBLIC eru::core)