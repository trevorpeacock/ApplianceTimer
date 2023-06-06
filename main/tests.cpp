void display_failure(Display* display, bool ok, const char *data, const char *s1, const char *s2) {
  if(ok) return;
  u8g2_ClearBuffer(&display->u8g2);
  u8g2_SetFont(&display->u8g2, u8g2_font_timR08_tf);
  u8g2_DrawStr(&display->u8g2, 0, 8, "Test Failure");
  u8g2_DrawStr(&display->u8g2, 0, 18, s1);
  u8g2_DrawStr(&display->u8g2, 0, 28, s2);
  u8g2_DrawStr(&display->u8g2, 0, 38, data);
  u8g2_SendBuffer(&display->u8g2);
  while(true);
}

void format_schedule_result(char *s, schedule_calc_data* schedule_result) {
  sprintf(s, "%04d-%02d-%02d (%d) %02d:%02d:%02d %dmin", schedule_result->datetime.date.year, schedule_result->datetime.date.month, schedule_result->datetime.date.day, schedule_result->datetime.date.day_of_week,
    schedule_result->datetime.time.hour, schedule_result->datetime.time.minute, schedule_result->datetime.time.second, schedule_result->duration);
}

void format_datetime(char *s, datetime_data* datetime) {
  sprintf(s, "%04d-%02d-%02d (%d) %02d:%02d:%02d", datetime->date.year, datetime->date.month, datetime->date.day, datetime->date.day_of_week,
    datetime->time.hour, datetime->time.minute, datetime->time.second);
}

void format_dst_check_result(char *s, dst_check_result* res) {
  sprintf(s, "%s %s %d", res->change ? "true" : "false", res->dst_state ? "true" : "false", res->hour_change);
}

