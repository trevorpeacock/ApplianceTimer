class TimeEditMenu : public MultiEditMenu {
  public:
    time_data data;
    bool seconds_edited;
    TimeEditMenu(Display* _display, Buttons* _buttons) : MultiEditMenu(_display, _buttons) {
    }
    virtual void init() {
      MultiEditMenu::init();
      data = current_datetime.time;
      seconds_edited = false;
    }
    virtual uint8_t element_count() {
      if(storage->data->h24format)
        return 3;
      return 4;
    }
    virtual bool allow_holding(uint8_t e) {
      return e!=0;
    };
    virtual void draw_background() {
      if(!seconds_edited) {
        data.second = current_datetime.time.second;
      }
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB14_tr);
      if(storage->data->h24format) {
        u8g2_DrawStrCenter(&display->u8g2, 48, 24, ":");
        u8g2_DrawStrCenter(&display->u8g2, 80, 24, ":");
      } else {
        u8g2_DrawStrCenter(&display->u8g2, 29, 24, ":");
        u8g2_DrawStrCenter(&display->u8g2, 61, 24, ":");
      }
    }
    virtual void draw_element(uint8_t e, bool selected) {
      char s[11];
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB14_tr);
      switch(e) {
        case 0:
          if(storage->data->h24format) {
            sprintf(s, "%02d", data.hour);
            u8g2_SetDrawColor(&display->u8g2, 1);
            if(selected) u8g2_DrawBox(&display->u8g2, 19, 15, 25, 18);
            u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
            u8g2_DrawStrCenter(&display->u8g2, 32, 24, s);
          } else {
            sprintf(s, "%d", ((data.hour+11)%12)+1);
            u8g2_SetDrawColor(&display->u8g2, 1);
            if(selected) u8g2_DrawBox(&display->u8g2, 0, 15, 25, 18);
            u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
            u8g2_DrawStrCenter(&display->u8g2, 13, 24, s);
          }
          break;
        case 1:
          if(storage->data->h24format) {
            sprintf(s, "%02d", data.minute);
            u8g2_SetDrawColor(&display->u8g2, 1);
            if(selected) u8g2_DrawBox(&display->u8g2, 51, 15, 25, 18);
            u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
            u8g2_DrawStrCenter(&display->u8g2, 64, 24, s);
          } else {
            sprintf(s, "%02d", data.minute);
            u8g2_SetDrawColor(&display->u8g2, 1);
            if(selected) u8g2_DrawBox(&display->u8g2, 32, 15, 25, 18);
            u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
            u8g2_DrawStrCenter(&display->u8g2, 45, 24, s);
          }
          break;
        case 2:
          if(storage->data->h24format) {
            sprintf(s, "%02d", data.second);
            u8g2_SetDrawColor(&display->u8g2, 1);
            if(selected) u8g2_DrawBox(&display->u8g2, 83, 15, 25, 18);
            u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
            u8g2_DrawStrCenter(&display->u8g2, 96, 24, s);
          } else {
            sprintf(s, "%02d", data.second);
            u8g2_SetDrawColor(&display->u8g2, 1);
            if(selected) u8g2_DrawBox(&display->u8g2, 64, 15, 25, 18);
            u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
            u8g2_DrawStrCenter(&display->u8g2, 77, 24, s);
          }
          break;
        case 3:
          sprintf(s, (data.hour<12) ? "am" : "pm");
          u8g2_SetDrawColor(&display->u8g2, 1);
          if(selected) u8g2_DrawBox(&display->u8g2, 93, 15, 33, 22);
          u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
          u8g2_DrawStrCenter(&display->u8g2, 110, 24, s);
          break;
      }
      u8g2_SetDrawColor(&display->u8g2, 1);
    }
    virtual void change_element(uint8_t e, int8_t change) {
      switch(e) {
        case 0:
          timeadd(&data, change, 0, 0, false);
          break;
        case 1:
          timeadd(&data, 0, change, 0, false);
          break;
        case 2:
          if(change!=0) seconds_edited = true;
          timeadd(&data, 0, 0, change, false);
          break;
        case 3:
          timeadd(&data, change*12, 0, 0, false);
          break;
      }
    }
    virtual void exit() {
      current_datetime.time = data;
      ds1302->write_time();
      ds1302->check_dst(false);
      MultiEditMenu::exit();
    }
};

