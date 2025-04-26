#pragma once

#include <stdint.h>

static const uint32_t MAX_AP_LIST_SIZE = 20;

typedef struct {
  /*int8_t channel;*/
  uint8_t mac[6];
  int8_t rssi;
} AccessPoint;

/*typedef double Features[2];*/
typedef struct {
  double x, y;
} Features;

typedef struct {
  int16_t x, y;
} Label;

typedef struct {
  Features features;
  Label label;
} FeaturesLabel;

void setup_wifi(void);
void wifi_scan(AccessPoint aps[], uint32_t *ap_count);
