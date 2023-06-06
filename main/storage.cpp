#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <time.h>
#include <hardware/flash.h>
#include "hardware/sync.h"
#include <string.h>
#include <stdlib.h>

#define FLASH_SIZE 0x200000

#define MAX_SCHEDULES 8

typedef uint8_t day_of_week_pattern;

struct schedule_data {
  bool enabled;
  day_of_week_pattern day_of_week;
  uint8_t hour;
  uint8_t minute;
  uint16_t duration;
};

struct dst_change_schedule {
  uint8_t month;
  int8_t nth_sunday;
};

struct nvdata {
  uint8_t version;
  bool h24format;
  bool in_dst;
  bool dst_schedule_enabled;
  bool last_dst_change; // true if clock time is currently dst
  dst_change_schedule dst_enter;
  dst_change_schedule dst_exit;
  uint8_t schedule_count;
  schedule_data schedules[MAX_SCHEDULES];
};

int schedule_cmp(const void* _a, const void* _b) {
  schedule_data* a = (schedule_data*)_a;
  schedule_data* b = (schedule_data*)_b;
  if(a->enabled && !b->enabled)
    return 1;
  if(!a->enabled && b->enabled)
    return -1;
  if(a->day_of_week < b->day_of_week)
    return 1;
  if(a->day_of_week > b->day_of_week)
    return -1;
  if(a->hour > b->hour)
    return 1;
  if(a->hour < b->hour)
    return -1;
  if(a->minute > b->minute)
    return 1;
  if(a->minute < b->minute)
    return -1;
  return 0;
}

class NVStorage {
  public:
    uint8_t buffer[FLASH_PAGE_SIZE];
    nvdata *data;
    nvdata *nvlocation;
    NVStorage() {
      nvlocation = (nvdata *)(XIP_BASE + FLASH_SIZE - FLASH_SECTOR_SIZE); // last sector of program memory
      data = (nvdata*)buffer;
      reset();
    }
    void load() {
      if(sizeof(nvdata) > FLASH_PAGE_SIZE)
        while (true) {
          sleep_ms(250);
          gpio_put(LED_PIN, 1);
          sleep_ms(250);
          gpio_put(LED_PIN, 0);
        }
      memcpy(buffer, nvlocation, sizeof(buffer));
      if(data->version==0xFF) {
        reset();
        store();
      }
    }
    void reset() {
      data->version=1;
      data->h24format = true;
      data->in_dst = true;
      data->dst_schedule_enabled = true;
      data->last_dst_change = true;
      data->dst_enter.month = 10 - 1;
      data->dst_enter.nth_sunday = 1;
      data->dst_exit.month = 4 - 1;
      data->dst_exit.nth_sunday = 1;
      data->schedule_count = 0;
      for(uint8_t i; i < MAX_SCHEDULES; i++) {
        data->schedules[i].enabled = true;
        data->schedules[i].day_of_week = 0x7F;
        data->schedules[i].hour = 9;
        data->schedules[i].minute = 0;
        data->schedules[i].duration = 3600;
      }
    }
    void store() {
      uint32_t ints = save_and_disable_interrupts();
      flash_range_erase (((uint)nvlocation - XIP_BASE), FLASH_SECTOR_SIZE);
      flash_range_program (((uint)nvlocation - XIP_BASE), buffer, FLASH_PAGE_SIZE);
      restore_interrupts (ints);
    }
    void sort_schedules() {
      qsort(data->schedules, data->schedule_count, sizeof(schedule_data), schedule_cmp);
    }
    void delete_schedule(uint8_t index) {
      data->schedule_count--;
      for(uint8_t i = index; i < data->schedule_count; i++) {
        data->schedules[i] = data->schedules[i+1];
      }
    }
};

NVStorage* storage;
