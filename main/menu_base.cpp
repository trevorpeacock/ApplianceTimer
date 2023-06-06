#define SECONDS_IN_HOUR 3600

class Menu {
  public:
    Display* display;
    Buttons* buttons;
    virtual void init() {}
    virtual void enter() {}
    virtual void exit() {}
    virtual uint8_t run();
    virtual uint16_t check_buttons() {return 0;} // called only on the currently active menu
    Menu(Display* _display, Buttons* _buttons) {
      display = _display;
      buttons = _buttons;
    }
    virtual uint16_t process() {
      uint16_t result = check_buttons();
      if(result!=0) {
        return result;
      }
      run();
      return 0;
    }
};

class ParentMenu: public Menu {
  public:
    Menu* current_submenu;
    virtual void leave() {}
    virtual void return_to(Menu* from_submenu, uint16_t retval) {}
    ParentMenu(Display* _display, Buttons* _buttons) : Menu(_display, _buttons) {
      current_submenu = NULL;
    }
    virtual uint16_t check_child(uint16_t submenu_result) { // called on every menu with an active child
      if(submenu_result!=0) {
        unset_submenu(submenu_result);
      }
      return 0;
    };
    virtual uint16_t check_override() { // called on every menu
      return 0;
    };
    virtual uint16_t process() {
      uint16_t result;
      result = check_override();
      if(result!=0) {
        return result;
      }
      if(current_submenu==NULL) {
        return Menu::process();
      }
      result = current_submenu->process();
      return check_child(result);
    }
    void set_submenu(Menu* submenu) {
      if(current_submenu!=NULL) {
        unset_submenu();
      }
      current_submenu = submenu;
      leave();
      current_submenu->enter();
    }
    void unset_submenu(uint16_t retval) {
      Menu* previous_submenu = current_submenu;
      current_submenu = NULL;
      previous_submenu->exit();
      return_to(previous_submenu, retval);
    }
    void unset_submenu() {
      unset_submenu(0);
    }
    virtual void exit() {
      if(current_submenu!=NULL) {
        unset_submenu();
      }
      Menu::exit();
    }
};

class MessageDisplay : public Menu {
  public:
    char message[100];
    using Menu::Menu;
    virtual void init(const char *_message) {
      strcpy (message, _message);
      Menu::init();
    }
    virtual uint8_t run() {
      u8g2_ClearBuffer(&display->u8g2);
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB14_tr);
      u8g2_DrawStrCenter(&display->u8g2, message);
      u8g2_SendBuffer(&display->u8g2);
      return 0;
    }
};

class RollingGenericMenu : public ParentMenu {
  public:
    uint8_t selected_item;
    char message[100];
    uint64_t start_time;
    using ParentMenu::ParentMenu;
    virtual uint8_t item_count();
    virtual void item_text(uint8_t item, char s[]);
    virtual uint16_t item_enter(uint8_t item);
    virtual void init() {
      selected_item = 0;
      ParentMenu::init();
    }
    float scroll_position() {
      uint64_t t = (time_us_64() - start_time) % 3000000;
      if(t<1000000) return 0.0;
      if(t>=2000000) return 1.0;
      return (float)(t-1000000)/1000000;
    }
    void item_text_pos(int8_t item, char s[]) {
      uint8_t itemcount = item_count();
      item_text((item + selected_item + itemcount) % itemcount, s);
    }
    virtual uint16_t check_buttons() {
      int button_l = buttons->left_button_actions->get_last_state();
      int button_c = buttons->centre_button_actions->get_last_state();
      int button_r = buttons->right_button_actions->get_last_state();
      int button_dual = buttons->dual_buttons->get_last_state();
      uint8_t itemcount = item_count();
      if(button_l==ButtonActions::ButtonActionsState::bas_pressed) {
        selected_item = (selected_item + itemcount - 1) % itemcount;
        start_time = time_us_64();
      }
      if(button_c==ButtonActions::ButtonActionsState::bas_pressed) {
        return item_enter(selected_item);
      }
      if(button_r==ButtonActions::ButtonActionsState::bas_pressed) {
        selected_item = (selected_item + itemcount + 1) % itemcount;
        start_time = time_us_64();
      }
      //if(button_dual==DualButtonActions::DualButtonActionsState::dbas_pressed) {
      //  return 1;
      //}
      return 0;
    }
    virtual void enter() {
      ParentMenu::enter();
      start_time = time_us_64();
    }
    virtual void return_to(Menu* from_submenu, uint16_t retval) {
      start_time = time_us_64();
      ParentMenu::return_to(from_submenu, retval);
    }
    virtual uint8_t run() {
      u8g2_ClearBuffer(&display->u8g2);
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB10_tr);
      item_text_pos(-1, message);
      u8g2_DrawStrCenter(&display->u8g2, 6, message);
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB14_tr);
      u8g2_DrawBox(&display->u8g2, 0, 15, 128, 21);
      u8g2_SetDrawColor(&display->u8g2, 0);
      item_text_pos(0, message);
      if(u8g2_GetStrWidth(&display->u8g2, message)>128) {
        u8g2_DrawStr(&display->u8g2, (int16_t)((124-u8g2_GetStrWidth(&display->u8g2, message))*scroll_position())+2, 24+u8g2_GetAscent(&display->u8g2)/2, message);
      } else {
        u8g2_DrawStrCenter(&display->u8g2, 24, message);
      }
      u8g2_SetDrawColor(&display->u8g2, 1);
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB10_tr);
      item_text_pos(1, message);
      u8g2_DrawStrCenter(&display->u8g2, 43, message);

      u8g2_DrawRFrame(&display->u8g2, 0, 52, 43, 14, 3);
      u8g2_DrawRFrame(&display->u8g2, 42, 52, 44, 14, 3);
      u8g2_DrawRFrame(&display->u8g2, 85, 52, 43, 14, 3);
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB08_tr);
      u8g2_DrawStrCenter(&display->u8g2, 21, 58, "UP");
      u8g2_DrawStrCenter(&display->u8g2, 64, 58, "OK");
      u8g2_DrawStrCenter(&display->u8g2, 106, 58, "DOWN");

      u8g2_SendBuffer(&display->u8g2);
      return 0;
    }
};