class DateEditMenu : public MultiEditMenu {
  public:
    date_data data;
    DateEditMenu(Display* _display, Buttons* _buttons) : MultiEditMenu(_display, _buttons) {
    }
    virtual void init() {
      MultiEditMenu::init();
      data = current_datetime.date;
    }
    virtual uint8_t element_count() {
      return 4;
    }
    virtual bool allow_holding(uint8_t e) {
      return e!=0;
    };
    virtual void draw_background() {
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB14_tr);
      u8g2_DrawStrCenter(&display->u8g2, 52, 36, "-");
      u8g2_DrawStrCenter(&display->u8g2, 101, 36, "-");
    }
    virtual void draw_element(uint8_t e, bool selected) {
      char s[11];
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB14_tr);
      switch(e) {
        case 0:
          u8g2_SetDrawColor(&display->u8g2, 1);
          if(selected) u8g2_DrawBox(&display->u8g2, 5, 3, 118, 21);
          u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
          u8g2_DrawStrCenter(&display->u8g2, 64, 12, day_of_week_common_patterns[data.day_of_week].day_name);
          break;
        case 1:
          sprintf(s, "%02d", data.year);
          u8g2_SetDrawColor(&display->u8g2, 1);
          if(selected) u8g2_DrawBox(&display->u8g2, 0, 27, 49, 18);
          u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
          u8g2_DrawStrCenter(&display->u8g2, 23, 36, s);
          break;
        case 2:
          u8g2_SetDrawColor(&display->u8g2, 1);
          if(selected) u8g2_DrawBox(&display->u8g2, 56, 27, 42, 21);
          u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
          u8g2_DrawStrCenter(&display->u8g2, 77, 36, month_names[data.month]);
          break;
        case 3:
          if(selected && data.day>=days_in_month(data.month, data.year))
            data.day = days_in_month(data.month, data.year)-1;
          sprintf(s, "%02d", data.day+1);
          u8g2_SetDrawColor(&display->u8g2, 1);
          if(selected) u8g2_DrawBox(&display->u8g2, 106, 27, 22, 18);
          u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
          u8g2_DrawStrCenter(&display->u8g2, 117, 36, s);
          break;
      }
      u8g2_SetDrawColor(&display->u8g2, 1);
    }
    virtual void change_element(uint8_t e, int8_t change) {
      int8_t dow;
      switch(e) {
        case 0:
          dow = data.day_of_week;
          dow = dow + change;
          if(dow>=7)
            dow = 0;
          if(dow<0)
            dow = 6;
          data.day_of_week = dow;
          break;
        case 1:
          dateadd(&data, change, 0, 0);
          break;
        case 2:
          dateadd(&data, 0, change, 0);
          break;
        case 3:
          dateadd(&data, 0, 0, change);
          break;
      }
    }
    virtual void exit() {
      current_datetime.date = data;
      ds1302->write_time();
      ds1302->check_dst(false);
      MultiEditMenu::exit();
    }
};

class DSTDateEditMenu : public MultiEditMenu {
  public:
    dst_change_schedule* stored_schedule;
    dst_change_schedule schedule;
    DSTDateEditMenu(Display* _display, Buttons* _buttons) : MultiEditMenu(_display, _buttons) {
    }
    virtual void init(dst_change_schedule* _schedule) {
      MultiEditMenu::init();
      stored_schedule = _schedule;
      schedule = *stored_schedule;
    }
    virtual uint8_t element_count() {
      return 2;
    }
    virtual bool allow_holding(uint8_t e) {
      return false;
    };
    virtual void draw_background() {
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB10_tr);
      u8g2_DrawStrCenter(&display->u8g2, 96, 13, "Sunday");
      u8g2_DrawStrCenter(&display->u8g2, 48, 39, "in");
    }
    virtual void draw_element(uint8_t e, bool selected) {
      char s[11];
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB10_tr);
      switch(e) {
        case 0:
          u8g2_SetDrawColor(&display->u8g2, 1);
          if(selected) u8g2_DrawBox(&display->u8g2, 0, 5, 64, 15);
          u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
          nth_day_format(s, schedule.nth_sunday);
          u8g2_DrawStrCenter(&display->u8g2, 32, 13, s);
          break;
        case 1:
          u8g2_SetDrawColor(&display->u8g2, 1);
          if(selected) u8g2_DrawBox(&display->u8g2, 58, 31, 36, 18);
          u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
          strcpy(s, month_names[schedule.month]);
          u8g2_DrawStrCenter(&display->u8g2, 76, 39, s);
          break;
      }
      u8g2_SetDrawColor(&display->u8g2, 1);
    }
    virtual void change_element(uint8_t e, int8_t change) {
      switch(e) {
        case 0:
          schedule.nth_sunday += change;
          if(schedule.nth_sunday == 0)
            schedule.nth_sunday += change;
          if(schedule.nth_sunday < -4)
            schedule.nth_sunday = 4;
          if(schedule.nth_sunday > 4)
            schedule.nth_sunday = -4;
          break;
        case 1:
          schedule.month += 12 + change;
          if((schedule.month % 12) == storage->data->dst_enter.month || (schedule.month % 12) == storage->data->dst_exit.month)
            schedule.month += change;
          schedule.month = schedule.month % 12;
          break;
      }
    }
    virtual void exit() {
      *stored_schedule = schedule;
      ds1302->write_time();
      ds1302->check_dst(true);
      storage->store();
      MultiEditMenu::exit();
    }
};

