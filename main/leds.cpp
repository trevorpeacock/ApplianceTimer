#include "defines.cpp"

#define R_LED_PIN 22 //PWM3 A
#define R_LED_PWM_CHANNEL PWM_CHAN_A
#define G_LED_PIN 21 //PWM2 B
#define G_LED_PWM_CHANNEL PWM_CHAN_B
#define B_LED_PIN 26 //PWM5 A
#define B_LED_PWM_CHANNEL PWM_CHAN_A

uint r_led_slice_num;
uint g_led_slice_num;
uint b_led_slice_num;

void setup_leds() {
  gpio_set_function(R_LED_PIN, GPIO_FUNC_PWM);
  r_led_slice_num = pwm_gpio_to_slice_num(R_LED_PIN);
  pwm_set_clkdiv(r_led_slice_num, 5);
  pwm_set_wrap(r_led_slice_num, 255);
  pwm_set_chan_level(r_led_slice_num, R_LED_PWM_CHANNEL, 255);
  pwm_set_enabled(r_led_slice_num, true);

  gpio_set_function(G_LED_PIN, GPIO_FUNC_PWM);
  g_led_slice_num = pwm_gpio_to_slice_num(G_LED_PIN);
  pwm_set_clkdiv(g_led_slice_num, 5);
  pwm_set_wrap(g_led_slice_num, 255);
  pwm_set_chan_level(g_led_slice_num, G_LED_PWM_CHANNEL, 255);
  pwm_set_enabled(g_led_slice_num, true);

  gpio_set_function(B_LED_PIN, GPIO_FUNC_PWM);
  b_led_slice_num = pwm_gpio_to_slice_num(B_LED_PIN);
  pwm_set_clkdiv(b_led_slice_num, 5);
  pwm_set_wrap(b_led_slice_num, 255);
  pwm_set_chan_level(b_led_slice_num, B_LED_PWM_CHANNEL, 255);
  pwm_set_enabled(b_led_slice_num, true);
}

void set_leds(uint8_t r, uint8_t g, uint8_t b) {
  pwm_set_chan_level(r_led_slice_num, R_LED_PWM_CHANNEL, 255-r);
  pwm_set_chan_level(g_led_slice_num, G_LED_PWM_CHANNEL, 255-g);
  pwm_set_chan_level(b_led_slice_num, B_LED_PWM_CHANNEL, 255-b);
}