#define VISIBLE_MENU_ITEMS 3

class GenericMenu : public ParentMenu {
  public:
    uint8_t menu_item_offset;
    uint8_t selected_item;
    char message[100];
    uint64_t start_time;
    using ParentMenu::ParentMenu;
    virtual uint8_t item_count();
    virtual void menu_title(char s[]);
    virtual void item_text(uint8_t item, char s[]);
    virtual uint16_t item_enter(uint8_t item);
    virtual void init() {
      selected_item = 0;
      menu_item_offset = 0;
      ParentMenu::init();
    }
    float scroll_position() {
      uint64_t t = (time_us_64() - start_time) % 3000000;
      if(t<1000000) return 0.0;
      if(t>=2000000) return 1.0;
      return (float)(t-1000000)/1000000;
    }
    void item_text_pos(int8_t item, char s[]) {
      item_text(item, s);
    }
    virtual uint16_t check_buttons() {
      int button_l = buttons->left_button_actions->get_last_state();
      int button_c = buttons->centre_button_actions->get_last_state();
      int button_r = buttons->right_button_actions->get_last_state();
      int button_dual = buttons->dual_buttons->get_last_state();
      uint8_t itemcount = item_count();
      if(button_l==ButtonActions::ButtonActionsState::bas_pressed) {
        selected_item = (selected_item + itemcount - 1) % itemcount;
        start_time = time_us_64();
      }
      if(button_c==ButtonActions::ButtonActionsState::bas_pressed) {
        return item_enter(selected_item);
      }
      if(button_r==ButtonActions::ButtonActionsState::bas_pressed) {
        selected_item = (selected_item + itemcount + 1) % itemcount;
        start_time = time_us_64();
      }
      while((selected_item - menu_item_offset)>=VISIBLE_MENU_ITEMS) {
        menu_item_offset++;
      }
      while((selected_item - menu_item_offset)<0) {
        menu_item_offset--;
      }
      //if(button_dual==DualButtonActions::DualButtonActionsState::dbas_pressed) {
      //  return 1;
      //}
      return 0;
    }
    virtual void enter() {
      ParentMenu::enter();
      start_time = time_us_64();
    }
    virtual void return_to(Menu* from_submenu, uint16_t retval) {
      start_time = time_us_64();
      ParentMenu::return_to(from_submenu, retval);
    }
    virtual uint8_t run() {
      u8g2_ClearBuffer(&display->u8g2);

      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB08_tr);
      uint8_t bottom_item = item_count();
      if(bottom_item > VISIBLE_MENU_ITEMS)
        bottom_item = VISIBLE_MENU_ITEMS;
      for(uint8_t i = 0; i < bottom_item; i++) {
        bool selected = i == (selected_item - menu_item_offset);
        item_text_pos(menu_item_offset+i, message);
        if(selected) {
          u8g2_DrawBox(&display->u8g2, 0, 14 + i*12, 128, 12);
        }
        u8g2_SetDrawColor(&display->u8g2, selected ? 0 : 1);
        if(u8g2_GetStrWidth(&display->u8g2, message)>128) {
          u8g2_DrawStr(&display->u8g2, (int16_t)((124-u8g2_GetStrWidth(&display->u8g2, message))*scroll_position())+2, 19 + i*12 +u8g2_GetAscent(&display->u8g2)/2, message);
        } else {
          u8g2_DrawStr(&display->u8g2, 0, 19 + i*12 +u8g2_GetAscent(&display->u8g2)/2, message);
          //u8g2_DrawStrCenter(&display->u8g2, 6 + i*12, message);
        }
        u8g2_SetDrawColor(&display->u8g2, 1);
      }

