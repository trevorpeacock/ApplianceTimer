#include "display.cpp"
#include "buttons.cpp"
#include "menu_base.cpp"
#include "menu_time.cpp"
#include "menu_schedule.cpp"
#include <stdio.h>
#include <math.h>

class SettingsMenu : public GenericMenu {
  public:
    TimeSettingsMenu* timesettings;
    ScheduleListMenu* schedule;
    SettingsMenu(Display* _display, Buttons* _buttons) : GenericMenu(_display, _buttons) {
      timesettings = new TimeSettingsMenu(display, buttons);
      schedule = new ScheduleListMenu(display, buttons);
    }
    virtual void menu_title(char s[]) {
      strcpy(s, "Settings");
    }
    virtual uint8_t item_count() {
      return 3;
    }
    virtual void item_text(uint8_t item, char s[]) {
      switch(item) {
        case 0:
          strcpy (s, "Set Time"); break;
        case 1:
          strcpy (s, "Set Schedule"); break;
        case 2:
          strcpy (s, "Exit"); break;
      }
    }
    virtual uint16_t item_enter(uint8_t item) {
      switch(item) {
        case 0:
          timesettings->init();
          set_submenu(timesettings);
          break;
        case 1:
          schedule->init();
          set_submenu(schedule);
          break;
        case 2:
          return 1;
      }
      return 0;
    }
};

class PowerSaveDisplay : public Menu {
  public:
    uint64_t start_time;
    using Menu::Menu;
    virtual void init() {
      Menu::init();
      start_time = time_us_64();
    }
    virtual void enter() {
      Menu::enter();
      u8g2_SetPowerSave(&display->u8g2, 1); // sleep display
    }
    virtual void exit() {
      u8g2_SetPowerSave(&display->u8g2, 0); // wake up display
      set_leds(4, 4, 3);
      Menu::exit();
    }
    virtual uint8_t run() {
      uint32_t cycle_time = ((time_us_64() - start_time) % 16000000) / 4000;
      if(cycle_time > 2000)
        cycle_time = 0;
      if(cycle_time > 1000)
        cycle_time = 2000 - cycle_time;
      uint16_t brightness = pow((double)(cycle_time * (16)) / 1000, 2);
      if(brightness<3)
        brightness = 3;
      if(brightness>255)
        brightness = 255;
      set_leds(4, 4, brightness);
      return 0;
    }
};

class CentreButtonMessageDisplay : public Menu {
  public:
    using Menu::Menu;
    virtual void draw_message();
    virtual uint8_t run() {
      u8g2_ClearBuffer(&display->u8g2);
      u8g2_DrawTriangle(&display->u8g2, 64, 62, 64-10, 62-10, 64+10, 62-10);
      u8g2_DrawBox(&display->u8g2, 64-4, 62-10-10, 8, 10);
      draw_message();
      u8g2_SendBuffer(&display->u8g2);
      return 0;
    }
};

class PressOnMessageDisplay : public CentreButtonMessageDisplay {
  public:
    using CentreButtonMessageDisplay::CentreButtonMessageDisplay;
    virtual void draw_message() {
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB14_tr);
      u8g2_DrawStrCenter(&display->u8g2, 16-8, "Press to");
      u8g2_DrawStrCenter(&display->u8g2, 16+8, "Turn On");
    }
};

class PressLongerMessageDisplay : public CentreButtonMessageDisplay {
  public:
    using CentreButtonMessageDisplay::CentreButtonMessageDisplay;
    virtual void draw_message() {
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB14_tr);
      u8g2_DrawStrCenter(&display->u8g2, 16-8, "Press to");
      u8g2_DrawStrCenter(&display->u8g2, 16+8, "Add Time");
    }
};

class HoldOffMessageDisplay : public CentreButtonMessageDisplay {
  public:
    using CentreButtonMessageDisplay::CentreButtonMessageDisplay;
    virtual void draw_message() {
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB14_tr);
      u8g2_DrawStrCenter(&display->u8g2, 16-8, "Hold to");
      u8g2_DrawStrCenter(&display->u8g2, 16+8, "Turn Off");
    }
};

