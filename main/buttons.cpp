#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <time.h>
#include <hardware/flash.h>
#include "hardware/sync.h"
#include <string.h>


class Button {
  public:
    int last_state;
    virtual int state();
    virtual void update_state() {
      last_state = state();
    }
    int get_last_state() {
      return last_state;
    }
};

class PinButton : public Button {
  public:
    int pin;
    PinButton(int _pin) {
      pin = _pin;
      gpio_init(pin);
      gpio_pull_up(pin);
      gpio_set_dir(pin, GPIO_IN);
    }
    int state() {
      return !gpio_get(pin);
    }
};

class ButtonDebounce : public Button {
  public:
    enum ButtonDebounceState { none, down, activated };
    static const int DEBOUNCE_TIME = 20000;
    uint32_t button_down_time;
    ButtonDebounceState button_state;
    Button* button;
    ButtonDebounce(Button* _button) {
      button = _button;
      button_state = ButtonDebounceState::none;
    }
    int state() {
      switch(button_state) {
        case ButtonDebounceState::none:
          if(button->get_last_state()) {
            button_state = ButtonDebounceState::down;
            button_down_time = time_us_32();
          }
          return false;
        case ButtonDebounceState::down:
          if(!button->get_last_state()) {
            button_state = ButtonDebounceState::none;
            return false;
          }
          if((time_us_32()-button_down_time) > DEBOUNCE_TIME) {
            button_state = ButtonDebounceState::activated;
            return true;
          }
          return false;
        case ButtonDebounceState::activated:
          if(!button->get_last_state()) {
            button_state = ButtonDebounceState::none;
            return false;
          }
          return true;
      }
      return false;
    }
    void update_state() {
      button->update_state();
      Button::update_state();
    }
};

class DuplicateButton : public Button {
  public:
    Button* button1;
    Button* button2;
    DuplicateButton(Button* _b1, Button* _b2) {
      button1 = _b1;
      button2 = _b2;
    }
    int state() {
      bool b1 = button1->get_last_state();
      bool b2 = button2->get_last_state();
      return b1 || b2;
    }
    void update_state() {
      button1->update_state();
      button2->update_state();
      Button::update_state();
    }
};

class ButtonActions : public Button {
  public:
    enum ButtonInternalState { bis_none, bis_just_pressed, bis_holding };
    enum ButtonActionsState { bas_none, bas_pressed, bas_holding_pending, bas_longpress, bas_holding };
    static const int HOLD_TIME = 500000;
    Button* button;
    ButtonInternalState internal_state;
    uint32_t button_down_time;
    ButtonActions(Button* _button) {
      button = _button;
      internal_state = ButtonInternalState::bis_none;
    }
    int state() {
      bool button_state = button->get_last_state();
      switch(internal_state) {
        case ButtonInternalState::bis_none:
          if(button_state) {
            internal_state = ButtonInternalState::bis_just_pressed;
            button_down_time = time_us_32();
            return ButtonActionsState::bas_holding_pending;
          }
          return ButtonActionsState::bas_none;
        case ButtonInternalState::bis_just_pressed:
          if(!button_state) {
            internal_state = ButtonInternalState::bis_none;
            return ButtonActionsState::bas_pressed;
          }
          if((time_us_32()-button_down_time) > HOLD_TIME) {
            internal_state = ButtonInternalState::bis_holding;
            return ButtonActionsState::bas_longpress;
          }
          return ButtonActionsState::bas_holding_pending;
        case ButtonInternalState::bis_holding:
          if(!button_state) {
            internal_state = ButtonInternalState::bis_none;
            return ButtonActionsState::bas_none;
          }
          return ButtonActionsState::bas_holding;
      }
      return ButtonActionsState::bas_none;
    }
    void update_state() {
      button->update_state();
      Button::update_state();
    }
};

class DualButtonActions : public Button {
  public:
    ButtonActions* button1;
    ButtonActions* button2;
    enum DualButtonInternalState { dbis_none, dbis_holding };
    enum DualButtonActionsState { dbas_none, dbas_pressed };
    DualButtonInternalState internal_state;
    DualButtonActions(ButtonActions* _b1, ButtonActions* _b2) {
      button1 = _b1;
      button2 = _b2;
      internal_state = DualButtonInternalState::dbis_none;
    }
    int state() {
      int b1 = button1->get_last_state();
      int b2 = button2->get_last_state();
      switch(internal_state) {
        case DualButtonInternalState::dbis_none:
          if(b1 == ButtonActions::ButtonActionsState::bas_holding_pending && b2 == ButtonActions::ButtonActionsState::bas_holding_pending) {
            internal_state = DualButtonInternalState::dbis_holding;
            button1->last_state = ButtonActions::ButtonActionsState::bas_none;
            button2->last_state = ButtonActions::ButtonActionsState::bas_none;
            return DualButtonActionsState::dbas_pressed;
          }
          if(b1 != ButtonActions::ButtonActionsState::bas_none && b2 != ButtonActions::ButtonActionsState::bas_none) {
            internal_state = DualButtonInternalState::dbis_holding;
            button1->last_state = ButtonActions::ButtonActionsState::bas_none;
            button2->last_state = ButtonActions::ButtonActionsState::bas_none;
            return DualButtonActionsState::dbas_none;
          }
          return DualButtonActionsState::dbas_none;
        case DualButtonInternalState::dbis_holding:
          if(b1 == ButtonActions::ButtonActionsState::bas_none && b2 == ButtonActions::ButtonActionsState::bas_none) {
            internal_state = DualButtonInternalState::dbis_none;
          }
          button1->last_state = ButtonActions::ButtonActionsState::bas_none;
          button2->last_state = ButtonActions::ButtonActionsState::bas_none;
          return DualButtonActionsState::dbas_none;
      }
      return DualButtonActionsState::dbas_none;
    }
    void update_state() {
      button1->update_state();
      button2->update_state();
      Button::update_state();
    }
};


class Buttons {
  public:
    PinButton* left_button_raw;
    PinButton* right_button_raw;
    PinButton* centre_button_raw;

    ButtonDebounce* left_button;
    ButtonDebounce* right_button;
    ButtonDebounce* centre_button;

    ButtonActions* left_button_actions;
    ButtonActions* right_button_actions;
    ButtonActions* centre_button_actions;

    DualButtonActions* dual_buttons;
    Buttons() {
      left_button_raw = new PinButton(LEFT_BUTTON_PIN);
      right_button_raw = new PinButton(RIGHT_BUTTON_PIN);
      centre_button_raw = new PinButton(CENTRE_BUTTON_PIN);

      left_button = new ButtonDebounce(left_button_raw);
      right_button = new ButtonDebounce(right_button_raw);
      centre_button = new ButtonDebounce(centre_button_raw);

      left_button_actions = new ButtonActions(left_button);
      right_button_actions = new ButtonActions(right_button);
      centre_button_actions = new ButtonActions(centre_button);

      dual_buttons = new DualButtonActions(left_button_actions, right_button_actions);
    }
    void update_state() {
      dual_buttons->update_state();
      centre_button_actions->update_state();
    }
};

