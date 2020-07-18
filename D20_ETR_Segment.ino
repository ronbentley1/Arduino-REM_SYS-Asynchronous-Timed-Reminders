// Copyright (c) Ron D Bentley (UK), see copyright notice
//
//  Scan the reminder list to process active reminder entries (ETR and RTR types).
//
void scan_R_list(int E_or_R) {
  int R_entry, R_type, R_status;
  //  scan each possible reminder list entry and process if active according to reminder type
  for (R_entry = 0; R_entry < max_R_list_entries; R_entry++) {
    if (R_list[R_entry].R_type != inactive) {
      // this reminder entry is active
      R_type = R_list[R_entry].R_type;
      R_status = 0;    //  assume the timed remider is not the final one, for now
      // process ETR or RTR entries depending on value of the parameter
      if (E_or_R == ETR && R_type >= ET_oneoff_type && R_type <= ET_repeat_duration_type) {
        // process ETR reminder entry
        if (R_list[R_entry].R_count_down > 0) {
          R_list[R_entry].R_count_down--; // decrement countdown timer
        }
        if (R_list[R_entry].R_count_down == 0) {
          //  count down time elapsed so time to raise the reminder via the
          //  RQ for end user asynchronous processing
          //R_status = 0;    //  assume the timed remider is not the final one, for now
          switch (R_type) {
            case ET_oneoff_type:
              // 'oneoff' reminder type so remove this reminder as it has elapsed
              R_list[R_entry].R_type = inactive;
              R_status = final_alert;
              break;
            case ET_recurring_type:
              //  'recurring' reminder type so reset counter for next wait cycle
              R_list[R_entry].R_count_down = R_list[R_entry].R_freq;
              break;
            case ET_repeat_duration_type:
              if (R_list[R_entry].R_duration == 0)
              { //  no further reminders are due of this reminder type
                //  ('remind_duration') time has now elapsed so remove this reminder
                R_list[R_entry].R_type = inactive;
                R_status = final_alert;
              } else {
                //  'repeat_duration' type, so reset counter for next cycle
                if (R_list[R_entry].R_duration < R_list[R_entry].R_freq) {
                  // less time left than defined by R_freq, so take lesser value
                  R_list[R_entry].R_count_down = R_list[R_entry].R_duration;
                  R_list[R_entry].R_duration = 0;
                } else {
                  //  reduce end time by R_freq
                  if (R_list[R_entry].R_freq == 0) {
                    R_list[R_entry].R_count_down = R_list[R_entry].R_duration;
                    R_list[R_entry].R_duration = 0;
                  } else {
                    R_list[R_entry].R_duration   = R_list[R_entry].R_duration - R_list[R_entry].R_freq;
                    R_list[R_entry].R_count_down = R_list[R_entry].R_freq;
                  }
                }
              }
              break;
          }
          // assemble R_status to be a compound value to store whether this is a final alert
          // for this reminder or not, together with the reminder type and subtype.
          // These calues will be unpicked at set to global variables once the reminder
          // has triggered and scan_RQ takes it off the RQ. This saves a little on space.
          R_status = (R_status << 15) + (R_type << 8) + R_list[R_entry].R_subtype;
          if (insert_into_RQ(R_status,
                             R_list[R_entry].R_user1,
                             R_list[R_entry].R_user2,
                             R_list[R_entry].R_user3,
                             R_list[R_entry].R_user4) == fail) {
            if (diags_on) {
              Serial.println(F("\n!!scan_list ETR insert failure!!"));
              Serial.flush();
            }
          }
        }
      }
      else if (E_or_R == RTR && R_type >= RT_oneoff_type && R_type <= RT_repeat_duration_type) {
        // process RTR reminder entry
        if (R_list[R_entry].R_remind_at == Seconds_since_midnight) {
          switch (R_type) {
            case RT_oneoff_type:
              R_list[R_entry].R_type = inactive; // reminder has now expired so clear it
              R_status = final_alert;
              break;
            case RT_recurring_type:
              // perform modulo 24 hours arithmetic on next reminder time (86400 = 24 hrs in seconds)
              R_list[R_entry].R_remind_at = (R_list[R_entry].R_remind_at + R_list[R_entry].R_freq) % 86400;
              break;
            case RT_repeat_duration_type:

              if (R_list[R_entry].R_duration == 0) {
                //  no further reminders are due of this reminder type
                //  ('remind_duration') time has now elapsed so remove this reminder
                R_list[R_entry].R_type = inactive;
                R_status = final_alert;
              } else {
                //  'repeat_duration' type, so reset counter for next cycle
                if (R_list[R_entry].R_duration < R_list[R_entry].R_freq) {
                  // less time left than defined by R_freq, so take lesser value
                  R_list[R_entry].R_remind_at = (R_list[R_entry].R_remind_at + R_list[R_entry].R_duration) % 86400;
                  R_list[R_entry].R_duration = 0;
                } else {
                  //  reduce end time by R_freq
                  if (R_list[R_entry].R_freq == 0) {
                    R_list[R_entry].R_remind_at = (R_list[R_entry].R_remind_at + R_list[R_entry].R_duration) % 86400;
                    R_list[R_entry].R_duration  = 0;
                  } else {
                    R_list[R_entry].R_duration  = R_list[R_entry].R_duration - R_list[R_entry].R_freq;
                    R_list[R_entry].R_remind_at = (R_list[R_entry].R_remind_at + R_list[R_entry].R_freq) % 86400;
                  }
                }
              }
              break;
          }
          R_status = (R_status << 15) + (R_type << 8) + R_list[R_entry].R_subtype;
          if (insert_into_RQ(R_status,
                             R_list[R_entry].R_user1,
                             R_list[R_entry].R_user2,
                             R_list[R_entry].R_user3,
                             R_list[R_entry].R_user4) == fail) {
            if (diags_on) {
              Serial.println(F("\n!!scan_list RTR insert failure!!"));
              Serial.flush();
            }
          }
        }
      }
    }
  }
}