      u8g2_DrawRFrame(&display->u8g2, 0, 52, 43, 14, 3);
      u8g2_DrawRFrame(&display->u8g2, 42, 52, 44, 14, 3);
      u8g2_DrawRFrame(&display->u8g2, 85, 52, 43, 14, 3);
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB08_tr);
      u8g2_DrawStrCenter(&display->u8g2, 21, 58, "UP");
      u8g2_DrawStrCenter(&display->u8g2, 64, 58, "OK");
      u8g2_DrawStrCenter(&display->u8g2, 106, 58, "DOWN");

      u8g2_DrawRFrame(&display->u8g2, 0, -10, 128, 23, 3);
      menu_title(message);
      u8g2_DrawStrCenter(&display->u8g2, 5, message);

      u8g2_SendBuffer(&display->u8g2);
      return 0;
    }
};

class SettingsMenuTest : public GenericMenu {
  public:
    using GenericMenu::GenericMenu;
    virtual uint8_t item_count() {
      return 5;
    }
    virtual void item_text(uint8_t item, char s[]) {
      sprintf(s, "Item #%d", item);
    }
    virtual uint16_t item_enter(uint8_t item) {
      return 1;
    }
};

class MultiEditMenu : public Menu {
  public:
    uint8_t element;
    uint64_t hold_timer;
    MultiEditMenu(Display* _display, Buttons* _buttons) : Menu(_display, _buttons) {
    }
    virtual void init() {
      Menu::init();
      element = 0;
      hold_timer = time_us_64();
    }
    virtual uint8_t element_count();
    virtual bool allow_holding(uint8_t e) { return false; }
    virtual void draw_element(uint8_t e, bool selected);
    virtual void draw_background();
    virtual void change_element(uint8_t e, int8_t change);
    virtual uint16_t check_buttons() {
      int button_l = buttons->left_button_actions->get_last_state();
      int button_c = buttons->centre_button_actions->get_last_state();
      int button_r = buttons->right_button_actions->get_last_state();
      int8_t direction = 0;
      if(button_l==ButtonActions::ButtonActionsState::bas_pressed) {
        direction = -1;
      }
      if(button_l==ButtonActions::ButtonActionsState::bas_holding && allow_holding(element)) {
        if(time_us_64() - hold_timer > 100000) {
          direction = -1;
          hold_timer = time_us_64();
        }
      }
      if(button_c==ButtonActions::ButtonActionsState::bas_pressed) {
        element++;
        if(element==element_count())
          return 1;
      }
      if(button_r==ButtonActions::ButtonActionsState::bas_pressed) {
        direction = 1;
      }
      if(button_r==ButtonActions::ButtonActionsState::bas_holding && allow_holding(element)) {
        if(time_us_64() - hold_timer > 100000) {
          direction = 1;
          hold_timer = time_us_64();
        }
      }
      change_element(element, direction);
      return 0;
    }
    virtual uint8_t run() {
      u8g2_ClearBuffer(&display->u8g2);
      draw_background();
      for(uint8_t i=0; i<element_count(); i++) {
        draw_element(i, i==element);
      }

      u8g2_DrawRFrame(&display->u8g2, 0, 52, 43, 14, 3);
      u8g2_DrawRFrame(&display->u8g2, 42, 52, 44, 14, 3);
      u8g2_DrawRFrame(&display->u8g2, 85, 52, 43, 14, 3);
      u8g2_SetFont(&display->u8g2, u8g2_font_ncenB08_tr);
      u8g2_DrawStrCenter(&display->u8g2, 21, 58, "-");
      if(element==element_count()-1) {
        u8g2_DrawStrCenter(&display->u8g2, 64, 58, "Done");
      } else {
        u8g2_DrawStrCenter(&display->u8g2, 64, 58, "Next");
      }
      u8g2_DrawStrCenter(&display->u8g2, 106, 58, "+");

      u8g2_SendBuffer(&display->u8g2);
      return 0;
    }
};

class CycleMenus : public ParentMenu {
  public:
    using ParentMenu::ParentMenu;
    virtual Menu* get_menu();
    virtual uint16_t check_override() {
      if(current_submenu==NULL) {
        set_submenu(get_menu());
      }
      return ParentMenu::check_override();
    };
    virtual uint16_t check_child(uint16_t submenu_result) {
      Menu* menu = get_menu();
      if(menu==NULL)
        return 1;
      if(submenu_result!=0 || current_submenu != menu) {
        unset_submenu();
        set_submenu(menu);
      }
      return 0;
    }
};
