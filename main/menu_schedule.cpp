class ScheduleDOWEditMenu : public MultiEditMenu {
  public:
    schedule_data* data;
    int8_t dow;
    ScheduleDOWEditMenu(Display* _display, Buttons* _buttons) : MultiEditMenu(_display, _buttons) {
    }
    virtual void init(schedule_data* d) {
      MultiEditMenu::init();
      data = d;
      dow = dow_pattern_match(data->day_of_week);
    }
    virtual uint8_t element_count() {
      if(dow == -1)
        return 8;
      return 1;
    }
    virtual bool allow_holding(uint8_t e) {
      return true;
    };
    virtual void draw_background() {
    }
    virtual void draw_element(uint8_t e, bool selected) {
      char s[11];
      switch(e) {
        case 0:
          if(dow==-1) {
            strcpy(s, "Custom");
          } else {
            strcpy(s, day_of_week_common_patterns[dow].dow_name);
          }
          u8g2_SetFont(&display->u8g2, u8g2_font_ncenB14_tr);
          u8g2_SetDrawColor(&display->u8g2, 1);
          if(selected) u8g2_DrawBox(&display->u8g2, 2, 3, 123, 21);
          u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
          u8g2_DrawStrCenter(&display->u8g2, 64, 12, s);
          break;
        default:
          u8g2_SetFont(&display->u8g2, u8g2_font_ncenB08_tr);
          u8g2_SetDrawColor(&display->u8g2, 1);
          if(selected) u8g2_DrawTriangle(&display->u8g2, 8 + (e-1)*16, 50, 8 + (e)*16, 50, (e)*16, 42);
          if(data->day_of_week & (0x80 >> e)) u8g2_DrawBox(&display->u8g2, 8 + (e-1)*16 + 1, 24, 15, 16);
          u8g2_SetDrawColor(&display->u8g2, data->day_of_week & 0x80 >> e ? 0 : 1);
          u8g2_DrawStrCenter(&display->u8g2, 16 + (e-1)*16, 32, day_names_short[e-1]);
          break;
      }
      u8g2_SetDrawColor(&display->u8g2, 1);
    }
    virtual void change_element(uint8_t e, int8_t change) {
      int8_t m = sizeof(day_of_week_common_patterns)/sizeof(day_of_week_common_patterns[0]);
      switch(e) {
        case 0:
          dow += change;
          if(dow >= m) {
            dow = -1;
          }
          if(dow < -1) {
            dow = m - 1;
          }
          break;
        default:
          if(change!=0)
            data->day_of_week ^= (0x80 >> e);
          break;
      }
    }
    virtual void exit() {
      if(dow>-1)
        data->day_of_week = day_of_week_common_patterns[dow].dow;
      MultiEditMenu::exit();
    }
};

class ScheduleTimeEditMenu : public MultiEditMenu {
  public:
    schedule_data* data;
    ScheduleTimeEditMenu(Display* _display, Buttons* _buttons) : MultiEditMenu(_display, _buttons) {
    }
    virtual void init(schedule_data* d) {
      MultiEditMenu::init();
      data = d;
    }
    virtual uint8_t element_count() {
      if(storage->data->h24format)
        return 2;
      return 3;
    }
    virtual bool allow_holding(uint8_t e) {
      return e!=0;
    };
    virtual void draw_background() {
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB14_tr);
      if(storage->data->h24format) {
        u8g2_DrawStrCenter(&display->u8g2, 64, 24, ":");
      } else {
        u8g2_DrawStrCenter(&display->u8g2, 46, 24, ":");
      }
    }
    virtual void draw_element(uint8_t e, bool selected) {
      char s[11];
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB14_tr);
      switch(e) {
        case 0:
          if(storage->data->h24format) {
            sprintf(s, "%02d", data->hour);
            u8g2_SetDrawColor(&display->u8g2, 1);
            if(selected) u8g2_DrawBox(&display->u8g2, 34, 15, 25, 18);
            u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
            u8g2_DrawStrCenter(&display->u8g2, 47, 24, s);
          } else {
            sprintf(s, "%d", ((data->hour+11)%12)+1);
            u8g2_SetDrawColor(&display->u8g2, 1);
            if(selected) u8g2_DrawBox(&display->u8g2, 17, 15, 25, 18);
            u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
            u8g2_DrawStrCenter(&display->u8g2, 30, 24, s);
          }
          break;
        case 1:
          if(storage->data->h24format) {
            sprintf(s, "%02d", data->minute);
            u8g2_SetDrawColor(&display->u8g2, 1);
            if(selected) u8g2_DrawBox(&display->u8g2, 68, 15, 25, 18);
            u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
            u8g2_DrawStrCenter(&display->u8g2, 81, 24, s);
          } else {
            sprintf(s, "%02d", data->minute);
            u8g2_SetDrawColor(&display->u8g2, 1);
            if(selected) u8g2_DrawBox(&display->u8g2, 49, 15, 25, 18);
            u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
            u8g2_DrawStrCenter(&display->u8g2, 62, 24, s);
          }
          break;
        case 2:
          sprintf(s, (data->hour<12) ? "am" : "pm");
          u8g2_SetDrawColor(&display->u8g2, 1);
          if(selected) u8g2_DrawBox(&display->u8g2, 76, 15, 33, 22);
          u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
          u8g2_DrawStrCenter(&display->u8g2, 93, 24, s);
          break;
      }
      u8g2_SetDrawColor(&display->u8g2, 1);
    }
    virtual void change_element(uint8_t e, int8_t change) {
      switch(e) {
        case 0:
          timeadd(data, change, 0, false);
          break;
        case 1:
          timeadd(data, 0, change, false);
          break;
        case 2:
          timeadd(data, change*12, 0, false);
          break;
      }
    }
    virtual void exit() {
      MultiEditMenu::exit();
    }
};

