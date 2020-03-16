set(COMPONENT_SRCDIRS ".")
set(COMPONENT_ADD_INCLUDEDIRS ".")
set(COMPONENT_PRIV_REQUIRES "esp32-owb" "esp-ds18b20" "tarablessd1306" "esp32-pcd8544")

idf_component_register(SRCS "networking.c" "display.c" "app_main.c" "temp_sensors.c" "test_task.c" "test_rf24.cpp" "temp_sensors2.c" "display.c" "networking.c"
                    INCLUDE_DIRS "")