class OutsideButtonsMessageDisplay : public Menu {
  public:
    using Menu::Menu;
    virtual void draw_message();
    virtual uint8_t run() {
      u8g2_ClearBuffer(&display->u8g2);
      u8g2_DrawTriangle(&display->u8g2, 14, 62, 14-10, 62-10, 14+10, 62-10);
      u8g2_DrawBox(&display->u8g2, 14-2, 62-10-10, 4, 10);
      u8g2_DrawTriangle(&display->u8g2, 113, 62, 113-10, 62-10, 113+10, 62-10);
      u8g2_DrawBox(&display->u8g2, 113-2, 62-10-10, 4, 10);
      u8g2_DrawBox(&display->u8g2, 14-2, 62-10-10-8+8, 127-28+4, 4);
      draw_message();
      u8g2_SendBuffer(&display->u8g2);
      return 0;
    }
};

class PressSettingsMessageDisplay : public OutsideButtonsMessageDisplay {
  public:
    using OutsideButtonsMessageDisplay::OutsideButtonsMessageDisplay;
    virtual void draw_message() {
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB14_tr);
      u8g2_DrawStrCenter(&display->u8g2, 16-8, "Press both");
      u8g2_DrawStrCenter(&display->u8g2, 16+8, "for Settings");
    }
};

class HourDisplay : public Menu {
  public:
    using Menu::Menu;
    uint16_t totaltime;
    char message[10];
    virtual void init(uint16_t _totaltime) {
      Menu::init();
      totaltime = _totaltime;
    }
    uint8_t run() {
      u8g2_ClearBuffer(&display->u8g2);
      u8g2_SetFont(&display->u8g2, u8g2_font_fub14_tr);
      u8g2_DrawStrCenter(&display->u8g2, 8, "Turn off in");
      u8g2_SetFont(&display->u8g2, u8g2_font_fub42_tr);
      sprintf(message, "%dh", totaltime);
      u8g2_DrawStrCenter(&display->u8g2, 40, message);
      u8g2_SendBuffer(&display->u8g2);
      return 0;
    }
};

class IncreaseTimer : public CycleMenus {
  public:
    HourDisplay* hourdisplay;
    CentreButtonMessageDisplay* messagedisplay1;
    CentreButtonMessageDisplay* messagedisplay2;
    uint64_t start_time;
    uint8_t state;
    IncreaseTimer(Display* _display, Buttons* _buttons) : CycleMenus(_display, _buttons) {
      hourdisplay = new HourDisplay(display, buttons);
      messagedisplay1 = new PressLongerMessageDisplay(display, buttons);
      messagedisplay2 = new HoldOffMessageDisplay(display, buttons);
    }
    virtual void init(uint16_t _totaltime) {
      CycleMenus::init();
      start_time = time_us_64();
      state = 0;
      hourdisplay->init(_totaltime);
    }
    virtual Menu* get_menu() {
      if(time_us_64() - start_time > 1000000 * 2.5) {
        start_time = time_us_64();
        state = state + 1;
      }
      if(state>3)
        state = 3;
      switch(state) {
        case 0:
          return hourdisplay;
        case 1:
          return messagedisplay1;
        case 2:
          return messagedisplay2;
        default:
          return NULL;
      }
    }
    virtual void leave() {
      CycleMenus::leave();
      current_submenu->init();
    }
    virtual uint8_t run() {
      return 0;
    }
};

