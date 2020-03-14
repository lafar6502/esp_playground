set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")

idf_component_register(SRCS "app_main.c" "temp_sensors.c" "test_task.c" "test_rf24.cpp"
                    INCLUDE_DIRS "")
