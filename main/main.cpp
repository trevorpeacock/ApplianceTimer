#include "pico/stdlib.h"
#include "u8g2/csrc/u8g2.h"
#include "defines.cpp"
#include "storage.cpp"
#include "time.cpp"
#include "leds.cpp"
#include "menu.cpp"
#include "tests.cpp"


int main() {
  stdio_init_all();

  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  gpio_init(RELAY_PIN);
  gpio_set_dir(RELAY_PIN, GPIO_OUT);

  setup_leds();
  set_leds(250, 0, 0);
  sleep_ms(250);
  set_leds(0, 255, 0);
  sleep_ms(250);
  set_leds(0, 0, 255);
  sleep_ms(250);
  set_leds(4, 4, 3);

  // If you don't do anything before initializing a display pi pico is too fast and starts sending
  // commands before the screen controller had time to set itself up, so we add an artificial delay for
  // ssd1306 to set itself up
  //sleep_ms(250);

  Display display = Display();
  test_time_calculations(&display);

  Buttons buttons = Buttons();

  storage = new NVStorage();
  ds1302 = new DS1302();

  buttons.update_state();
  if(buttons.left_button_raw->get_last_state() && buttons.right_button_raw->get_last_state()) {
    storage->reset();
    storage->store();
    current_datetime = (datetime_data){{0, 0, 0, 0}, {0, 0, 0}};
    ds1302->write_time();

    u8g2_ClearBuffer(&display.u8g2);
    u8g2_SetFont(&display.u8g2, u8g2_font_timR18_tf);
    u8g2_DrawStrCenter(&display.u8g2, 16, "Memory");
    u8g2_DrawStrCenter(&display.u8g2, 48, "Reset");
    u8g2_SendBuffer(&display.u8g2);

    while(buttons.left_button_raw->get_last_state() && buttons.right_button_raw->get_last_state()) {
      buttons.update_state();
      set_leds(255, 4, 3);
      sleep_ms(100);
      set_leds(4, 4, 3);
      sleep_ms(100);
    }
  } else {
  }
  storage->load();

  //TODO: Debug mode

  ds1302->read_time();
  ds1302->check_dst();


  WelcomeDisplay menu = WelcomeDisplay(&display, &buttons);
  menu.init();
  menu.enter();
  timer = new Timer();

  uint8_t last_schedule_check_seconds = 255;
  schedule_calc_data schedules_temp[MAX_SCHEDULES];

  bool status_led = false;

  while (true) {
    buttons.update_state();
    ds1302->read_time();

    if(last_schedule_check_seconds != current_datetime.time.second) {
      last_schedule_check_seconds = current_datetime.time.second;

      build_next_schedule(schedules_temp);
      for(uint8_t i = 0; i < storage->data->schedule_count; i++) {
        if(schedules_temp[i].datetime==current_datetime)
          timer->update_timer(schedules_temp[i].duration);
      }

      if(storage->data->dst_schedule_enabled && next_dst_change==current_datetime) {
        ds1302->check_dst();
      }
    }

    menu.process();
    gpio_put(LED_PIN, status_led);
    status_led = !status_led;
  }
}