class TimerDisplay : public ParentMenu {
  public:
    IncreaseTimer* increasetimerdisplay;
    TimerDisplay(Display* _display, Buttons* _buttons) : ParentMenu(_display, _buttons) {
      increasetimerdisplay = new IncreaseTimer(display, buttons);
    }
    virtual void init(uint16_t _totaltime) {
      ParentMenu::init();
      increasetimerdisplay->init(_totaltime);
      set_submenu(increasetimerdisplay);
    }
    virtual uint16_t check_override() {
      int button_c = buttons->centre_button_actions->get_last_state();
      if(button_c==ButtonActions::ButtonActionsState::bas_pressed) {
        uint16_t time = timer->add_time(SECONDS_IN_HOUR) / SECONDS_IN_HOUR;
        increasetimerdisplay->init(time);
        if(current_submenu!=increasetimerdisplay) {
          set_submenu(increasetimerdisplay);
        }
      }
      if(button_c==ButtonActions::ButtonActionsState::bas_holding) {
        timer->set_timer(0);
        return 1;
      }
      return ParentMenu::check_override();
    }
    virtual uint16_t check_buttons() {
      if(timer->time_remaining_s()==0) {
        return 1;
      }
      return 0;
    }
    virtual uint8_t run() {
      time_data current_time = timer->time_remaining();
      char timeformat[14];
      u8g2_ClearBuffer(&display->u8g2);

      u8g2_SetFont(&display->u8g2, u8g2_font_timR08_tf);
      time_format(timeformat, get_time());
      u8g2_DrawStrCenter(&display->u8g2, 40, 8, timeformat);
      u8g2_SetFont(&display->u8g2, u8g2_font_timR08_tf);
      date_format(timeformat, get_date());
      u8g2_DrawStrCenter(&display->u8g2, 88, 8, day_of_week_common_patterns[get_date().day_of_week].day_name);
      u8g2_SetFont(&display->u8g2, u8g2_font_timR08_tf);
      date_format(timeformat, get_date());
      u8g2_DrawStrCenter(&display->u8g2, 18, timeformat);

      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB14_tr);
      u8g2_DrawStrCenter(&display->u8g2, 30, "Turn off in");
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB24_tr);
      countdown_format(timeformat, current_time);
      u8g2_DrawStrCenter(&display->u8g2, 52, timeformat);
      u8g2_SendBuffer(&display->u8g2);
      return 0;
    }
};

class InstructionDisplay : public CycleMenus {
  public:
    CentreButtonMessageDisplay* messagedisplay1;
    CentreButtonMessageDisplay* messagedisplay2;
    OutsideButtonsMessageDisplay* messagedisplay3;
    uint64_t start_time;
    uint8_t state;
    InstructionDisplay(Display* _display, Buttons* _buttons) : CycleMenus(_display, _buttons) {
      messagedisplay1 = new PressOnMessageDisplay(display, buttons);
      messagedisplay2 = new HoldOffMessageDisplay(display, buttons);
      messagedisplay3 = new PressSettingsMessageDisplay(display, buttons);
    }
    virtual void init() {
      CycleMenus::init();
      start_time = time_us_64();
      state = 0;
    }
    virtual Menu* get_menu() {
      if(time_us_64() - start_time > 1000000LL * 2.5) {
        start_time = time_us_64();
        state = state + 1;
      }
      switch(state) {
        case 0:
          return messagedisplay1;
        case 1:
          return messagedisplay2;
        case 2:
          return messagedisplay3;
        default:
          return NULL;
      }
    }
    virtual uint16_t check_override() {
      int button_dual = buttons->dual_buttons->get_last_state();
      if(button_dual!=DualButtonActions::DualButtonActionsState::dbas_none) {
        return 2;
      }
      return CycleMenus::check_override();
    };
    virtual void leave() {
      CycleMenus::leave();
      current_submenu->init();
    }
    virtual uint8_t run() {
      return 0;
    }
};