//
//  This routine drives the Real-Time scan process and is initiated via timer0 interrupts
//
int RTR_Processor() {
  static  int last_seconds = 60; // a start value that now_seconds cant achieve
  static int now_second = 0;
  if (rtc.isrunning()) {
    DateTime now = rtc.now();
    now_second = now.second();   // get RTC second now
    if (now_second != last_seconds) {
      // must have moved on by at least 1 second so scan RT reminder list entries
      last_seconds = now_second; // ready for next pass
      Seconds_since_midnight = seconds_since_midnight(); // used in scan_R_list for RTR checking
      scan_R_list(RTR);
      return 1;                  // return that remind list was scanned this pass
    }
  } else  {
    Serial.println(F("!RTR_Processor - RTC not operating, terminating!"));
    Serial.flush();
    exit(0);
  }
  return 0;                      // return that remind list not scanned this pass
}

//
//  create_ET_reminder() - routine creates a new Elapsed Time (ET) reminder in R_list if
//  there is space and parameters are valid!
//
//  Paramerters in scope, by R_type:
//  one_off, R_type = 1
//  R_start_in: Yes
//  R_freq:     n/a, ignored if set
//  R_duration: n/a, ignored if set
//
//  recurring, R_type = 2
//  R_start_in: Yes
//  R_freq:     Yes
//  R_duration: n/a, ignored if set
//
//  for a duration, R_type = 3
//  R_start_in: Yes
//  R_freq:     Yes
//  R_duration: Yes
//
//  Note that:
//  1.  the parameters for start, frequency and for (duration) all follow same format:
//      hrs, mins, secs, subsecs
//  2.  the subsecs value is the number of cycles per second the remind list is scanned.
//      It is 0..'scans_per_sec'-1.  'scans_per_sec' is 'timer1_freq' / 'ETR_R_list_scan_freq'.
//
int create_ET_reminder(int R_type, int R_subtype,
                       long signed int R_start_in_hrs,   long signed int R_start_in_mins,
                       long signed int R_start_in_secs,  long signed int R_start_in_subsecs,
                       long signed int R_freq_hrs,       long signed int R_freq_mins,
                       long signed int R_freq_secs,      long signed int R_freq_subsecs,
                       long signed int R_duration_hrs,   long signed int R_duration_mins,
                       long signed int R_duration_secs,  long signed int R_duration_subsecs,
                       int R_user1, int R_user2, int R_user3, int R_user4) {
  int R_entry, R_list_status;
  long signed int R_start_in, R_freq, R_duration;
  //  start by validating the parameters
  if (R_type < ET_oneoff_type || R_type > ET_repeat_duration_type) {
    return invalid_R_type;
  }
  if (R_subtype < 0 || R_subtype > 255) {
    return invalid_R_subtype;
  }
  // R_start_in is required for all R_types, so validate
  if (R_start_in_mins < 0 || R_start_in_mins > 59) {
    return invalid_start_in_mins;
  }
  if (R_start_in_secs < 0 || R_start_in_secs > 59) {
    return invalid_start_in_secs;
  }
  if (R_start_in_subsecs < 0 || R_start_in_subsecs >= scans_per_sec) {
    return invalid_start_in_subsecs;
  }
  //  validate R_freq
  if (R_type != ET_oneoff_type) {
    //  R_frequency is required for recurring_type or repeat_duration_type, so validate
    //  and set up the interval count
    if (R_freq_mins < 0 || R_freq_mins > 59) {
      return invalid_freq_mins;
    }
    if (R_freq_secs < 0 || R_freq_secs > 59) {
      return invalid_freq_secs;
    }
    if (R_freq_subsecs < 0 || R_freq_subsecs >= scans_per_sec) {
      return invalid_freq_subsecs;
    }
    R_freq_secs = R_freq_secs + timer_drift_adjustment * R_freq_hrs; // add the number of secs per hour adjustment

    R_freq =  R_freq_hrs   * scans_per_hr  +
              R_freq_mins  * scans_per_min +
              R_freq_secs  * scans_per_sec +
              R_freq_subsecs;
    if (R_type == ET_recurring_type && R_freq == 0) {
      return invalid_freq;
    }
  } else {
    R_freq = 0;
  }
  //  validate R_duration time if R_type is repeat_duration_type
  if (R_type == ET_repeat_duration_type) {
    if (R_duration_mins < 0 || R_duration_mins > 59) {
      return invalid_duration_mins;
    }
    if (R_duration_secs < 0 || R_duration_secs > 59) {
      return invalid_duration_secs;
    }
    if ( R_duration_subsecs < 0 || R_duration_subsecs >= scans_per_sec) {
      return invalid_duration_subsecs;
    }
    R_duration_secs = R_duration_secs + timer_drift_adjustment * R_duration_hrs; // add the number of secs per hour adjustment
    R_duration =  R_duration_hrs * scans_per_hr    +
                  R_duration_mins * scans_per_min  +
                  R_duration_secs * scans_per_sec  +
                  R_duration_subsecs;
    if (R_freq > R_duration) {
      return invalid_freq;
    }
  } else {
    R_duration = 0;
  }
  R_start_in_secs = R_start_in_secs + timer_drift_adjustment * R_start_in_hrs; // add the number of secs per hour adjustment
  R_start_in = R_start_in_hrs * scans_per_hr    +
               R_start_in_mins * scans_per_min  +
               R_start_in_secs * scans_per_sec  +
               R_start_in_subsecs;
  //  now look for an empty slot...
  R_entry = 0; //  start with first reminder entry and scan until empty entry found, or not
  do {
    // R_list_status=R_list[R_entry].R_type;
    if ( R_list[R_entry].R_type == inactive) {
      //  this entry is not used, so set up it...
      R_list[R_entry].R_subtype     = R_subtype;
      R_list[R_entry].R_start_in    = R_start_in;     //  used for all R_types
      R_list[R_entry].R_freq        = R_freq;         //  only used if R_type is recurring_type or repeat_duration_type
      R_list[R_entry].R_duration    = R_duration;     //  only used if R_type is repeat_duration_type
      R_list[R_entry].R_count_down  = R_start_in;
      //  End user variables held in a reminder entry
      R_list[R_entry].R_user1 = R_user1;
      R_list[R_entry].R_user2 = R_user2;
      R_list[R_entry].R_user3 = R_user3;
      R_list[R_entry].R_user4 = R_user4;
      //  set this reminder as active
      noInterrupts();
      R_list[R_entry].R_type = R_type;  //  note, setting this last variable will trigger the timer scan
      interrupts();
      return  R_entry;  //  return with the entry number of the reminder
    }
    R_entry++;  // look at next entry
  }
  while ( R_entry < max_R_list_entries);
  return reminder_list_full;
}
//
//  create_RT_reminder() - routine creates a new Real-Time (RT)reminder in R_list if
//  there is space and parameters are valid!
//
//  Paramerters in scope, by R_type:
//  one_off, R_type = 4
//  R_remind_at:  Yes
//  R_freq:       n/a, ignored if set
//  R_duration:   n/a, ignored if set
//
//  recurring, R_type = 5
//  R_remind_at:  Yes
//  R_freq:       Yes
//  R_duration:   n/a, ignored if set
//
//  for a duration, R_type = 6
//  R_remind_at:  Yes
//  R_freq:       Yes
//  R_duration:   Yes
//
//  Note that:
//  1.  the parameters for remind_at, frequency and for (duration) all follow same format:
//      hrs, mins, secs (no subsecs)
//
int create_RT_reminder(int R_type, int R_subtype,
                       long signed int R_remind_at_hrs,  long signed int R_remind_at_mins,
                       long signed int R_remind_at_secs,
                       long signed int R_freq_hrs,       long signed int R_freq_mins,
                       long signed int R_freq_secs,
                       long signed int R_duration_hrs,   long signed int R_duration_mins,
                       long signed int R_duration_secs,
                       int R_user1, int R_user2, int R_user3, int R_user4) {
  int R_entry;
  long signed int R_remind_at, R_freq, R_duration;
  //  start by validating the parameters
  if (R_type < RT_oneoff_type || R_type > RT_repeat_duration_type) {
    return invalid_R_type;
  }
  if (R_subtype < 0 || R_subtype > 255) {
    return invalid_R_subtype;
  }
  if (R_remind_at_hrs  < 0 || R_remind_at_hrs  > 23) {
    return invalid_RT_hrs;
  }
  if (R_remind_at_mins < 0 || R_remind_at_mins > 59) {
    return invalid_RT_mins;
  }
  if (R_remind_at_secs < 0 || R_remind_at_secs > 59) {
    return invalid_RT_secs;
  }
  //  validate R_freq
  if (R_type != RT_oneoff_type) {
    //  R_frequency is required for recurring_type or repeat_duration_type, so validate
    //  and set up the interval count
    if (R_freq_mins < 0 || R_freq_mins > 59) {
      return invalid_freq_mins;
    }
    if (R_freq_secs < 0 || R_freq_secs > 59) {
      return invalid_freq_secs;
    }
    R_freq =  R_freq_hrs * 3600 +
              R_freq_mins * 60 +
              R_freq_secs;
    if (R_type == RT_recurring_type && R_freq == 0) {
      return invalid_freq;
    }
  } else {
    R_freq = 0;
  }
  //  validate R_duration time if R_type is repeat_duration_type
  if (R_type == RT_repeat_duration_type) {
    if (R_duration_mins < 0 || R_duration_mins > 59) {
      return invalid_duration_mins;
    }
    if (R_duration_secs < 0 || R_duration_secs > 59) {
      return invalid_duration_secs;
    }
    R_duration =  R_duration_hrs * 3600 +
                  R_duration_mins * 60 +
                  R_duration_secs;
    if (R_freq > R_duration) {
      return invalid_freq;
    }
  } else {
    R_duration = 0;
  }
  // now calculate the numer of seconds since last midnight of this real-time
  R_remind_at = R_remind_at_hrs   * 3600  +
                R_remind_at_mins  * 60    +
                R_remind_at_secs ;
  //  now look for an empty slot...
  R_entry = 0; //  start with first reminder entry and scan until empty entry found, or not
  do {
    if ( R_list[R_entry].R_type == inactive) {
      //  this entry is not used, so set up it...
      R_list[R_entry].R_subtype     = R_subtype;
      R_list[R_entry].R_remind_at   = R_remind_at;    // real-time of reminder in secs from last midnight
      R_list[R_entry].R_freq        = R_freq;         // only used if R_type is recurring_type or repeat_duration_type
      R_list[R_entry].R_duration    = R_duration;     // only used if R_type is repeat_duration_type
      R_list[R_entry].R_count_down  = 0;              // not used for RTRs
      //  End user variables held in a reminder entry
      R_list[R_entry].R_user1 = R_user1;
      R_list[R_entry].R_user2 = R_user2;
      R_list[R_entry].R_user3 = R_user3;
      R_list[R_entry].R_user4 = R_user4;
      //  set this reminder as active
      noInterrupts();
      R_list[R_entry].R_type = R_type;  //  note, setting this last value/field will trigger the scan process
      interrupts();
      return  R_entry;  //  return with the entry number of the reminder
    }
    R_entry++;  // look at next entry
  }
  while ( R_entry < max_R_list_entries);
  return reminder_list_full;
}

