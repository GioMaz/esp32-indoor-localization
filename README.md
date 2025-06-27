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
   
### Build and Flash into ESP
```bash
idf.py build flash
```

## Project layout

```
.
├── CMakeLists.txt
├── components
│   ├── ap_scan
│   │   ├── ap_scan.c
│   │   ├── ap_scan.h
│   │   └── CMakeLists.txt
│   ├── dataset
│   │   ├── CMakeLists.txt
│   │   ├── dataset.c
│   │   └── dataset.h
│   ├── gpio
│   │   ├── CMakeLists.txt
│   │   ├── gpio.c
│   │   ├── gpio.h
│   │   └── Kconfig.projbuild
│   ├── http_server
│   │   ├── CMakeLists.txt
│   │   ├── http_server.c
│   │   ├── http_server.h
│   │   ├── routes.c
│   │   └── routes.h
│   ├── nvs
│   │   ├── CMakeLists.txt
│   │   ├── nvs.c
│   │   └── nvs.h
│   ├── setup
│   │   ├── CMakeLists.txt
│   │   ├── setup.c
│   │   └── setup.h
│   ├── state_inference
│   │   ├── CMakeLists.txt
│   │   ├── state_inference.c
│   │   └── state_inference.h
│   ├── state_training
│   │   ├── CMakeLists.txt
│   │   ├── state_training.c
│   │   └── state_training.h
│   ├── storage
│   │   ├── CMakeLists.txt
│   │   ├── storage.c
│   │   └── storage.h
│   ├── utils
│   │   ├── CMakeLists.txt
│   │   ├── utils.c
│   │   └── utils.h
│   └── wifi
│       ├── CMakeLists.txt
│       ├── Kconfig.projbuild
│       ├── wifi.c
│       └── wifi.h
├── dependencies.yml
├── main
│   ├── CMakeLists.txt
│   ├── idf_component.yml
│   └── main.c
├── partitions.csv
├── README.md
├── storage_image
│   ├── dataset.bin
│   └── index.html
└── test
    └── Makefile

```     