class DSTScheduleMenu : public GenericMenu {
  public:
    DSTDateEditMenu* dst_edit;
    DSTScheduleMenu(Display* _display, Buttons* _buttons) : GenericMenu(_display, _buttons) {
      dst_edit = new DSTDateEditMenu(_display, _buttons);
    }
    virtual void init() {
      GenericMenu::init();
    }
    virtual void menu_title(char s[]) {
      strcpy(s, "DST Schedule");
    }
    virtual uint8_t item_count() {
      if(!storage->data->dst_schedule_enabled)
        return 2;
      return 4;
    }
    virtual void item_text(uint8_t item, char s[]) {
      char tmp[50];
      switch(item) {
        case 0:
          if(storage->data->dst_schedule_enabled) {
            strcpy (s, "Auto DST: On");
          } else {
            strcpy (s, "Auto DST: Off");
          }
          break;
        case 1:
          if(!storage->data->dst_schedule_enabled) {
            strcpy (s, "Exit");
          } else {
            dst_change_format(tmp, storage->data->dst_enter.nth_sunday, storage->data->dst_enter.month);
            sprintf(s, "Start: %s", tmp);
          }
          break;
        case 2:
          dst_change_format(tmp, storage->data->dst_exit.nth_sunday, storage->data->dst_exit.month);
          sprintf(s, "End: %s", tmp);
          break;
        case 3:
          strcpy (s, "Exit"); break;
      }
    }
    virtual uint16_t item_enter(uint8_t item) {
      switch(item) {
        case 0:
          storage->data->dst_schedule_enabled = !storage->data->dst_schedule_enabled;
          break;
        case 1:
          if(!storage->data->dst_schedule_enabled)
            return 1;
          dst_edit->init(&storage->data->dst_enter);
          set_submenu(dst_edit);
          break;
        case 2:
          dst_edit->init(&storage->data->dst_exit);
          set_submenu(dst_edit);
          break;
        case 3:
          return 1;
      }
      return 0;
    }
    virtual void exit() {
      storage->store();
      ds1302->check_dst(true);
      GenericMenu::exit();
    }
};

class DSTSettingsMenu : public GenericMenu {
  public:
    DSTScheduleMenu* dstschedule;
    DSTSettingsMenu(Display* _display, Buttons* _buttons) : GenericMenu(_display, _buttons) {
      dstschedule = new DSTScheduleMenu(display, buttons);
    }
    virtual void init() {
      GenericMenu::init();
    }
    virtual void menu_title(char s[]) {
      strcpy(s, "DST Settings");
    }
    virtual uint8_t item_count() {
      return 3;
    }
    virtual void item_text(uint8_t item, char s[]) {
      switch(item) {
        case 0:
          if(storage->data->in_dst) {
            strcpy (s, "DST: On");
          } else {
            strcpy (s, "DST: Off");
          }
          break;
        case 1:
          strcpy (s, "DST Schedule"); break;
        case 2:
          strcpy (s, "Exit"); break;
      }
    }
    virtual uint16_t item_enter(uint8_t item) {
      switch(item) {
        case 0:
          storage->data->in_dst = !storage->data->in_dst;
          timeadd(&current_datetime.time, storage->data->in_dst ? 1 : -1, 0, 0, true);
          ds1302->write_time();
          storage->store();
          ds1302->check_dst(false);
          break;
        case 1:
          dstschedule->init();
          set_submenu(dstschedule);
          break;
        case 2:
          return 1;
      }
      return 0;
    }
    virtual void exit() {
      GenericMenu::exit();
    }
};

class TimeSettingsMenu : public GenericMenu {
  DSTSettingsMenu* dstmenu;
  TimeEditMenu* time_edit;
  DateEditMenu* date_edit;
  bool h24format;
  public:
    TimeSettingsMenu(Display* _display, Buttons* _buttons) : GenericMenu(_display, _buttons) {
      dstmenu = new DSTSettingsMenu(display, buttons);
      time_edit = new TimeEditMenu(display, buttons);
      date_edit = new DateEditMenu(display, buttons);
    }
    virtual void init() {
      GenericMenu::init();
      h24format = true;
    }
    virtual void menu_title(char s[]) {
      strcpy(s, "Clock Settings");
    }
    virtual uint8_t item_count() {
      return 5;
    }
    virtual void item_text(uint8_t item, char s[]) {
      switch(item) {
        case 0:
          strcpy (s, "Change Time"); break;
        case 1:
          strcpy (s, "Change Date"); break;
        case 2:
          strcpy (s, "Set DST"); break;
        case 3:
          if(storage->data->h24format) {
            strcpy (s, "Format: 24h"); break;
          } else {
            strcpy (s, "Format: 12h"); break;
          }
        case 4:
          strcpy (s, "Exit"); break;
      }
    }
    virtual uint16_t item_enter(uint8_t item) {
      switch(item) {
        case 0:
          time_edit->init();
          set_submenu(time_edit);
          break;
        case 1:
          date_edit->init();
          set_submenu(date_edit);
          break;
        case 2:
          dstmenu->init();
          set_submenu(dstmenu);
          break;
        case 3:
          storage->data->h24format = !storage->data->h24format;
          break;
        case 4:
          return 1;
      }
      return 0;
    }
    virtual void return_to(Menu* from_submenu, uint16_t retval) {
      GenericMenu::return_to(from_submenu, retval);
    }
    virtual void exit() {
      storage->store();
      ds1302->check_dst(true);
      GenericMenu::exit();
    }
};
