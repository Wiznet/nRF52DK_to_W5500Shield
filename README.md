# BLE to TCP using [nRF51-DK](https://www.nordicsemi.com/eng/Products/Bluetooth-low-energy/nRF52-DK) + [W5500 Shield](http://wizwiki.net/wiki/doku.php?id=osh:w5500_ethernet_shield:start)


![nRF51DK + W5500 Shield](http://wizwiki.net/wiki/lib/exe/fetch.php?media=osh:cookie:nrf52_w5500_shield.jpg "nRF51DK + W5500 Shield")

## Summary

Using WIZnet TCP/IP Ethernet chip W5500 and the Nordic BLE SoC nRF52832 implemented the BLE to Ethernet. BLE Central/Peripheral communicate using a UART Nordic Service (NUS) Profile. And BLE Central and W5500 communicate with SPI. W5500 also operates as a TCP Client, TCP Server to communicate with the LAN (through the line). The purpose of this Application Note is to build a low power IoT node environment. 

## Testing

### BLE Stack (SoftDevice) Writing
Program the S132_nrf52_2.0.1_softdevice.hex attached to project on two boards. (PCA10040 + W5500, PCA10040) 
![SD Writing](http://wizwiki.net/wiki/lib/exe/fetch.php?media=osh:cookie:3-1_bte_testing.jpg "SD Writing")

### nRF52DK to W5500Shield Board application Writing
Open the nRF52DK_to_W5500Shield\examples\ble_central\ble_uart_c_to_tcpc\pca10040\s132\arm5_no_packs path of the project. Put the IP Address of the TCP server PC to the 56th line of the variable targetIP main.c, build, and Program a ble to ethernet board.
![ipconfig](http://wizwiki.net/wiki/lib/exe/fetch.php?media=osh:cookie:ipconfig.png "ipconfig")
![targetIP](http://wizwiki.net/wiki/lib/exe/fetch.php?media=osh:cookie:3-3_bte_testing.jpg "targetIP")
![APP Writing](http://wizwiki.net/wiki/lib/exe/fetch.php?media=osh:cookie:3-4_bte_testing.jpg "APP Writing")

### PCA10040 application Writing
Build the project to open a path nRF52DK_to_W5500Shield\examples\ble_peripheral\ble_app_uart\pca10040\s132\arm5_no_packs, and Writing in PCA10040 board. 

### Config HyperTerminal & TCP Server
Three open the Hercules. Two Hercules is set to 115200 baud rate in the Serial Tab and open the Com port. One of the Hercules Port is set to 5000 in the TCP Server tab and click on the listen. 
![Hercules1](http://wizwiki.net/wiki/lib/exe/fetch.php?media=osh:cookie:3-5_bte_testing.jpg "Hercules1")
![Hercules2](http://wizwiki.net/wiki/lib/exe/fetch.php?media=osh:cookie:3-6_bte_testing.jpg "Hercules2")

### Data Input
Input from the WIZNET PCA10040 Terminal can see that is sent to the TCP Server. 
![Data Input1](http://wizwiki.net/wiki/lib/exe/fetch.php?media=osh:cookie:3-7_bte_testing.jpg "Data Input1")
![Data Input2](http://wizwiki.net/wiki/lib/exe/fetch.php?media=osh:cookie:3-8_bte_testing.jpg "Data Input2")


