#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"
#include "temp_sensors.h"

#define MAX_DEVICES 8
#define DS18B20_RESOLUTION (DS18B20_RESOLUTION_10_BIT)

OneWireBus_ROMCode g_device_rom_codes[MAX_DEVICES] = {0};
DS18B20_Info * g_devices[MAX_DEVICES] = {0};
OneWireBus * owb = NULL;
int num_devices = 0;
owb_rmt_driver_info rmt_driver_info;
    
void tempSensorsInit() {
    printf(" gpio %d\r\n", GPIO_DS18B20_0);


    
    owb = owb_rmt_initialize(&rmt_driver_info, GPIO_DS18B20_0, RMT_CHANNEL_1, RMT_CHANNEL_0);
    owb_use_crc(owb, true);  // enable CRC check for ROM code
    printf("owb init owb 0x%x, driver 0x%x\r\n",  (uint32_t) owb, (uint32_t) owb->driver);
    
    // Find all connected devices
    printf("Find devices:\n");
    
    OneWireBus_SearchState search_state = {0};
    bool found = false;
    owb_search_first(owb, &search_state, &found);
    while (found)
    {
        char rom_code_s[17];
        owb_string_from_rom_code(search_state.rom_code, rom_code_s, sizeof(rom_code_s));
        printf("  %d : %s\n", num_devices, rom_code_s);
        g_device_rom_codes[num_devices] = search_state.rom_code;
        ++num_devices;
        owb_search_next(owb, &search_state, &found);
    }
    printf("Found %d device%s\n", num_devices, num_devices == 1 ? "" : "s");


    for (int i = 0; i < num_devices; ++i)
    {
        DS18B20_Info * ds18b20_info = ds18b20_malloc();  // heap allocation
        g_devices[i] = ds18b20_info;

        if (num_devices == 0)
        {
            printf("Single device optimisations enabled\n");
            ds18b20_init_solo(ds18b20_info, owb);          // only one device on bus
        }
        else
        {
            ds18b20_init(ds18b20_info, owb, g_device_rom_codes[i]); // associate with bus and device
        }
        ds18b20_use_crc(ds18b20_info, true);           // enable CRC check for temperature readings
        ds18b20_set_resolution(ds18b20_info, DS18B20_RESOLUTION);
    }
    printf("requesting read. devs %d, owb 0x%x, driver 0x%x\r\n", num_devices, (uint32_t) owb, (uint32_t) owb->driver);
    ds18b20_convert_all(owb);
};


void tempSensorsRead() 
{
    float readings[MAX_DEVICES] = { 0 };
    DS18B20_ERROR errors[MAX_DEVICES] = { 0 };
    printf("trying to read. devs %d, owb 0x%x, driver 0x%x\r\n", num_devices, (uint32_t) owb, (uint32_t) owb->driver);

    ds18b20_convert_all(owb);
    ds18b20_wait_for_conversion(g_devices[0]);
    
    for (int i = 0; i < num_devices; ++i)
    {
        errors[i] = ds18b20_read_temp(g_devices[i], &readings[i]);
    }


    for (int i = 0; i < num_devices; ++i)
    {
        if (errors[i] != DS18B20_OK)
        {
            printf("dev %d error!\r\n", i);
        }
	else 
        {
		printf("dev %d TEMP %.1f\r\n", i, readings[i]);
        }
    }    
    
}

