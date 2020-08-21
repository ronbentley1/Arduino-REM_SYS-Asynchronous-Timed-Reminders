/* Ron D Bentley (UK) (c) 2020
   This tab contains additional routines (and more) to support real-time reminders
   these being in addition to the base framework which privides elapsed time reminder (ETR)
   functionality.
*/
//****************************************************************************************
//                                    Date Functions
//****************************************************************************************
// Define the start date (origin) for elapsed days calculations.
// There is a constraint on the origin date:
//    1. it MUST be of the form 01/01/LEAP_YEAR(or pseudo Leap year, eg divisible by 100)
//    2. Also, if this is changed then change day_of_week_offset too.
//
int origin_day   = 1;
int origin_month = 1;
int origin_year  = 2020;

int today_day_number;   // used to keep a note of the day_number for today in midnight process
int today_day_of_week;  // used to keep a note of the day_of_week for today in midnight process

#define day_of_week_offset  2 // offset value for referencing days_of_week array as
// 1/1/2020 is a Wednesday, so offset provides
// correcting adjustment for day_of_week function,
// as day_number(1,1,2020) gives 1
#define valid_date     1
#define invalid_day   -1
#define invalid_month -2
#define invalid_year  -3

#define Jan            1
#define Feb            2
#define Mar            3
#define Apr            4
#define May            5
#define Jun            6
#define Jul            7
#define Aug            8
#define Sep            9
#define Oct           10
#define Nov           11
#define Dec           12

#define Sunday         0
#define Monday         1
#define Tuesday        2
#define Wednesday      3
#define Thursday       4
#define Friday         5
#define Saturday       6

char days_of_week[7][12] =
{
  "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};
byte days_in_month[13] =
{
  0,  // entry 0 not used
  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31        //  Jan, Feb, March, etc
};
int accumulated_days_in_month[13] =
{
  0, // entry 0 not used
  0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 // Jan, Feb, March, etc
};
//
// Checks the given day,month,year being a valid date
//
int check_date(int day, int month, int year) {
  int Days_in_month;
  if (year < origin_year) {
    return invalid_year;
  }
  if (month < Jan || month > Dec) {
    return invalid_month;
  }
  Days_in_month = days_in_month[month];
  if (month == Feb && leap_year(year)) {
    Days_in_month++; // adjust for leap year
  }
  if (day < 1 || day > Days_in_month) {
    return invalid_day;
  }
  return valid_date;
}
//
// leap_year will return whether the given year is a leap year
// and takes into account the leap year rule:
//    if divisible by   4 then it IS a leap year, UNLESS
//    divisible by 100 then it is NOT a leap year, UNLESS
//    divisible by 400 the it IS a leap year.
//
int leap_year(int year) {
  if (year % 400 == 0) {
    return true;
  }
  if (year % 100 == 0) {
    return false;
  }
  if (year % 4   == 0) {
    return true;
  }
  return false;
}
//
// day_number will return the number of inclusive elapsed days from the
// origin date of the given parameter date, taking ino account leap years.
//
int day_number(int day, int month, int year) {
  int num_leaps, yr;
  // start by counting the number of leap years from the origin year
  // to the given year.
  num_leaps = 0;
  for (yr = origin_year; yr <= year; yr = yr + 4) {
    if (leap_year(yr)) num_leaps++;
  }
  // final adjustment if given year being a leap
  // year and given month is Jan or Feb.
  if (month < Mar && leap_year(year)) {
    num_leaps--;
  }
  // now calculate elapsed days...
  return (year - origin_year) * 365
         + accumulated_days_in_month[month]
         + day
         + num_leaps;
}
//
// Given a day_number, function will determine the date as day, month, year.
//
void date_from_day_number(int day_number, int &day, int &month, int &year) {
  int days_in_year, Days_in_month;
  // determine what year the day_number falls in, count up from origin_year
  // in full years as days.
  year = origin_year;
  if (leap_year(year)) {
    days_in_year = 366;
  } else {
    days_in_year = 365;
  }
  do {
    if (day_number > days_in_year) {
      // move year on
      day_number = day_number - days_in_year;
      year++;
      if (leap_year(year)) {
        days_in_year = 366;
      } else {
        days_in_year = 365;
      }
    }
  }
  while (day_number > days_in_year);
  // now step through days_in_month to determine month.  What is
  // left as the remainder is the day of the month, and we are done.
  month = 1;
  Days_in_month = days_in_month[1];
  while (month <= Dec && day_number > Days_in_month) {
    if (month == Feb && leap_year(year)) {
      Days_in_month = 29;
    } else {
      Days_in_month = days_in_month[month];
    }
    day_number = day_number - Days_in_month;
    month++;
    if (month <= Dec) { // this set up for next while cycle test condition
      Days_in_month = days_in_month[month];
      if (month == Feb && leap_year(year)) {
        Days_in_month++;
      }
    }
  }
  day = day_number; // what is left in day_number is now that day of the month
}
//
// returns the index for the day of the week in the days_of_week array
//
int day_of_week(int day, int month, int year) {
  return (day_number(day, month, year)
          + day_of_week_offset) % 7; // map to 0 (Sunday), 1 (Monday), ... , 6 (Saturday)
}

//******************************************************************************************
//                  date & time funcions required for other ETR/RTR code
//                                  DO NOT REMOVE
//******************************************************************************************
//  RTC libraries and declarations
#include <RTClib.h>

RTC_DS1307 rtc;

void display_now_date_time() {
  if (RTC_enabled) {
    DateTime now = rtc.now();
    Serial.print(now.day());
    Serial.print(F("/"));
    Serial.print(now.month());
    Serial.print(F("/"));
    Serial.print(now.year());
    Serial.print(F(", "));
    Serial.print(days_of_week[now.dayOfTheWeek()]);
    Serial.print(F(", "));
    Serial.print(now.hour());
    Serial.print(F(":"));
    Serial.print(now.minute());
    Serial.print(F(":"));
    Serial.println(now.second());
    Serial.flush();
  }
}

//
// Function returns the number of seconds since midnight.
long unsigned int seconds_since_midnight() {
  long unsigned int secs, mins, hrs;
  if (RTC_enabled) {
    if (rtc.isrunning()) {
      DateTime now = rtc.now();
      hrs  = now.hour();
      mins = now.minute();
      secs = now.second();
      return hrs * 3600 + mins * 60 + secs;
    }  else  {
      Serial.println(F("!secs_since_midnight - RTC is not operating, terminating!"));
      Serial.flush();
      exit(0);
    }
  } else return 0; // RTC not configured
}