class ScheduleDurationEditMenu : public MultiEditMenu {
  public:
    schedule_data* data;
    ScheduleDurationEditMenu(Display* _display, Buttons* _buttons) : MultiEditMenu(_display, _buttons) {
    }
    virtual void init(schedule_data* d) {
      MultiEditMenu::init();
      data = d;
    }
    virtual uint8_t element_count() {
      return 1;
    }
    virtual bool allow_holding(uint8_t e) {
      return true;
    };
    virtual void draw_background() {
    }
    virtual void draw_element(uint8_t e, bool selected) {
      char s[11];
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB14_tr);
      duration_format(s, data);
      u8g2_SetDrawColor(&display->u8g2, 1);
      u8g2_DrawBox(&display->u8g2, 13, 15, 101, 18);
      u8g2_SetDrawColor(&display->u8g2, 0);
      u8g2_DrawStrCenter(&display->u8g2, 24, s);
      u8g2_SetDrawColor(&display->u8g2, 1);
    }
    virtual void change_element(uint8_t e, int8_t change) {
      int16_t d = data->duration;
      d+= change*15*60;
      if(d<15*60)
        d = 15*60;
      if(d>36000)
        d = 36000;
      data->duration = d;
    }
    virtual void exit() {
      MultiEditMenu::exit();
    }
};

class ScheduleEditMenu : public GenericMenu {
  public:
    ScheduleTimeEditMenu* schedule_edit;
    ScheduleDurationEditMenu* duration_edit;
    ScheduleDOWEditMenu* dow_edit;
    schedule_data* data;
    ScheduleEditMenu(Display* _display, Buttons* _buttons) : GenericMenu(_display, _buttons) {
      schedule_edit = new ScheduleTimeEditMenu(_display, _buttons);
      duration_edit = new ScheduleDurationEditMenu(_display, _buttons);
      dow_edit = new ScheduleDOWEditMenu(_display, _buttons);
    }
    virtual void init(schedule_data* s) {
      GenericMenu::init();
      data = s;
    }
    virtual void menu_title(char s[]) {
      strcpy(s, "Edit Schedule");
    }
    virtual uint8_t item_count() {
      return 6;
    }
    virtual void item_text(uint8_t item, char s[]) {
      char timeformat[14];
      switch(item) {
        case 0:
          dow_format(s, data); break;
        case 1:
          time_format(s, data); break;
        case 2:
          duration_format(s, data); break;
        case 3:
          if(data->enabled) {
            strcpy (s, "Schedule Enabled");
          } else {
            strcpy (s, "Schedule Disabled");
          }
          break;
        case 4:
          strcpy (s, "Delete"); break;
        case 5:
          strcpy (s, "Exit"); break;
      }
    }
    virtual uint16_t item_enter(uint8_t item) {
      switch(item) {
        case 0:
          dow_edit->init(data);
          set_submenu(dow_edit);
          break;
        case 1:
          schedule_edit->init(data);
          set_submenu(schedule_edit);
          break;
        case 2:
          duration_edit->init(data);
          set_submenu(duration_edit);
          break;
        case 3:
          data->enabled = !data->enabled;
          break;
        case 4:
          return 2;
        case 5:
          return 1;
      }
      return 0;
    }
    virtual void exit() {
      storage->store();
      GenericMenu::exit();
    }
};

class ScheduleListMenu : public GenericMenu {
  public:
    ScheduleEditMenu* scheduleedit;
    ScheduleListMenu(Display* _display, Buttons* _buttons) : GenericMenu(_display, _buttons) {
      scheduleedit = new ScheduleEditMenu(display, buttons);
    }
    virtual void init() {
      GenericMenu::init();
    }
    virtual void menu_title(char s[]) {
      strcpy(s, "Schedules");
    }
    virtual uint8_t item_count() {
      uint8_t items = storage->data->schedule_count + 1;
      if(storage->data->schedule_count < MAX_SCHEDULES)
        items ++;
      return items;
    }
    virtual void item_text(uint8_t item, char s[]) {
      if(item==item_count()-1) {
          strcpy (s, "Exit");
      } else if(storage->data->schedule_count < MAX_SCHEDULES && item==item_count()-2) {
          strcpy (s, "Add schedule");
      } else {
        char t[50];
        schedule_format(t, &storage->data->schedules[item]);
        sprintf(s, "#%d: %s", item+1, t);
      }
    }
    virtual uint16_t item_enter(uint8_t item) {
      if(item==item_count()-1) {
        return 1;
      } else if(storage->data->schedule_count < MAX_SCHEDULES && item==item_count()-2) {
        storage->data->schedule_count++;
        scheduleedit->init(&storage->data->schedules[item]);
        set_submenu(scheduleedit);
      } else {
        scheduleedit->init(&storage->data->schedules[item]);
        set_submenu(scheduleedit);
      }
      return 0;
    }
    virtual void return_to(Menu* from_submenu, uint16_t retval) {
      if(from_submenu==scheduleedit && retval==2) {
        storage->delete_schedule(selected_item);
      }
      GenericMenu::return_to(from_submenu, retval);
    }
    virtual void exit() {
      //TODO: delete duplidate schedules?
      storage->sort_schedules();
      GenericMenu::exit();
    }
};