class MainDisplay : public ParentMenu {
  public:
    TimerDisplay* timerdisplay;
    SettingsMenu* settings;
    InstructionDisplay* instructions;
    bool auto_enter_settings;
    uint64_t start_time;
    MainDisplay(Display* _display, Buttons* _buttons) : ParentMenu(_display, _buttons) {
      timerdisplay = new TimerDisplay(display, buttons);
      settings = new SettingsMenu(display, buttons);
      instructions = new InstructionDisplay(display, buttons);
      auto_enter_settings = false;
    }
    virtual void init() {
      ParentMenu::init();
      start_time = time_us_64();
    }
    virtual uint16_t check_override() {
      int button_l = buttons->left_button_actions->get_last_state();
      int button_c = buttons->centre_button_actions->get_last_state();
      int button_r = buttons->right_button_actions->get_last_state();
      int button_dual = buttons->dual_buttons->get_last_state();
      if(button_l==ButtonActions::ButtonActionsState::bas_pressed) {
        if(current_submenu==NULL || current_submenu==timerdisplay) {
          instructions->init();
          set_submenu(instructions);
        }
      }
      if(button_r==ButtonActions::ButtonActionsState::bas_pressed) {
        if(current_submenu==NULL || current_submenu==timerdisplay) {
          instructions->init();
          set_submenu(instructions);
        }
      }
      if(button_c==ButtonActions::ButtonActionsState::bas_pressed) {
        if(current_submenu==NULL || current_submenu==instructions) {
          //uint16_t time = timer->add_time(SECONDS_IN_HOUR) / SECONDS_IN_HOUR;
          //because timerdisplay also uses check_override, it will see button event
          timerdisplay->init(0);
          set_submenu(timerdisplay);
        }
      }
      if(button_dual==DualButtonActions::DualButtonActionsState::dbas_pressed) {
        if(current_submenu!=settings) {
          settings->init();
          set_submenu(settings);
        }
      }
      if((timer->time_remaining_s()) && (time_us_64() - start_time) % 1000000 < 500000) {
        set_leds(4, 16, 3);
      } else {
        set_leds(4, 4, 3);
      }
      return ParentMenu::check_override();
    }
    virtual void enter() {
      ParentMenu::enter();
      if(auto_enter_settings) {
        settings->init();
        set_submenu(settings);
        return;
      }
      if(timer->time_remaining_s()) {
        set_submenu(timerdisplay);
      }
    }
    virtual void return_to(Menu* from_submenu, uint16_t retval) {
      if(timer->time_remaining_s()) {
        set_submenu(timerdisplay);
      }
      ParentMenu::return_to(from_submenu, retval);
    }
    uint8_t debug_run() {
      u8g2_ClearBuffer(&display->u8g2);
      u8g2_SetFont(&display->u8g2, u8g2_font_timR08_tf);

      char s[100];
      rtc_data rtc;
      ds1302->read(0xBF, (uint8_t*)&rtc, 8);

      sprintf(s, "%02X %02X %02X %02X %02X %02X %02X %02X",
        rtc.second,
        rtc.minute,
        rtc.hour,
        rtc.date,
        rtc.month,
        rtc.day,
        rtc.year,
        rtc.wp
      );
      u8g2_DrawStr(&display->u8g2, 0, 8, s);

      u8g2_SendBuffer(&display->u8g2);
      return 0;


      schedule_calc_data schedules_temp[MAX_SCHEDULES];
      build_next_schedule(schedules_temp);
      u8g2_ClearBuffer(&display->u8g2);
      u8g2_SetFont(&display->u8g2, u8g2_font_timR08_tf);

      sprintf(s, "%d-%d-%d %02d:%02d:%02d %d",
        current_datetime.date.year, current_datetime.date.month, current_datetime.date.day,
        current_datetime.time.hour, current_datetime.time.minute, current_datetime.time.second,
        current_datetime.date.day_of_week
      );
      u8g2_DrawStr(&display->u8g2, 0, 8, s);

      for(uint8_t i = 0; i < storage->data->schedule_count; i++) {
        sprintf(s, "%d-%d-%d %02d:%02d:%02d %d",
          schedules_temp[i].datetime.date.year, schedules_temp[i].datetime.date.month, schedules_temp[i].datetime.date.day,
          schedules_temp[i].datetime.time.hour, schedules_temp[i].datetime.time.minute, schedules_temp[i].datetime.time.second,
          schedules_temp[i].datetime.date.day_of_week
        );
        u8g2_DrawStr(&display->u8g2, 0, 28 + 10*i, s);
      }

      u8g2_SendBuffer(&display->u8g2);
      return 0;
    }
    virtual uint8_t run() {
      if(timer->time_remaining_s()) {
        set_submenu(timerdisplay);
        return 0;
      }
      //return debug_run();
      time_data current_time = timer->time_remaining();
      char timeformat[14];
      u8g2_ClearBuffer(&display->u8g2);
      u8g2_SetFont(&display->u8g2, u8g2_font_timR10_tf);
      date_format(timeformat, get_date());
      u8g2_DrawStrCenter(&display->u8g2, 8, day_of_week_common_patterns[get_date().day_of_week].day_name);
      u8g2_SetFont(&display->u8g2, u8g2_font_timR10_tf);
      date_format(timeformat, get_date());
      u8g2_DrawStrCenter(&display->u8g2, 24, timeformat);
      if(storage->data->h24format) {
        u8g2_SetFont(&display->u8g2, u8g2_font_timR24_tf);
      } else {
        u8g2_SetFont(&display->u8g2, u8g2_font_timR18_tf);
      }
      time_format(timeformat, get_time());
      u8g2_DrawStrCenter(&display->u8g2, 48, timeformat);
      u8g2_SendBuffer(&display->u8g2);
      return 0;
    }
};

