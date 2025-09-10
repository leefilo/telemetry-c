#pragma once

typedef struct {
  char id[16];
  char unit[12];
  double min, max;
  unsigned period_ms;   
} sensor_cfg_t;

typedef struct {
  sensor_cfg_t s[4];
  int count;  // expect 4 if valid
} car_sensors_t;