//
//  Routine deletes a reminder entry of the specified parameters
//  That is, for the given R_id and R_id_subtype values
//  if 'lockout' true then interrupts are first disabled and then reenabled
//  otherwise.
//  if called from within a routine where interrupts are disabled, eg and
//  interrupt routine then 'lockout' should be set to false as interrupts
//  will already be disabled.
//
int delete_reminder(int R_type, int R_subtype) {
  int R_entry;
  noInterrupts();
  for (R_entry = 0; R_entry < max_R_list_entries; R_entry++) {
    if ( R_list[R_entry].R_type != inactive) {
      if (R_type == R_list[R_entry].R_type && R_subtype == R_list[R_entry].R_subtype) {
        //  match on this entry, so remove it, ie make inactive
        R_list[R_entry].R_type = inactive;
        interrupts();
        return success;
      }
    }
  }
  interrupts();
  return fail;
}

//
//  Print given Reminder details
//
void print_reminder(int R_entry) {
  int R_type;
  long unsigned int R_remind_at, hrs, mins, secs;
  noInterrupts();
  if (R_list[R_entry].R_type != inactive) {
    Serial.println(F("============ Reminder Parameters ================"));
    Serial.print(F("Reminder entry no: ")); Serial.println(R_entry);
    R_type = R_list[R_entry].R_type;
    Serial.print(F("R_type: ")); Serial.print(R_type);
    Serial.print(F("  R_subtype:  ")); Serial.println(R_list[R_entry].R_subtype);
    if (R_type >= ET_oneoff_type && R_type <= ET_repeat_duration_type) {
      Serial.print(F("R_start_in: ")); Serial.print(R_list[R_entry].R_start_in);
      Serial.print(F(" "));
    } else {
      R_remind_at = R_list[R_entry].R_remind_at;
      hrs = R_remind_at / 3600;
      secs = R_remind_at % 3600;
      mins = secs / 60;
      secs = secs % 60;
      Serial.print(F("R_remind_at: "));
      Serial.print(hrs); Serial.print(F(":"));
      Serial.print(mins); Serial.print(F(":"));
      Serial.print(secs); Serial.print(F(" ("));
      Serial.print(R_remind_at);
      Serial.println(F(")" ));
    }
    Serial.print(F("R_freq: ")); Serial.print(R_list[R_entry].R_freq);
    Serial.print(F(" R_duration: ")); Serial.println(R_list[R_entry].R_duration);
    Serial.print(F("R_count_down: ")); Serial.println(R_list[R_entry].R_count_down);
    Serial.println(F("============== User Parameters =================="));
    Serial.print(F("R_user1: ")); Serial.println(R_list[R_entry].R_user1);
    Serial.print(F("R_user2: ")); Serial.println(R_list[R_entry].R_user2);
    Serial.print(F("R_user3: ")); Serial.println(R_list[R_entry].R_user3);
    Serial.print(F("R_user4: ")); Serial.println(R_list[R_entry].R_user4);
  } else {
    Serial.print(F("\n\n** Reminder "));
    Serial.print(R_entry);
    Serial.println(F(" is inactive **"));
  }
  Serial.flush();
  interrupts();
}