void test_time_calculations(Display* display) {
  char s[100];
  datetime_data test_current_time;
  schedule_data test_schedule;
  schedule_calc_data schedule_result;
  dst_change_schedule dst_schedule;
  dst_change_schedule dst_schedule2;
  dst_check_result dst_result;
  datetime_data next_dst_change;

  /*
   * check schedule calculations via schedule_next_time
   */
  test_current_time = (datetime_data){{2023, 0, 0, 0}, {9, 0, 0}};
  test_schedule = (schedule_data){true, 0b01111111, 9, 0, 3600};
  schedule_next_time(&test_schedule, &schedule_result, &test_current_time);
  format_schedule_result(s, &schedule_result);
  display_failure(display, schedule_result==(schedule_calc_data){{{2023, 0, 0, 0}, {9, 0, 0}}, 3600}, s, "schedule_next_time", "Identical Time");

  test_current_time = (datetime_data){{2023, 0, 0, 0}, {9, 0, 1}};
  test_schedule = (schedule_data){true, 0b01111111, 9, 0, 3600};
  schedule_next_time(&test_schedule, &schedule_result, &test_current_time);
  format_schedule_result(s, &schedule_result);
  display_failure(display, schedule_result==(schedule_calc_data){{{2023, 0, 1, 1}, {9, 0, 0}}, 3600}, s, "schedule_next_time", "Time Passed");

  test_current_time = (datetime_data){{2023, 0, 0, 0}, {9, 0, 0}};
  test_schedule = (schedule_data){true, 0b00000001, 9, 0, 3600};
  schedule_next_time(&test_schedule, &schedule_result, &test_current_time);
  format_schedule_result(s, &schedule_result);
  display_failure(display, schedule_result==(schedule_calc_data){{{2023, 0, 6, 6}, {9, 0, 0}}, 3600}, s, "schedule_next_time", "Different Day");

  /*
   * check future dst calculations via dst_next_date
   */
  test_current_time = (datetime_data){{2023, 5-1, 28-1, 0}, {9, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_next_date(&dst_schedule, false, &test_current_time);
  format_datetime(s, &test_current_time);
  display_failure(display, test_current_time==(datetime_data){{2023, 10-1, 1-1, 0}, {3, 0, 0}}, s, "dst_next_date", "Enter DST");
  display_failure(display, test_current_time.date.day_of_week==0, s, "dst_next_date", "Enter DST");

  test_current_time = (datetime_data){{2023, 5-1, 28-1, 0}, {9, 0, 0}};
  dst_schedule = (dst_change_schedule){4 - 1, 1};
  dst_next_date(&dst_schedule, true, &test_current_time);
  format_datetime(s, &test_current_time);
  display_failure(display, test_current_time==(datetime_data){{2024, 4-1, 7-1, 0}, {2, 0, 0}}, s, "dst_next_date", "Exit DST");
  display_failure(display, test_current_time.date.day_of_week==0, s, "dst_next_date", "Exit DST");

  //2nd sunday
  test_current_time = (datetime_data){{2023, 5-1, 28-1, 0}, {9, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 2};
  dst_next_date(&dst_schedule, false, &test_current_time);
  format_datetime(s, &test_current_time);
  display_failure(display, test_current_time==(datetime_data){{2023, 10-1, 8-1, 0}, {3, 0, 0}}, s, "dst_next_date", "2nd Sunday");
  display_failure(display, test_current_time.date.day_of_week==0, s, "dst_next_date", "Enter DST");

  //last sunday
  test_current_time = (datetime_data){{2023, 5-1, 28-1, 0}, {9, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, -1};
  dst_next_date(&dst_schedule, false, &test_current_time);
  format_datetime(s, &test_current_time);
  display_failure(display, test_current_time==(datetime_data){{2023, 10-1, 29-1, 0}, {3, 0, 0}}, s, "dst_next_date", "Last Sunday");
  display_failure(display, test_current_time.date.day_of_week==0, s, "dst_next_date", "Enter DST");

  //2nd last sunday
  test_current_time = (datetime_data){{2023, 5-1, 28-1, 0}, {9, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, -2};
  dst_next_date(&dst_schedule, false, &test_current_time);
  format_datetime(s, &test_current_time);
  display_failure(display, test_current_time==(datetime_data){{2023, 10-1, 22-1, 0}, {3, 0, 0}}, s, "dst_next_date", "2nd Last Sunday");
  display_failure(display, test_current_time.date.day_of_week==0, s, "dst_next_date", "Enter DST");

  /*
   * check past dst calculations via dst_previous_date
   */
  test_current_time = (datetime_data){{2023, 5-1, 28-1, 0}, {9, 0, 0}};
  dst_schedule = (dst_change_schedule){4 - 1, 1};
  dst_previous_date(&dst_schedule, true, &test_current_time);
  format_datetime(s, &test_current_time);
  display_failure(display, test_current_time==(datetime_data){{2023, 4-1, 2-1, 0}, {2, 0, 0}}, s, "dst_previous_date", "Exit DST");
  display_failure(display, test_current_time.date.day_of_week==0, s, "dst_next_date", "Exit DST");

  test_current_time = (datetime_data){{2023, 5-1, 28-1, 0}, {9, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_previous_date(&dst_schedule, false, &test_current_time);
  format_datetime(s, &test_current_time);
  display_failure(display, test_current_time==(datetime_data){{2022, 10-1, 2-1, 0}, {3, 0, 0}}, s, "dst_previous_date", "Enter DST");
  display_failure(display, test_current_time.date.day_of_week==0, s, "dst_next_date", "Enter DST");

  //2nd sunday
  test_current_time = (datetime_data){{2023, 5-1, 28-1, 0}, {9, 0, 0}};
  dst_schedule = (dst_change_schedule){4 - 1, 2};
  dst_previous_date(&dst_schedule, true, &test_current_time);
  format_datetime(s, &test_current_time);
  display_failure(display, test_current_time==(datetime_data){{2023, 4-1, 9-1, 0}, {2, 0, 0}}, s, "dst_previous_date", "2nd Sunday");
  display_failure(display, test_current_time.date.day_of_week==0, s, "dst_next_date", "Exit DST");

  //last sunday
  test_current_time = (datetime_data){{2023, 5-1, 28-1, 0}, {9, 0, 0}};
  dst_schedule = (dst_change_schedule){4 - 1, -1};
  dst_previous_date(&dst_schedule, true, &test_current_time);
  format_datetime(s, &test_current_time);
  display_failure(display, test_current_time==(datetime_data){{2023, 4-1, 30-1, 0}, {2, 0, 0}}, s, "dst_previous_date", "Last Sunday");
  display_failure(display, test_current_time.date.day_of_week==0, s, "dst_next_date", "Exit DST");

  //2nd last sunday
  test_current_time = (datetime_data){{2023, 5-1, 28-1, 0}, {9, 0, 0}};
  dst_schedule = (dst_change_schedule){4 - 1, -2};
  dst_previous_date(&dst_schedule, true, &test_current_time);
  format_datetime(s, &test_current_time);
  display_failure(display, test_current_time==(datetime_data){{2023, 4-1, 23-1, 0}, {2, 0, 0}}, s, "dst_previous_date", "2nd Last Sunday");
  display_failure(display, test_current_time.date.day_of_week==0, s, "dst_next_date", "Exit DST");

  /*
   * check DST entry/exit process via format_dst_check_result
   */
  test_current_time = (datetime_data){{2023, 5-1, 28-1, 0}, {9, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  dst_result = get_dst_change(&test_current_time, &dst_schedule, &dst_schedule2, false);
  format_dst_check_result(s, &dst_result);
  display_failure(display, dst_result==(dst_check_result){false, false, 0}, s, "get_dst_change", "No Change");

  test_current_time = (datetime_data){{2023, 10-1, 1-1, 0}, {1, 59, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  dst_result = get_dst_change(&test_current_time, &dst_schedule, &dst_schedule2, false);
  format_dst_check_result(s, &dst_result);
  display_failure(display, dst_result==(dst_check_result){false, false, 0}, s, "get_dst_change", "About to enter");

  test_current_time = (datetime_data){{2023, 10-1, 1-1, 0}, {2, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  dst_result = get_dst_change(&test_current_time, &dst_schedule, &dst_schedule2, false);
  format_dst_check_result(s, &dst_result);
  display_failure(display, dst_result==(dst_check_result){true, true, 1}, s, "get_dst_change", "Enter Exact");

  test_current_time = (datetime_data){{2023, 10-1, 8-1, 0}, {2, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  dst_result = get_dst_change(&test_current_time, &dst_schedule, &dst_schedule2, false);
  format_dst_check_result(s, &dst_result);
  display_failure(display, dst_result==(dst_check_result){true, true, 1}, s, "get_dst_change", "Enter After");

  test_current_time = (datetime_data){{2023, 10-1, 1-1, 0}, {2, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  dst_result = get_dst_change(&test_current_time, &dst_schedule, &dst_schedule2, true);
  format_dst_check_result(s, &dst_result);
  display_failure(display, dst_result==(dst_check_result){false, false, 0}, s, "get_dst_change", "Already entered");

  test_current_time = (datetime_data){{2024, 4-1, 7-1, 0}, {2, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  dst_result = get_dst_change(&test_current_time, &dst_schedule, &dst_schedule2, true);
  format_dst_check_result(s, &dst_result);
  display_failure(display, dst_result==(dst_check_result){false, false, 0}, s, "get_dst_change", "2am before exit");

  test_current_time = (datetime_data){{2024, 4-1, 7-1, 0}, {2, 59, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  dst_result = get_dst_change(&test_current_time, &dst_schedule, &dst_schedule2, true);
  format_dst_check_result(s, &dst_result);
  display_failure(display, dst_result==(dst_check_result){false, false, 0}, s, "get_dst_change", "About to exit");

  test_current_time = (datetime_data){{2024, 4-1, 7-1, 0}, {3, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  dst_result = get_dst_change(&test_current_time, &dst_schedule, &dst_schedule2, true);
  format_dst_check_result(s, &dst_result);
  display_failure(display, dst_result==(dst_check_result){true, false, -1}, s, "get_dst_change", "Exit exact");

  test_current_time = (datetime_data){{2024, 4-1, 7-1, 0}, {2, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  dst_result = get_dst_change(&test_current_time, &dst_schedule, &dst_schedule2, false);
  format_dst_check_result(s, &dst_result);
  display_failure(display, dst_result==(dst_check_result){false, false, 0}, s, "get_dst_change", "Just after exit");

  test_current_time = (datetime_data){{2024, 4-1, 7-1, 0}, {2, 0, 1}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  dst_result = get_dst_change(&test_current_time, &dst_schedule, &dst_schedule2, false);
  format_dst_check_result(s, &dst_result);
  display_failure(display, dst_result==(dst_check_result){false, false, 0}, s, "get_dst_change", "Just after exit2");

  test_current_time = (datetime_data){{2024, 4-1, 7-1, 0}, {3, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  dst_result = get_dst_change(&test_current_time, &dst_schedule, &dst_schedule2, false);
  format_dst_check_result(s, &dst_result);
  display_failure(display, dst_result==(dst_check_result){false, false, 0}, s, "get_dst_change", "Already exited");

  test_current_time = (datetime_data){{2024, 5-1, 7-1, 0}, {3, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  dst_result = get_dst_change(&test_current_time, &dst_schedule, &dst_schedule2, true);
  format_dst_check_result(s, &dst_result);
  display_failure(display, dst_result==(dst_check_result){true, false, -1}, s, "get_dst_change", "Exit After");

  test_current_time = (datetime_data){{2024, 5-1, 26-1, 0}, {9, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  dst_result = get_dst_change(&test_current_time, &dst_schedule, &dst_schedule2, false);
  format_dst_check_result(s, &dst_result);
  display_failure(display, dst_result==(dst_check_result){false, false, 0}, s, "get_dst_change", "Skipped DST");

  test_current_time = (datetime_data){{2024, 12-1, 1-1, 0}, {9, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  dst_result = get_dst_change(&test_current_time, &dst_schedule, &dst_schedule2, false);
  format_dst_check_result(s, &dst_result);
  display_failure(display, dst_result==(dst_check_result){true, true, 1}, s, "get_dst_change", "Enter year late");

  /*
   * check DST checks via get_next_dst_change
   */
  test_current_time = (datetime_data){{2023, 5-1, 28-1, 0}, {9, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  next_dst_change = get_next_dst_change(&test_current_time, false, &dst_schedule, &dst_schedule2);
  format_datetime(s, &next_dst_change);
  display_failure(display, next_dst_change==(datetime_data){{2023, 10-1, 1-1, 0}, {2, 0, 0}}, s, "get_next_dst_change", "Next Enter");
  display_failure(display, next_dst_change.date.day_of_week==0, s, "get_next_dst_change", "Next Enter");

  test_current_time = (datetime_data){{2023, 10-1, 8-1, 0}, {2, 0, 0}};
  dst_schedule = (dst_change_schedule){10 - 1, 1};
  dst_schedule2 = (dst_change_schedule){4 - 1, 1};
  next_dst_change = get_next_dst_change(&test_current_time, true, &dst_schedule, &dst_schedule2);
  format_datetime(s, &next_dst_change);
  display_failure(display, next_dst_change==(datetime_data){{2024, 4-1, 7-1, 0}, {3, 0, 0}}, s, "get_next_dst_change", "Next Exit");
  display_failure(display, next_dst_change.date.day_of_week==0, s, "get_next_dst_change", "Next Exit");


  /*
datetime_data get_next_dst_change(datetime_data* datetime, dst_change_schedule* enter, dst_change_schedule* exit) {

  */
  // last dst change made
  // most recent dst change
  // current time
}
