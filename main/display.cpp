#include "defines.cpp"
#include "hardware/i2c.h"

uint8_t u8x8_gpio_and_delay (u8x8_t * u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
  // Re-use library for delays

  switch (msg) {
    // called once during init phase of u8g2/u8x8
    // can be used to setup pins
    case U8X8_MSG_GPIO_AND_DELAY_INIT:	// called once during init phase of u8g2/u8x8
      break;							// can be used to setup pins
    case U8X8_MSG_DELAY_NANO:			// delay arg_int * 1 nano second
      sleep_us(arg_int/1000);
      break;    
    case U8X8_MSG_DELAY_100NANO:		// delay arg_int * 100 nano seconds
      sleep_us(arg_int/10);
      break;
    case U8X8_MSG_DELAY_10MICRO:		// delay arg_int * 10 micro seconds
      sleep_us(arg_int * 10);
      break;
    case U8X8_MSG_DELAY_MILLI:			// delay arg_int * 1 milli second
      sleep_ms(arg_int);
      break;
    case U8X8_MSG_DELAY_I2C:				// arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
      sleep_us(5/arg_int);
      break;							// arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us

    case U8X8_MSG_GPIO_I2C_CLOCK:		// arg_int=0: Output low at I2C clock pin
        if(arg_int) {
            gpio_set_dir(I2C_CLOCK, GPIO_IN);
        } else {
            gpio_set_dir(I2C_CLOCK, GPIO_OUT);
        }
      break;							// arg_int=1: Input dir with pullup high for I2C clock pin
    case U8X8_MSG_GPIO_I2C_DATA:			// arg_int=0: Output low at I2C data pin
        if(arg_int) {
            gpio_set_dir(I2C_DATA, GPIO_IN);
        } else {
            gpio_set_dir(I2C_DATA, GPIO_OUT);
        }
      break;							// arg_int=1: Input dir with pullup high for I2C data pin

    default:
      u8x8_SetGPIOResult(u8x8, 1);
      break;
  }
  return 1;
}

uint8_t u8x8_byte_rp2040_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  static uint8_t buffer[32];		/* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
  static uint8_t buf_idx;
  uint8_t *data;

  switch(msg)
  {
    case U8X8_MSG_BYTE_SEND:
      data = (uint8_t *)arg_ptr;
      while( arg_int > 0 )
      {
        buffer[buf_idx++] = *data;
        data++;
        arg_int--;
      }
      break;
    case U8X8_MSG_BYTE_INIT:
      /* add your custom code to init i2c subsystem */
      break;
    case U8X8_MSG_BYTE_SET_DC:
      /* ignored for i2c */
      break;
    case U8X8_MSG_BYTE_START_TRANSFER:
      buf_idx = 0;
      break;
    case U8X8_MSG_BYTE_END_TRANSFER:
      i2c_write_blocking(i2c1, u8x8_GetI2CAddress(u8x8) >> 1, buffer, buf_idx, false);
      //i2c_transfer(u8x8_GetI2CAddress(u8x8) >> 1, buf_idx, buffer);
      break;
    default:
      return 0;
  }
  return 1;
}
u8g2_uint_t u8g2_DrawStrCenter(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, const char *s) {
  return u8g2_DrawStr(u8g2, x - u8g2_GetStrWidth(u8g2, s)/2, y+u8g2_GetAscent(u8g2)/2, s);
}

u8g2_uint_t u8g2_DrawStrCenter(u8g2_t *u8g2, const char *s) {
  return u8g2_DrawStrCenter(u8g2, u8g2_GetDisplayWidth(u8g2)/2, u8g2_GetDisplayHeight(u8g2)/2, s);
}

u8g2_uint_t u8g2_DrawStrCenter(u8g2_t *u8g2, u8g2_uint_t y, const char *s) {
  return u8g2_DrawStrCenter(u8g2, u8g2_GetDisplayWidth(u8g2)/2, y, s);
}

u8g2_uint_t u8g2_DrawStrCenterH(u8g2_t *u8g2, u8g2_uint_t x, u8g2_uint_t y, const char *s) {
  return u8g2_DrawStr(u8g2, x - u8g2_GetStrWidth(u8g2, s)/2, y, s);
}

#define HW_I2C

class Display {
  public:
    u8g2_t u8g2;
    Display() {
      #ifdef HW_I2C
      i2c_init(i2c1, 2000000);
      gpio_set_function(6, GPIO_FUNC_I2C);
      gpio_set_function(7, GPIO_FUNC_I2C);

      u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_rp2040_hw_i2c, u8x8_gpio_and_delay);  // init u8g2 structure
      u8g2_SetI2CAddress(&u8g2, 0x78);
      #endif
      #ifndef HW_I2C
      gpio_init(I2C_DATA);
      gpio_init(I2C_CLOCK);
      gpio_pull_up(I2C_DATA);
      gpio_pull_up(I2C_CLOCK);
      gpio_set_dir(I2C_DATA, GPIO_IN);
      gpio_set_dir(I2C_CLOCK, GPIO_IN);
      gpio_put(I2C_CLOCK, 0);
      gpio_put(I2C_DATA, 0);

      u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8x8_gpio_and_delay);  // init u8g2 structure
      u8g2_SetI2CAddress(&u8g2, 0x78);
      #endif

      //sleep_ms(25);

      u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
      u8g2_SetPowerSave(&u8g2, 0); // wake up display
    }
};

