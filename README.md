## ESP-IDF Project Setup

### Prerequisites

#### Install ESP-IDF (recommended version: 5.4.1 or later).

https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/index.html#installation

#### Initialize esp idf environment

```bash
$IDF_PATH/export.sh
```

### Initial Project Configuration

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

### Build

```bash
idf.py build
```
