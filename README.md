# Mailbox Notification Device Project
Repository for my wireless-temperature-and-humidity logger project. Code for transmitter and receiver are placed under separate folder, namely "tx-" and "rx-". 

**<u>FYI</u>:** This project is currently ***In Progress***.
<hr>

## <u>Pins used (Transmitter STM32-blackpill)</u>:

### RYLR689 LoRa module:
- PA5: SCK
- PA6: MISO
- PA7: MOSI
- PA8: NSS (CS)
- PB0: RF CONTROL: RFSW_V1_PIN_POS
- PB1: RF CONTROL: RFSW_V2_PIN_POS
- PB5: BUSY
- PB8: RESET (Active-Low)
- PB9: DIO (Interrupt)

### IR beam transmitter and receiver sensor:
- PB10: IR beam receiver output (Pull-up required for MCU input pin).
- PB12: IR beam receiver power.
- PB13: IR beam transmitter power.

### For debug:
- PA2: UART TX
- PA3: UART RX
- PC13: User LED for debugging purposes.

<hr>

## <u>Pins used (Receiver STM32-blackpill)</u>:

### RYLR689 LoRa module:
- PA5: SCLK
- PA6: MISO
- PA7: MOSI
- PA8: NSS (CS)
- PB0: RF CONTROL: RFSW_V1_PIN_POS
- PB1: RF CONTROL: RFSW_V2_PIN_POS
- PB5: BUSY
- PB8: RESET (Active-Low)
- PB9: DIO (Interrupt)

### DS3231 (RTC module):
- PB6: SCL
- PB7: SDA

### SD-card module (SPI-based):
- PA1: CS
- PA5: SCLK
- PA6: MISO
- PA7: MOSI

### OLED Display (SPI-based):
- PA4: CS
- PA5: SCLK
- PA7: MOSI
- PB14: DC
- PB15: RESET (Active-Low)

## LED indication lights:
- PB12: Blue LED 
- PB13: Yellow LED
  
### For debug:
- PA2: UART TX
- PA3: UART RX
- PC13: User LED for debugging purposes.

<hr>

# Flow:
- **TBU**: I will update this with an image of a diagram, typing it is not easy.

<hr>

# Demo:

### RX side Image(s):

**<u>Components:<u>**
- STM32F411CEU (blackpill), RYLR689 LoRa module, SD card module, DS3231 RTC module, OLED SH1106 module, Power supply, 9v Battery.
![Components used for RX device in this project.](/demo/images/rx_device_images/rx-setup.jpg)

**<u>Wirings:<u>**
- ***Note:*** The solder blobs you would see here were intentionally done for better contact.
![Wirings of RX device.](/hardware/rx-dev-wirings.jpg)

**<u>Initial Status:<u>**
![This is initial status at boot.](/demo/images/rx_device_images/rx-no-mail-demo.jpg)

**<u>Updated Status: New Mail arrived!<u>**
![This is the demo when new mail arrives.](/demo/images/rx_device_images/rx-new-mail-demo.jpg)

**<u>Updated Status: New Mail arrived!<u>**
![This is the demo when old and possibly new mail is inside the mailbox.](/demo/images/rx_device_images/rx-old-new-demo.jpg)


### TX side Image(s):

**<u>Components:<u>**
- STM32F411CEU (blackpill), RYLR689 LoRa module, SD card module, IR transmitter-receiver, Reed switch (Normally open).
![Components for TX device used in this project.](/demo/images/tx_device_images/tx_setup.jpg)

**<u>Wirings:<u>**
- ***Note:*** The solder blobs you would see here were intentionally done for better contact.
![Wirings of TX device.](/hardware/tx-dev-wirings.jpg)

**<u>IR-sensor and Reed switch:<u>**
![Close-up image of the IR sensor used in this project.](/demo/images/tx_device_images/tx_ir_sensor_close_up_md.jpg)

### Video(s):
- **TBU**: I will post a simple demonstration video here.
