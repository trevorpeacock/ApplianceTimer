#include <stdio.h>
#include <string>

struct time_data {
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  bool operator>(const time_data &d) const {
    return hour != d.hour ? hour > d.hour :
           minute != d.minute ? minute > d.minute :
           second > d.second;
  }
  bool operator<(const time_data &d) const {
    return hour != d.hour ? hour < d.hour :
           minute != d.minute ? minute < d.minute :
           second < d.second;
  }
  bool operator==(const time_data &d) const {
    return hour == d.hour &&
           minute == d.minute &&
           second == d.second;
  }
  bool operator!=(const time_data &d) const {
    return !(*this==d);
  }
  bool operator<=(const time_data &d) const {
    return *this<d || *this==d;
  }
  bool operator>=(const time_data &d) const {
    return *this>d || *this==d;
  }
};

struct date_data {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t day_of_week;
  bool operator>(const date_data &d) const {
    return year != d.year ? year > d.year :
           month != d.month ? month > d.month :
           day > d.day;
  }
  bool operator<(const date_data &d) const {
    return year != d.year ? year < d.year :
           month != d.month ? month < d.month :
           day < d.day;
  }
  bool operator==(const date_data &d) const {
    return year == d.year &&
           month == d.month &&
           day == d.day;
  }
  bool operator!=(const date_data &d) const {
    return !(*this==d);
  }
  bool operator<=(const date_data &d) const {
    return *this<d || *this==d;
  }
  bool operator>=(const date_data &d) const {
    return *this>d || *this==d;
  }
};

struct datetime_data {
  date_data date;
  time_data time;
  bool operator>(const datetime_data &d) const {
    return date != d.date ? date > d.date :
           time > d.time;
  }
  bool operator<(const datetime_data &d) const {
    return date != d.date ? date < d.date :
           time < d.time;
  }
  bool operator==(const datetime_data &d) const {
    return date == d.date &&
           time == d.time;
  }
  bool operator!=(const datetime_data &d) const {
    return !(*this==d);
  }
  bool operator<=(const datetime_data &d) const {
    return *this<d || *this==d;
  }
  bool operator>=(const datetime_data &d) const {
    return *this>d || *this==d;
  }
};

struct schedule_calc_data {
  datetime_data datetime;
  uint16_t duration;
  bool operator==(const schedule_calc_data &d) const {
    return datetime == d.datetime &&
           duration == d.duration;
  }
};

struct day_of_week_common_pattern {
  day_of_week_pattern dow;
  char day_name[10];
  char dow_name[11];
  char dow_short_name[5];
};

