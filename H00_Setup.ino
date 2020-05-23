// Copyright (c) Ron D Bentley (UK), see copyright notice
//
// This framework is provided with two standing reminders that provide:
//  1.  a heart beat visual monitor which shows that the code is operating.
//      This is configured for pin 13 (on board LED), but do change if necessary.
//  2.  a midnight processor to allow daily processing/housekeeping to be
//      carried out as necessary ech midnight.a
//
int hb_intensity                = 255; // start by setting to max output level
#define heart_beat_pin             13  // digital pin for visible heart beat
#define heart_beat                254  // ETR reminder sub_type for heart beat
#define midnight                  255  // RTR reminder sub_type for midnight processing

//
void setup()
{
  int result;
  if (diags_on) {
    Serial.begin(115200);
    !rtc.begin();
    if (!rtc.isrunning())
    {
      Serial.println("!setup() - RTC is not operating, terminating!");
      Serial.flush();
      exit(0);
    }
  }
  //  Set up 'today_day_number' to provide ready access to this value at any point
  //  in the program without the need for it to be recalculated.
  //  This will get incremented each midnight to stay in step with the current date.
  DateTime now = rtc.now();
  today_day_number = day_number(now.day(), now.month(), now.year());
  today_day_of_week = (today_day_number + day_of_week_offset) % 7; // map to 0 (Sunday), 1 (Monday),.., 6 (Saturday)
  // **********************************************************************************
  // set up the two standing reminders:
  //  1.  the heart beat visual monitor as an ETR, to show processes are operating, and
  //  2.  the midnight RTR each midnight to deal with any daily processing/housekeeping
  // The alerts for each of the above remnders are dealt with in the main segment.
  // **********************************************************************************
  int freq_secs, freq_subsecs;
  pinMode(heart_beat_pin, OUTPUT);
  // set the ETR reminder for the heart beat to be 1/2 second if possible, if not,
  // set to 1 second.
  freq_subsecs = scans_per_sec / 2;                           // take as 1/2 scan rate per second, if possible
  if (freq_subsecs == 0) freq_secs = 1; else freq_secs = 0;
  result = create_ET_reminder(ET_recurring_type, heart_beat,
                              0, 0, 0, 0,                     // start immediately
                              0, 0, freq_secs, freq_subsecs,  // 1/2 or 1 second frequency
                              0, 0, 0, 0,                     // not used
                              0, 0, 0, 0);                    // not used
  if (result < 0 && diags_on) {
    Serial.print("setup() - error creating ETR for heart_beat, error value = ");
    Serial.println(result);
    Serial.flush();
  }
  //  set up RTR for each midnight, so that any daily processing can be performed
  result = create_RT_reminder(RT_recurring_type, midnight,
                              0, 0, 0,      // start at midnight (00:00:00)
                             24, 0, 0,      // repeat each midnight (24 hrs)
                              0, 0, 0,      // not used
                              0, 0, 0, 0);  // not used
  if (result < 0 && diags_on) {
    Serial.print("setup() error creating RTR for midnight, error value = ");
    Serial.println(result);
    Serial.flush();
  }
  //  ******************************************************************
  //  insert all initial reminder ETR/RTR create remider requests here,
  //  before the timers and remind queues are initialised
  //  ******************************************************************


  //  ******************************************************************
  //  create free chain and initialise timers (0 or 2 and 1)
  //  ******************************************************************
  create_RQ_free_chain();   //  create free chain of reminder queue blocks
  suspend_reminders();      //  ensure timers do not action the reminder list yet
  initialise_timers();
  resume_reminders();       //  allow timer scans of the reminder list
}
