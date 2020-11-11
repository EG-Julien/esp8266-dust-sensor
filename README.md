# Smoke, dust and air quality 
This sensor is based on [this module](https://www.amazon.fr/gp/product/B07P94LCFZ/ref=ppx_yo_dt_b_asin_title_o04_s00?ie=UTF8&psc=1), which is capable of mesuring PM25 density in µg / m3. You have to connect IRLED to GPIO 13 and AOUT to ADC Input. Home App return the air quality which is calculated with the PM25 density.

## Homekit integration
No bridge is required. Instead, this project uses the esp-homekit implementation found
here: https://github.com/maximkulkin/esp-homekit to directly connect with homekit
controller devices.

# Installation
1. Install [esp-open-sdk](https://github.com/pfalcon/esp-open-sdk), build it with `make toolchain esptool libhal STANDALONE=n`, then edit your PATH and add the generated toolchain bin directory. The path will be something like /path/to/esp-open-sdk/xtensa-lx106-elf/bin.
2. Install [esptool.py](https://github.com/themadinventor/esptool) and make it available on your PATH. If you used esp-open-sdk then this is done already.
3. Checkout [esp-open-rtos](https://github.com/SuperHouse/esp-open-rtos) and set SDK_PATH environment variable pointing to it.
4. Clone [esp-homekit-demo](https://github.com/maximkulkin/esp-homekit-demo) (for archive purposes forked [here](https://github.com/DominikHorn/esp-homekit-demo))
   and initialize all submodules (recursively):
```shell
git clone https://github.com/maximkulkin/esp-homekit-demo
cd esp-homekit-demo
git submodule update --init --recursive
```
5. Copy wifi.h.sample -> wifi.h and edit it with correct WiFi SSID and password:
```shell
cp wifi.h.sample wifi.h
vi wifi.h
```
6. Clone this project into the examples subfolder and copy config.h.sample -> config.h. Set correct config options afterwards:
```shell
cd examples
git clone https://github.com/EG-Julien/esp8266-dust-sensor.git
cd esp8266-dust-sensor
```
7. Configure settings:
    1. If you use ESP8266 with 4MB of flash (32m bit), then you're fine. If you have
1MB chip, you need to set following environment variables:
    ```shell
    export FLASH_SIZE=8
    export HOMEKIT_SPI_FLASH_BASE_ADDR=0x7a000
    ```
    2. If you're debugging stuff, or have troubles and want to file issue and attach log, please enable DEBUG output:
    ```shell
    export HOMEKIT_DEBUG=1
    ```
    3. Depending on your device, it might be required to change the flash mode:
    ```shell
    export FLASH_MODE=dout
    ```
    (see issue #80)
8. Build:
```shell
cd esp-homekit-demo/examples/esp8266-dust-sensor
make -C . all
```
9. Set ESPPORT environment variable pointing to USB device your ESP8266 is attached
   to (assuming your device is at /dev/tty.SLAB_USBtoUART):
```shell
export ESPPORT=/dev/tty.SLAB_USBtoUART
```
10. To prevent any effects from previous firmware (e.g. firmware crashing right at
   start), highly recommend to erase flash:
```shell
    cd esp-homekit-demo/examples/esp8266-dust-sensor
    make -C . erase_flash
```
11. Upload firmware to ESP:
```shell
    cd esp-homekit-demo/examples/esp8266-dust-sensor
    make -C . test
```
  or
```shell
    cd esp-homekit-demo/examples/esp8266-dust-sensor
    make -C flash
    make -C monitor
```

12. Open Home app on your device, and click '+'. If Home does not recognize the ESP8266,
proceed to adding device anyways by entering code.

# Troubleshooting
Try to monitor the ESP8266 while it's executing the software:
```shell
    cd esp-homekit-demo/examples/esp8266-dust-sensor
    make -C . monitor
```
It is often also a good idea to reset the ESP (cutoff power).