char month_names[][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
day_of_week_common_pattern day_of_week_common_patterns[] = {
  {0b01000000, "Sunday", "Sundays", "Sun"},
  {0b00100000, "Monday", "Mondays", "Mon"},
  {0b00010000, "Tuesday", "Tuesdays", "Tue"},
  {0b00001000, "Wednesday", "Wednesdays", "Wed"},
  {0b00000100, "Thursday", "Thursdays", "Thur"},
  {0b00000010, "Friday", "Fridays", "Fri"},
  {0b00000001, "Saturday", "Saturdays", "Sat"},
  {0b01111111, "", "All Days", ""},
  {0b00111110, "", "Weekdays", "Week"},
  {0b01000001, "", "Weekends", "WE"}
};
char day_names_short[][3] = { "Su", "M", "Tu", "W", "Th", "F", "Sa" };

bool is_leap_year(uint16_t year) {
    return (year % 4 == 0 && year % 100 != 0) || year % 400 == 0;
}

uint8_t days_in_month(uint8_t month, uint16_t year) {
  month ++;
  return (month == 2) ? (28 + is_leap_year(year)) : 31 - (month - 1) % 7 % 2;
}

struct rtc_data {
  uint8_t second; // Clock-Halt tens tens tens unit unit unit unit
  uint8_t minute; // ? tens tens tens unit unit unit unit
  uint8_t hour; // 12/24 0 tens/am/pm tens unit unit unit unit
  uint8_t date; // 0 0 tens tens unit unit unit unit
  uint8_t month; // 0 0 0 tens unit unit unit unit
  uint8_t day; // 0 0 0 0 0 unit unit unit
  uint8_t year; // tens tens tens tens unit unit unit unit
  uint8_t wp; // write-protect 0 0 0 0 0 0 0
};

datetime_data rtc_to_datetime(rtc_data data) {
  datetime_data result;
  result.time.second = ((data.second & 0b01110000) >> 4) * 10 + (data.second & 0b00001111);
  result.time.minute = ((data.minute & 0b01110000) >> 4) * 10 + (data.minute & 0b00001111);
  result.time.hour = ((data.hour & 0b00110000) >> 4) * 10 + (data.hour & 0b00001111);
  result.date.day = ((data.date & 0b00110000) >> 4) * 10 + (data.date & 0b00001111) - 1;
  result.date.month = ((data.month & 0b00010000) >> 4) * 10 + (data.month & 0b00001111) - 1;
  result.date.day_of_week = (data.day & 0b00000111) - 1;
  result.date.year = 2000 + ((data.year & 0b11110000) >> 4) * 10 + (data.year & 0b00001111);
  return result;
}

rtc_data datetime_to_rtc(datetime_data data) {
  rtc_data result;
  result.second = (data.time.second / 10) << 4 | data.time.second % 10;
  result.minute = (data.time.minute / 10) << 4 | data.time.minute % 10;
  result.hour = (data.time.hour / 10) << 4 | data.time.hour % 10;
  result.date = ((data.date.day+1) / 10) << 4 | (data.date.day+1) % 10;
  result.month = ((data.date.month+1) / 10) << 4 | (data.date.month+1) % 10;
  result.day =  data.date.day_of_week + 1;
  result.year = ((data.date.year % 100) / 10) << 4 | data.date.year % 10;
  result.wp = 0b00000000;
  return result;
}

void time_format(char s[], time_data t) {
  if(storage->data->h24format) {
    sprintf(s, "%02d:%02d:%02d", t.hour, t.minute, t.second);
  } else {
    sprintf(s, "%d:%02d:%02d %s", ((t.hour+11)%12)+1, t.minute, t.second, (t.hour<12) ? "am" : "pm");
  }
}

void time_format(char s[], schedule_data* t) {
  if(storage->data->h24format) {
    sprintf(s, "%02d:%02d", t->hour, t->minute);
  } else {
    sprintf(s, "%d:%02d %s", ((t->hour+11)%12)+1, t->minute, (t->hour<12) ? "am" : "pm");
  }
}

void duration_format(char s[], schedule_data* t) {
  if(t->duration < 3600) {
    sprintf(s, "%d min", t->duration/60);
    return;
  }
  if(t->duration % 3600 == 0) {
    sprintf(s, "%d hours", t->duration/3600);
    return;
  }
  sprintf(s, "%dh %d min", t->duration/3600, (t->duration/60)%60);
}

void duration_short_format(char s[], schedule_data* t) {
  if(t->duration < 3600) {
    sprintf(s, "%dm", t->duration/60);
    return;
  }
  if(t->duration % 3600 == 0) {
    sprintf(s, "%dh", t->duration/3600);
    return;
  }
  sprintf(s, "%dh%dm", t->duration/3600, (t->duration/60)%60);
}

void raw_dow_format(char s[], schedule_data* t) {
  sprintf(s, "%s%s%s%s%s%s%s",
    t->day_of_week & 0b01000000 ? day_names_short[0] : "",
    t->day_of_week & 0b00100000 ? day_names_short[1] : "",
    t->day_of_week & 0b00010000 ? day_names_short[2] : "",
    t->day_of_week & 0b00001000 ? day_names_short[3] : "",
    t->day_of_week & 0b00000100 ? day_names_short[4] : "",
    t->day_of_week & 0b00000010 ? day_names_short[5] : "",
    t->day_of_week & 0b00000001 ? day_names_short[6] : ""
  );
}

int8_t dow_pattern_match(day_of_week_pattern p) {
  for(uint8_t i = 0; i < sizeof(day_of_week_common_patterns)/sizeof(day_of_week_common_patterns[0]); i++) {
    if(day_of_week_common_patterns[i].dow==p) {
      return i;
    }
  }
  return -1;
}

void dow_format(char s[], schedule_data* t) {
  int8_t p = dow_pattern_match(t->day_of_week);
  if(p==-1) {
    raw_dow_format(s, t);
    return;
  }
  strcpy (s, day_of_week_common_patterns[p].dow_name);
}

void dow_short_format(char s[], schedule_data* t) {
  int8_t p = dow_pattern_match(t->day_of_week);
  if(p==-1) {
    raw_dow_format(s, t);
    return;
  }
  strcpy (s, day_of_week_common_patterns[p].dow_short_name);
}

void time_short_format(char s[], schedule_data* t) {
  if(storage->data->h24format) {
    sprintf(s, "%02d:%02d", t->hour, t->minute);
  } else {
    if(t->minute==0) {
      sprintf(s, "%d%s", ((t->hour+11)%12)+1, (t->hour<12) ? "am" : "pm");
    } else {
      sprintf(s, "%d:%02d%s", ((t->hour+11)%12)+1, t->minute, (t->hour<12) ? "am" : "pm");
    }
  }
}

void schedule_format(char s[], schedule_data* t) {
  char enabled[] = "[Disabled] ";
  char day[11];
  char tim[8];
  char duration[6];
  if(t->enabled)
    strcpy(enabled, "");
  dow_short_format(day, t);
  time_short_format(tim, t);
  duration_short_format(duration, t);
  if(strlen(day)==0) {
    sprintf(s, "%s%s: %s", enabled, tim, duration);
  } else {
    sprintf(s, "%s%s %s: %s", enabled, day, tim, duration);
  }
}

void format_day_of_month(char s[], uint8_t d) {
  switch(d%10) {
    case 1:
      sprintf(s, "%dst", d); break;
    case 2:
      sprintf(s, "%dnd", d); break;
    case 3:
      sprintf(s, "%drd", d); break;
    default:
      sprintf(s, "%dth", d); break;
  }
}

void date_format(char s[], date_data t) {
  char day[5];
  format_day_of_month(day, t.day+1);
  sprintf(s, "%s %s %04d", day, month_names[t.month], t.year);
}

void countdown_format(char s[], time_data t) {
  if(t.hour!=0) {
    sprintf(s, "%d:%02d:%02d", t.hour, t.minute, t.second);
    return;
  }
  if(t.minute!=0) {
    sprintf(s, "%02d:%02d", t.minute, t.second);
    return;
  }
  sprintf(s, "%02d s", t.second);
}

void nth_day_format(char s[], int8_t sunday) {
  char day_number_format[5];
  if(sunday<0) {
    if(sunday==-1) {
      strcpy(s, "last");
    } else {
      format_day_of_month(day_number_format, -sunday);
      sprintf(s, "%s last", day_number_format);
    }
  } else {
    format_day_of_month(s, sunday);
  }
}

void dst_change_format(char s[], int8_t sunday, uint8_t month) {
  char day_format[9];
  nth_day_format(day_format, sunday);
  sprintf(s, "%s Sun of %s", day_format, month_names[month]);
}

class Timer {
  public:
    uint16_t countdown_time; //18 hours max
    uint64_t start_time;
    Timer() {
      countdown_time=0;
      start_time=0;
    }
    uint64_t seconds_elapsed() {
      return (time_us_64() - start_time) / 1000 / 1000;
    }
    uint16_t time_remaining_s() {
      int64_t remaining = ((int64_t)countdown_time) - seconds_elapsed();
      if(remaining < 0)
        return 0;
      return remaining;
    }
    time_data time_remaining() {
      uint16_t t = time_remaining_s();
      time_data result;
      result.second=t % 60;
      t = t / 60;
      result.minute=t % 60;
      t = t / 60;
      result.hour=t;
      return result;
    }
    void set_timer(uint16_t t) {
      start_time = time_us_64();
      if(t > 10*3600)
        t = 10*3600;
      countdown_time = t;
    }
    void update_timer(uint16_t t) {
      //sets time if new timer (t) finishes after current timer
      if(t > time_remaining_s())
        set_timer(t);
    }
    uint16_t add_time(uint16_t t) {
      set_timer(int(time_remaining_s() + 1.1 * t) / t * t);
      return countdown_time;
    }
    uint16_t add_time() {
      return add_time(3600);
    }
};

Timer* timer;

int8_t timeadd(time_data* data, int8_t hours, int8_t minutes, int8_t seconds, bool carry) {
  int16_t d = 0;
  int16_t h = data->hour;
  int16_t m = data->minute;
  int16_t s = data->second;
  s += seconds;
  while(s>=60) {
    s+=-60;
    if(carry) m+=1;
  }
  while(s<0) {
    s+=60;
    if(carry) m+=-1;
  }
  data->second = s;
  m += minutes;
  while(m>=60) {
    m+=-60;
    if(carry) h+=1;
  }
  while(m<0) {
    m+=60;
    if(carry) h+=-1;
  }
  data->minute = m;
  h += hours;
  while(h>=24) {
    h+=-24;
    if(carry) d+=1;
  }
  while(h<0) {
    h+=24;
    if(carry) d+=-1;
  }
  data->hour = h;
  return d;
}

void timeadd(schedule_data* data, int8_t hours, int8_t minutes, bool carry) {
  int16_t h = data->hour;
  int16_t m = data->minute;
  m += minutes;
  while(m>=60) {
    m+=-60;
    if(carry) h+=1;
  }
  while(m<0) {
    m+=60;
    if(carry) h+=-1;
  }
  data->minute = m;
  h += hours;
  while(h>=24) {
    h+=-24;
  }
  while(h<0) {
    h+=24;
  }
  data->hour = h;
}

void _dateadd_month(date_data* data, int16_t years, int8_t months) {
  int16_t y = data->year;
  int16_t m = data->month;

  y += years;

  m += months;
  while(m>=12) {
    m+=-12;
    y+=1;
  }
  while(m<0) {
    m+=12;
    y+=-1;
  }

  data->year=y;
  data->month=m;
}

void dateadd(date_data* data, int16_t years, int8_t months, int8_t days) {
  int16_t y = data->year;
  int16_t m = data->month;
  int16_t d = data->day;
  y += years;
  m += months;
  while(m>=12) {
    m+=-12;
  }
  while(m<0) {
    m+=12;
  }
  if(days!=0) {
    d += days;
    while(d>=days_in_month(m, y)) {
      d+=-days_in_month(m, y);
    }
    while(d<0) {
      d+=days_in_month(m, y);
    }
  }
  data->year=y;
  data->month=m;
  data->day=d;
}

void dateadd(date_data* data, int16_t days) {
  int16_t dow = data->day_of_week;
  int16_t d = data->day;

  d += days;
  dow += days;
  while(d>=days_in_month(data->month, data->year)) {
    d+=-days_in_month(data->month, data->year);
    _dateadd_month(data, 0, 1);
  }
  while(d<0) {
    _dateadd_month(data, 0, -1);
    d+=days_in_month(data->month, data->year);
  }

  while(dow>=7)
    dow+=-7;
  while(dow<0)
    dow+=7;

  data->day_of_week=dow;
  data->day=d;
}

void timeadd(datetime_data* data, int8_t hours, int8_t minutes, int8_t seconds) {
    dateadd(&data->date, timeadd(&data->time, hours, minutes, seconds, true));
}

void get_nth_sunday(int8_t dst_nth_sunday, date_data* date) {
  bool backwards = dst_nth_sunday < 0;
  if(backwards) {
    dst_nth_sunday = - dst_nth_sunday;
    dateadd(date, days_in_month(date->month, date->year)-date->day-1);
  } else {
    dateadd(date, -date->day);
  }
  while(date->day_of_week!=0)
    dateadd(date, backwards ? -1 : 1);
  dateadd(date, 7 * (dst_nth_sunday-1) * (backwards ? -1 : 1));
}

void dst_previous_date(dst_change_schedule* sched, bool dst_enter, datetime_data* datetime) {
  datetime_data tmp = *datetime;
  tmp.time.second = 0;
  tmp.time.minute = 0;
  tmp.time.hour = dst_enter ? 2 : 3;
  //if current month is dst change month, we need to figure out if its already passed
  if(tmp.date.month == sched->month) {
    get_nth_sunday(sched->nth_sunday, &tmp.date);
    if(tmp <= *datetime) {
      *datetime = tmp;
      return;
    }
  }
  //if its not dst change month, or we haven't hit it yet, we need to go back
  dateadd(&tmp.date, -tmp.date.day -days_in_month((tmp.date.month+11)%12, tmp.date.year));
  //and keep going back until we hit the correct month
  while(tmp.date.month != sched->month)
    dateadd(&tmp.date, -days_in_month((tmp.date.month+11)%12, tmp.date.year));
  //get correct day
  get_nth_sunday(sched->nth_sunday, &tmp.date);
  *datetime = tmp;
}

void dst_next_date(dst_change_schedule* sched, bool dst_enter, datetime_data* datetime) {
  datetime_data tmp = *datetime;
  tmp.time.second = 0;
  tmp.time.minute = 0;
  tmp.time.hour = dst_enter ? 2 : 3;
  //if current month is dst change month, we need to figure out if its already passed
  if(tmp.date.month == sched->month) {
    get_nth_sunday(sched->nth_sunday, &tmp.date);
    if(tmp > *datetime) {
      *datetime = tmp;
      return;
    }
  }
  //if its not dst change month, or we haven't hit it yet, we need to go forward
  dateadd(&tmp.date, -tmp.date.day +days_in_month(tmp.date.month, tmp.date.year));
  //and keep going back until we hit the correct month
  while(tmp.date.month != sched->month)
    dateadd(&tmp.date, days_in_month(tmp.date.month, tmp.date.year));
  //get correct day
  get_nth_sunday(sched->nth_sunday, &tmp.date);
  *datetime = tmp;
}

struct dst_check_result {
  bool change;
  bool dst_state;
  int8_t hour_change;
  bool operator==(const dst_check_result &d) const {
    return change == d.change &&
           dst_state == d.dst_state &&
           hour_change == d.hour_change;
  }
};

dst_check_result get_dst_change(datetime_data* data, dst_change_schedule* enter, dst_change_schedule* exit, bool currently_enabled) {
  datetime_data dst_enter_date = *data;
  datetime_data dst_exit_date = *data;
  dst_previous_date(enter, true, &dst_enter_date);
  dst_previous_date(exit, false || !currently_enabled, &dst_exit_date);

  bool expected_dst_state = dst_enter_date > dst_exit_date;
  if(expected_dst_state != currently_enabled) {
    return (dst_check_result){true, expected_dst_state, expected_dst_state ? (int8_t)1 : (int8_t)-1};
  }
  return (dst_check_result){false, false, 0};
}

datetime_data get_next_dst_change(datetime_data* datetime, bool current_state, dst_change_schedule* enter, dst_change_schedule* exit) {
  datetime_data dst_date = *datetime;
  if(current_state) {
    dst_next_date(exit, false, &dst_date);
  } else {
    dst_next_date(enter, true, &dst_date);
  }
  return dst_date;
  /*datetime_data dst_enter_date = current_datetime;
  datetime_data dst_exit_date = current_datetime;
  dst_next_date(storage->data.dst_enter.month, storage->data.dst_enter.nth_sunday, true, dst_enter.date);
  dst_next_date(storage->data.dst_exit.month, storage->data.dst_exit.nth_sunday, false, dst_exit_date);
  if(dst_enter_date < dst_exit_date)
    return dst_enter_date;
  return dst_exit_date;*/
}

datetime_data current_datetime;
datetime_data next_dst_change;

datetime_data get_next_dst_change() {
  return get_next_dst_change(&current_datetime, storage->data->in_dst, &storage->data->dst_enter, &storage->data->dst_exit);
}


#define DS1302_CE 0
#define DS1302_IO 1
#define DS1302_SCLK 2

#define DS1302_DELAY 2

class DS1302 {
  public:
    DS1302() {
      gpio_init(DS1302_CE);
      gpio_put(DS1302_CE, 0);
      gpio_set_dir(DS1302_CE, GPIO_OUT);
      gpio_init(DS1302_IO);
      gpio_put(DS1302_IO, 1);
      gpio_set_dir(DS1302_IO, GPIO_IN);
      gpio_init(DS1302_SCLK);
      gpio_put(DS1302_SCLK, 0);
      gpio_set_dir(DS1302_SCLK, GPIO_OUT);
      init_clock();
    }
    void init_clock() {
      rtc_data data;
      read(0xBF, (uint8_t*)&data, 8);
      if(data.wp & 0x80) // disable write protect
        write(0x8E, 0x00);
      if(data.second & 0x80) // disable clock halt
        write(0x80, data.second & 0x7F);
      if(data.hour & 0x80) // disable 12h time
        write(0x84, data.hour & 0x7F);
    }
    void read_time(datetime_data* data) {
      rtc_data rtc;
      read(0xBF, (uint8_t*)&rtc, 8);
      *data = rtc_to_datetime(rtc);
    }
    void read_time() {
      read_time(&current_datetime);
    }
    void write_time(datetime_data* data) {
      rtc_data rtc;
      rtc = datetime_to_rtc(*data);
      write(0xBE, (uint8_t*)&rtc, 8);
    }
    void write_time() {
      write_time(&current_datetime);
    }
    void set_io_out(bool output) {
      if(output) {
        gpio_set_dir(DS1302_IO, GPIO_OUT);
      } else {
        gpio_set_dir(DS1302_IO, GPIO_IN);
      }
    }
    void clock_out(uint8_t byte) {
      for(uint8_t i = 0; i < 8; i++) {
        set_io_out(byte & (0x1 << i));
        sleep_us(DS1302_DELAY);
        gpio_put(DS1302_SCLK, true);
        sleep_us(DS1302_DELAY);
        gpio_put(DS1302_SCLK, false);
      }
      gpio_set_dir(DS1302_IO, GPIO_IN);
    }
    uint8_t clock_in() {
      uint8_t byte = 0;
      for(uint8_t i = 0; i < 8; i++) {
        sleep_us(DS1302_DELAY);
        byte |= gpio_get(DS1302_IO) << i;
        gpio_put(DS1302_SCLK, true);
        sleep_us(DS1302_DELAY);
        gpio_put(DS1302_SCLK, false);
      }
      return byte;
    }
    void write(uint8_t addr, uint8_t* val, uint8_t bytes) {
      sleep_ms(4);
      gpio_put(DS1302_CE, 1);
      sleep_ms(4);
      clock_out(addr);
      while(bytes>0) {
        clock_out(*val);
        val++;
        bytes--;
      }
      sleep_ms(4);
      gpio_put(DS1302_CE, 0);
    }
    void write(uint8_t addr, uint8_t val) {
      write(addr, &val, 1);
    }
    void read(uint8_t addr, uint8_t* val, uint8_t bytes) {
      sleep_us(5);
      gpio_put(DS1302_CE, 1);
      sleep_us(5);
      clock_out(addr);
      while(bytes>0) {
        *val = clock_in();
        val++;
        bytes--;
      }
      sleep_us(5);
      gpio_put(DS1302_CE, 0);
    }
    uint8_t read(uint8_t addr) {
      uint8_t byte;
      read(addr, &byte, 1);
      return byte;
    }
    void check_dst(bool update_clock) {
      if(!storage->data->dst_schedule_enabled)
        return;
      dst_check_result result = get_dst_change(&current_datetime, &storage->data->dst_enter, &storage->data->dst_exit, storage->data->last_dst_change);
      if(result.change) {
        storage->data->last_dst_change = result.dst_state;
        storage->data->in_dst = result.dst_state;
        storage->store();
        if(update_clock) {
          timeadd(&current_datetime.time, result.hour_change, 0, 0, true);
          write_time();
        }
      }
      next_dst_change = get_next_dst_change();
    }
    void check_dst() {
      check_dst(true);
    }
};

DS1302* ds1302;

time_data get_time() {
  return current_datetime.time;
}

date_data get_date() {
  return current_datetime.date;
}

datetime_data schedules_times[MAX_SCHEDULES];

void schedule_next_time(schedule_data* sched, schedule_calc_data* datetime, datetime_data* compare_time) {
  datetime->datetime.time.second=0;
  datetime->datetime.time.minute=sched->minute;
  datetime->datetime.time.hour=sched->hour;
  datetime->datetime.date = compare_time->date;
  datetime->duration = sched->duration;

  if(!(sched->day_of_week & 0x7F) || !sched->enabled) {
    dateadd(&datetime->datetime.date, 8);
    return;
  }
  if(datetime->datetime.time < compare_time->time)
    dateadd(&datetime->datetime.date, 1);

  while(!((0b01000000 >> datetime->datetime.date.day_of_week) & sched->day_of_week))
    dateadd(&datetime->datetime.date, 1);
}

void schedule_previous_time(schedule_data* sched, schedule_calc_data* datetime, datetime_data* compare_time) {
  datetime->datetime.time.second=0;
  datetime->datetime.time.minute=sched->minute;
  datetime->datetime.time.hour=sched->hour;
  datetime->datetime.date = compare_time->date;
  datetime->duration = sched->duration;

  if(!(sched->day_of_week & 0x7F) || !sched->enabled) {
    dateadd(&datetime->datetime.date, -8);
    return;
  }
  if(datetime->datetime.time > compare_time->time)
    dateadd(&datetime->datetime.date, -1);

  while(!((0b01000000 >> datetime->datetime.date.day_of_week) & sched->day_of_week))
    dateadd(&datetime->datetime.date, -1);
}

void build_next_schedule(schedule_calc_data data[]) {
  for(uint8_t i = 0; i < storage->data->schedule_count; i++)
    schedule_next_time(&storage->data->schedules[i], data+i, &current_datetime);
}

void build_previous_schedule(schedule_calc_data data[]) {
  for(uint8_t i = 0; i < storage->data->schedule_count; i++)
    schedule_previous_time(&storage->data->schedules[i], data+i, &current_datetime);
}
