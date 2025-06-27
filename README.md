# ESP32 Indoor Localization

The objective of this project is to localize the esp32 in an indoor environment.

The workflow consists in two parts, first we perform the training, we map the environment by scanning each position, resulting in a dataset composed by fingerprints where a fingerprint is represented by the tuple: (x, y, access points list). Once the dataset is ready, the main loop continuously scans for near access points and based on that an algorithm determines the current position.

<div align="center">
  <p>
    <img src="https://github.com/user-attachments/assets/822847b6-4ac6-48a1-bccb-021eff6e030a" alt="inference_training" width="400px">
  </p>
  <h4>On the left the training phase, while on the right the inference phase (both sped up).</h4>
</div>

## Prerequisites
### Hardware
In order to run the project, the following hardware is needed:
- 1x ESP32 microcontroller
- 6x buttons
- Jumper wires
- Breadboard
  
### Software
- [Espressif IoT Development Framework](https://www.espressif.com/en/products/sdks/esp-idf)

## Setup
### Hardware
In order to end up with the intended behaviour, you must replicate the following configuration:
<div align="center">
  <p>
    <img src="https://github.com/user-attachments/assets/edbc8f3a-03c5-4459-8f9c-e31b1f543c51" alt="hardware setup" width="400px">
  </p>
  <h4>Hardwares setup and connections.</h4>
</div>

### Software
#### Initialize esp idf environment
```bash
$IDF_PATH/export.sh
```

#### Clone the repo
```bash
git clone https://github.com/GioMaz/esp32-rssi-location-estimator
cd esp32-rssi-location-estimator
```

#### Set the target for your board (e.g., ESP32S3, ESP32, etc.)
```bash
idf.py set-target esp32s3
```

#### Reconfigure the build environment and install dependencies
```bash
idf.py reconfigure
```

#### Set a Custom Partition Scheme
1. Run the menuconfig tool with `idf.py menuconfig`
2. Navigate to `Partition Table → Partition Table (Custom partition table CSV)`
   
#### Set Custom GPIO Pins
1. Run the menuconfig tool with `idf.py menuconfig`
2. Navigate to `GPIO Config → Set custom GPIO Pins (or leave them as default)`

   Default config:
   ```
   (21) Right Button Pin
   (3) Top Button Pin
   (22) Down Button Pin
   (23) Left Button Pin
   (19) Scan Button Pin
   (18) Mode Switch Button Pin 
   ```

#### Set Custom Access Point config
1. Run the menuconfig tool with `idf.py menuconfig`
2. Navigate to `Access Point Config → Set custom Access Point configuration (or leave it as default)`

   Default config:
   ```
   (esp32-indoor-localization) Access Point SSID
   (123456789) Access Point Password
   (unitn-x) Access Point SSID Filter 
   (4) Maximal STA Connections 
   ```

#### Set Main Task Size
1. Run the menuconfig tool with `idf.py menuconfig`
2. Navigate to `Component Config → ESP System Settings → Main task stack size (8000)`

#### Set Flash Size
1. Run the menuconfig tool with `idf.py menuconfig`
2. Navigate to `Serial Flasher Config → Flash Size (4 MB)`
   
### Build and Flash into ESP
```bash
idf.py build flash
```

Additionally, if you want to see the output logs you may run:
```bash
idf.py monitor
```

## Project layout

```
esp32-indoor-localization
├── CMakeLists.txt
├── components
│   ├── ap_scan                  # Scan for AP data and extract the relevant information
│   │   ├── ap_scan.c        
│   │   ├── ap_scan.h
│   │   └── CMakeLists.txt
│   ├── dataset                  # Data Structure for storing the Fingerprints
│   │   ├── CMakeLists.txt
│   │   ├── dataset.c
│   │   └── dataset.h
│   ├── gpio                     # Setup task and interrupts for handling GPIO
│   │   ├── CMakeLists.txt
│   │   ├── gpio.c
│   │   ├── gpio.h
│   │   └── Kconfig.projbuild
│   ├── http_server              # Setup task for running the Http Server and handling API endpoints
│   │   ├── CMakeLists.txt
│   │   ├── http_server.c
│   │   ├── http_server.h
│   │   ├── routes.c
│   │   └── routes.h
│   ├── nvs                      # Setup the ESP internal storage
│   │   ├── CMakeLists.txt
│   │   ├── nvs.c
│   │   └── nvs.h
│   ├── setup                    # Collect the initial setup into a single API
│   │   ├── CMakeLists.txt
│   │   ├── setup.c
│   │   └── setup.h
│   ├── state_inference          # Algorithm for predicting the user's current position
│   │   ├── CMakeLists.txt
│   │   ├── state_inference.c
│   │   └── state_inference.h
│   ├── state_training           # Handling the changes of positions, AP scans and dataset insertions
│   │   ├── CMakeLists.txt
│   │   ├── state_training.c
│   │   └── state_training.h
│   ├── storage                  # API for interaction with ESP file system (using littlefs)
│   │   ├── CMakeLists.txt
│   │   ├── storage.c
│   │   └── storage.h
│   ├── utils                    # Utily functions and definitions
│   │   ├── CMakeLists.txt
│   │   ├── utils.c
│   │   └── utils.h
│   └── wifi                     # Setup WiFi module for both running the AP and performing scans
│       ├── CMakeLists.txt
│       ├── Kconfig.projbuild
│       ├── wifi.c
│       └── wifi.h
├── dependencies.yml
├── main                         # Setup peripherals, initialize application dataset and other relevant variables, finally start main loop
│   ├── CMakeLists.txt
│   ├── idf_component.yml
│   └── main.c
├── partitions.csv
├── README.md
└── storage_image                # Content of the ESP storage (binary dataset, web page)
    ├── dataset.bin
    └── index.html
```

## User Guide

> Make sure that you followed the setup, correctly flashed the ESP and connected it to a power source.

### Training Phase
- Step 1: Connect to the ESP Access Point trought WiFi
- Step 2: Navigate to the web site, hosted at the default address `192.168.4.1`
- Step 3: Navigate to the intended position by using the directional buttons on the breadboard, you should see the map on the website updating in real time
- Step 4: Press the SCAN button on the breadboard, in order to acquire information about the nearby access points
- Step 5: Repeat the previous two steps, until the completion of the dataset

### Inference Phase
- Step 6: Click the `Download` button on the website in order to save the dataset into a binary file
- Step 7: Press the MODE button on the breadboard (or the `Switch to INFERENCE` button on the website) in order to start the inference loop
- Step 8: Move around the area and look at the predicted position on the website