class SleepDisplay : public CycleMenus {
  public:
    MainDisplay* mainmenu;
    PowerSaveDisplay* sleep;
    uint64_t start_time;
    SleepDisplay(Display* _display, Buttons* _buttons) : CycleMenus(_display, _buttons) {
      mainmenu = new MainDisplay(display, buttons);
      sleep = new PowerSaveDisplay(display, buttons);
    }
    virtual void init() {
      CycleMenus::init();
      start_time = time_us_64();
    }
    virtual Menu* get_menu() {
      if(time_us_64() - start_time > 1000000LL*60*5) {
        return sleep;
      }
      return mainmenu;
    }
    virtual uint16_t check_override() {
      int button_l = buttons->left_button_actions->get_last_state();
      int button_c = buttons->centre_button_actions->get_last_state();
      int button_r = buttons->right_button_actions->get_last_state();
      int button_dual = buttons->dual_buttons->get_last_state();
      if(button_l!=ButtonActions::ButtonActionsState::bas_none) {
        start_time = time_us_64();
      }
      if(button_c!=ButtonActions::ButtonActionsState::bas_none) {
        start_time = time_us_64();
      }
      if(button_r!=ButtonActions::ButtonActionsState::bas_none) {
        start_time = time_us_64();
      }
      if(button_dual!=DualButtonActions::DualButtonActionsState::dbas_none) {
        start_time = time_us_64();
      }
      if(timer->time_remaining_s()) {
        start_time = time_us_64();
        gpio_put(RELAY_PIN, 1);
      } else {
        gpio_put(RELAY_PIN, 0);
      }
      return CycleMenus::check_override();
    };
    virtual void leave() {
      current_submenu->init();
    }
    virtual uint8_t run() {
      return 0;
    }
};

class WelcomeDisplay : public CycleMenus {
  public:
    InstructionDisplay* instructions;
    SleepDisplay* menu;
    uint8_t state;
    WelcomeDisplay(Display* _display, Buttons* _buttons) : CycleMenus(_display, _buttons) {
      instructions = new InstructionDisplay(display, buttons);
      menu = new SleepDisplay(display, buttons);
    }
    virtual void init() {
      CycleMenus::init();
      state = 0;
    }
    virtual Menu* get_menu() {
      switch(state) {
        case 0:
          return instructions;
        default:
          return menu;
      }
    }
    virtual void leave() {
      CycleMenus::leave();
      current_submenu->init();
    }
    virtual uint16_t check_child(uint16_t submenu_result) { // called on every menu with an active child
      if(submenu_result==2) {
        menu->mainmenu->auto_enter_settings = true;
      }
      return CycleMenus::check_child(submenu_result);
    };
    virtual void return_to(Menu* from_submenu, uint16_t retval) {
      state++;
      CycleMenus::return_to(from_submenu, retval);
    }
    virtual uint8_t run() {
      return 0;
    }
};
