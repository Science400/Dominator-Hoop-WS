# Dominator Hoop Testing Server

## Description

This is an ESP32 application that cycles a stepper motor back and forth. It is controlled by a web page served by the ESP32. It was written by Isaiah Young for Home Sports.

## Prerequisites

Please install:

- [VS Code](https://code.visualstudio.com/)
- [PlatformIO](https://platformio.org/)

## Installation

Download this repository as a zip file and extract to the PlatformIO projects folder. From the PlatformIO Home > Projects, select Add Existing and navigate to the Dominator Hoop folder.

Edit main.cpp to update the Wi-Fi credentials and Basketball Hoop Constants.

With the PlatformIO extension selected in the sidebar you should see an option to Upload Filesystem Image (esp32dev). Select that to upload the files for the web app.

Then, above that, select Upload and Monitor. This will upload the code to the microcontroller then show the output in the terminal below. Once it has finished uploading you should see that it is trying to connect to the wifi and once it has it will show the IP address. Navigate to that IP on a computer or phone connected to the same Wi-Fi network.

## Use

Once the webpage is pulled up, simply enter the number of cycles and press start. You'll see a progress bar and countdown. You'll also see a countdown in the serial monitor.

## Troubleshooting

If uploading the filesystem image errors out, it is probably because the SPIFFS hasn't been initialized. Open the Arduino IDE and select File>Examples>SPIFFS>SPIFFS_test. Upload the the example and make sure it works by opening the Serial Monitor. It will just initialize the file system and run through a couple of tests to make sure it works.
