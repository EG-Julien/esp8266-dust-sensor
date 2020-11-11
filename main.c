#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>
#include <math.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include "wifi.h"
#include "config.h"

#define COV_RATIO                       0.2            //ug/mmm / mv
#define NO_DUST_VOLTAGE                 400            //mv
#define SYS_VOLTAGE                     1000           

const int SENSE_PIN = 0;
const int EMIT_PIN  = 13;

static void wifi_init() {
    struct sdk_station_config wifi_config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASSWORD,
    };

    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&wifi_config);
    sdk_wifi_station_connect();
}

void air_quality_sensor_identify() {
    printf("Dust sensor identify\n");
}

homekit_characteristic_t air_quality = HOMEKIT_CHARACTERISTIC_( AIR_QUALITY, 0 );
homekit_characteristic_t pm25_density = HOMEKIT_CHARACTERISTIC_( PM25_DENSITY, 0 );

int filter_adc_value(int m) {
  static int flag_first = 0, _buff[10], sum;
  const int _buff_max = 10;
  int i;
  
  if(flag_first == 0)
  {
    flag_first = 1;

    for(i = 0, sum = 0; i < _buff_max; i++)
    {
      _buff[i] = m;
      sum += _buff[i];
    }
    return m;
  }
  else
  {
    sum -= _buff[0];
    for(i = 0; i < (_buff_max - 1); i++)
    {
      _buff[i] = _buff[i + 1];
    }
    _buff[9] = m;
    sum += _buff[9];
    
    i = sum / 10.0;
    return i;
  }
}

void air_quality_sensor_task(void *_args) {

    gpio_enable(EMIT_PIN, GPIO_OUTPUT);
    
    float density, voltage;
    int adc_value, level;    

    while (1) {
        gpio_write(EMIT_PIN, 1);
        vTaskDelay(configTICK_RATE_HZ / 357); // ~ 280 µs
        adc_value = sdk_system_adc_read();
        //printf("Voltage acquired %d\n", adc_value);
        gpio_write(EMIT_PIN, 0);
        
        adc_value = filter_adc_value(adc_value);

        voltage = (SYS_VOLTAGE / 1024.0) * adc_value * 11;

        if(voltage >= NO_DUST_VOLTAGE) {
            voltage -= NO_DUST_VOLTAGE;
            density = voltage * COV_RATIO;
        }
        else
            density = 0;
        
        //printf("Voltage calculated %f\n", voltage);
        //printf("Desity calculated %f\n", density);

        if (density <= 35) {
            level = 1;
        } else if (density > 35 && density <= 75) {
            level = 2;
        } else if (density > 75 && density <= 115) {
            level = 3;
        } else if (density > 115 && density <= 150) {
            level = 4;
        } else {
            level = 5;
        }

        air_quality.value.int_value = level;
        //homekit_characteristic_notify(&air_quality, HOMEKIT_INT(level));
        homekit_characteristic_notify(&pm25_density, HOMEKIT_FLOAT(density));
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void air_quality_sensor_init () {
    xTaskCreate(air_quality_sensor_task, "Air Quality Sensor", 512, NULL, 2, NULL);
}


homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_sensor, .services=(homekit_service_t*[]){
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Dust sensor"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Kariboo"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "17102015JBHI"),
            HOMEKIT_CHARACTERISTIC(MODEL, "Kariboo Dust Sensor"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.1"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, air_quality_sensor_identify),
            NULL
        }),

        HOMEKIT_SERVICE(AIR_QUALITY_SENSOR, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "Dust sensor"),
                &air_quality,
                &pm25_density,
                NULL
            }),
        NULL
    }),
    NULL
};

homekit_server_config_t config = {
    .accessories = accessories,
    .password = "111-11-111"
};

void user_init(void) {
    uart_set_baud(0, 115200);

    wifi_init();
    air_quality_sensor_init();
    homekit_server_init(&config);
}