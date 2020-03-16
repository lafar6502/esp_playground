#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"
#include "temp_sensors.h"
#include "display.h"
#include "networking.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_log.h"


#define MAX_DEVICES 8
#define DS18B20_RESOLUTION (DS18B20_RESOLUTION_10_BIT)

#define MAX6675_MISO 19
#define MAX6675_SCK 18
#define MAX6675_CS 5
#define MAX6675_SPI_HOST VSPI_HOST

static const char* TAG = "PG_TS";

OneWireBus_ROMCode g_device_rom_codes[MAX_DEVICES] = {0};
DS18B20_Info * g_devices[MAX_DEVICES] = {0};
OneWireBus * owb = NULL;
int num_devices = 0;
owb_rmt_driver_info rmt_driver_info;

void spi_therm_init(void);

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
    printf("now init max6675\n");
    spi_therm_init();
    printf("max6675 init done\n");
};

uint16_t readMax6675();
float spi_therm_read();

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

    float mx = spi_therm_read();

    char buf[30];
    char buf2[30];
    sprintf(buf, "%2.1f %2.1f %2.1f", readings[0], readings[1], mx);
    sprintf(buf2, "%s %d", getCurrentIP(), num_devices);
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
    
    showText(buf2, buf, NULL, NULL);

    
}


uint16_t readMax6675() {
    //return 0;
	uint16_t data,rawtemp,temp=0;
    spi_bus_config_t bus_config;
	spi_device_interface_config_t dev_config;
	spi_transaction_t trans_word;
    spi_device_handle_t dev_handle;
	ESP_LOGD(TAG, "readMax6675 start");

    gpio_set_level(MAX6675_CS, 1); // MAX6675_CS
	memset(&bus_config, 0, sizeof(bus_config));
    bus_config.sclk_io_num   = MAX6675_SCK; // CLK
	bus_config.mosi_io_num   = -1; // MOSI not used
	bus_config.miso_io_num   = MAX6675_MISO; // MISO
	bus_config.quadwp_io_num = -1; // not used
	bus_config.quadhd_io_num = -1; // not used
    ESP_LOGD(TAG, "spi_bus_initialize");
	ESP_ERROR_CHECK(spi_bus_initialize(MAX6675_SPI_HOST, &bus_config, 0));
    printf("spi busi inited\r\n");
    

    memset(&dev_config, 0, sizeof(dev_config));
	dev_config.address_bits     = 0;
	dev_config.command_bits     = 0;
	dev_config.dummy_bits       = 0;
	dev_config.mode             = 0; // SPI_MODE0
	dev_config.duty_cycle_pos   = 0;
	dev_config.cs_ena_posttrans = 0;
	dev_config.cs_ena_pretrans  = 0;
	//dev_config.clock_speed_hz   = 2000000;  // 2 MHz
	dev_config.clock_speed_hz   = 10000;  // 10 kHz
	dev_config.spics_io_num     = -1; // CS External
	dev_config.flags            = 0; // SPI_MSBFIRST
	dev_config.queue_size       = 100;
	dev_config.pre_cb           = NULL;
	dev_config.post_cb          = NULL;
    ESP_LOGD(TAG, "spi_bus_add_device");
	ESP_ERROR_CHECK(spi_bus_add_device(MAX6675_SPI_HOST, &dev_config, &dev_handle));
    

    ESP_LOGD(TAG, "MAX6675_CS prepare");
    gpio_set_level(MAX6675_CS, 0); // MAX6675_CS prepare
    vTaskDelay(500 / portTICK_RATE_MS);  // see MAX6675 datasheet

    rawtemp = 0x000;
    data = 0x000;  // write dummy

	trans_word.addr   = 0;
	trans_word.cmd   = 0;
	trans_word.flags     = 0;
	trans_word.length    = 8 * 2; // Total data length, in bits NOT number of bytes.
	trans_word.rxlength  = 0; // (0 defaults this to the value of ``length``)
	trans_word.tx_buffer = &data;
	trans_word.rx_buffer = &rawtemp;
    ESP_LOGD(TAG, "spi_device_transmit");
	ESP_ERROR_CHECK(spi_device_transmit(dev_handle, &trans_word));

	gpio_set_level(MAX6675_CS, 1); // MAX6675_CS prepare

    temp = ((((rawtemp & 0x00FF) << 8) | ((rawtemp & 0xFF00) >> 8))>>3)*25;
   // temp = ((rawtemp)>>3)*25;
	ESP_LOGI(TAG, "readMax6675 spiReadWord=%x temp=%d.%d",rawtemp,temp/100,temp%100);
    
    ESP_LOGD(TAG, "spi_bus_remove_device");
    ESP_ERROR_CHECK(spi_bus_remove_device(dev_handle));
    
    ESP_LOGD(TAG, "spi_bus_free");
    ESP_ERROR_CHECK(spi_bus_free(MAX6675_SPI_HOST));

    return temp;
}

spi_bus_config_t g_bus_config;
spi_device_interface_config_t g_dev_config;
spi_device_handle_t dev_handle;
	

void spi_therm_init(void) {

    spi_bus_config_t bus_config;
	ESP_LOGD(TAG, "spi_therm_init start");

    gpio_set_level(MAX6675_CS, 1); // MAX6675_CS
	memset(&bus_config, 0, sizeof(bus_config));
    bus_config.sclk_io_num   = MAX6675_SCK; // CLK
	bus_config.mosi_io_num   = -1; // MOSI not used
	bus_config.miso_io_num   = MAX6675_MISO; // MISO
	bus_config.quadwp_io_num = -1; // not used
	bus_config.quadhd_io_num = -1; // not used
    ESP_LOGD(TAG, "spi_bus_initialize");
	ESP_ERROR_CHECK(spi_bus_initialize(MAX6675_SPI_HOST, &g_bus_config, 0));
    printf("spi busi inited\r\n");
    

    memset(&g_dev_config, 0, sizeof(g_dev_config));
    g_dev_config.mode = 0;
    g_dev_config.clock_speed_hz = 4 * 1000 * 1000;
    g_dev_config.spics_io_num = MAX6675_CS;
    g_dev_config.queue_size=3;
    
  ESP_ERROR_CHECK(spi_bus_add_device(VSPI_HOST, &g_dev_config, &dev_handle));
  printf("therm dev inited, h %lx\r\n", (long unsigned int) dev_handle);
};


float spi_therm_read(void) {

  uint16_t bits = 0;
  spi_transaction_t tM = {
    .tx_buffer = NULL,
    .rx_buffer = &bits,
    .length = 16,
    .rxlength = 16,
  };

  spi_device_acquire_bus(dev_handle, portMAX_DELAY); // Probably unnecessary
  spi_device_transmit(dev_handle, &tM);
  spi_device_release_bus(dev_handle);

  uint16_t res = SPI_SWAP_DATA_RX(bits,16);

  res >>= 3;
  ESP_LOGD(TAG, "Value %d, temp %f", res, res*0.25);

  return res*0.25;

}