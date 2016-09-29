# BLE to TCP using [nRF51-DK](https://www.nordicsemi.com/eng/Products/Bluetooth-low-energy/nRF52-DK) + [W5500 Shield](http://wizwiki.net/wiki/doku.php?id=osh:w5500_ethernet_shield:start)


![nRF51DK + W5500 Shield](http://wizwiki.net/wiki/lib/exe/fetch.php?media=osh:cookie:nrf52_w5500_shield.jpg "nRF51DK + W5500 Shield")

## Summary

Using WIZnet TCP/IP Ethernet chip W5500 and the Nordic BLE SoC nRF52832 implemented the BLE to Ethernet. BLE Central/Peripheral communicate using a UART Nordic Service (NUS) Profile. And BLE Central and W5500 communicate with SPI. W5500 also operates as a TCP Client, TCP Server to communicate with the LAN (through the line). The purpose of this Application Note is to build a low power IoT node environment. 

## Testing

### BLE Stack (SoftDevice) Writing
Program the S132_nrf52_2.0.1_softdevice.hex attached to project on two boards. (PCA10040 + W5500, PCA10040) 
![nRF51DK + W5500 Shield](http://wizwiki.net/wiki/lib/exe/fetch.php?media=osh:cookie:nrf52_w5500_shield.jpg "nRF51DK + W5500 Shield")

![SD Writing](http://wizwiki.net/wiki/lib/exe/fetch.php?media=osh:cookie:3-1_bte_testing.jpg "SD Writing")

