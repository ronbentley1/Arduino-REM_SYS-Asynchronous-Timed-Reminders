// Copyright (c) Ron D Bentley (UK), see copyright notice
//
//  main segment for end user code
//  insert end user code where indicated:
//  1.  if timed reminder alert processing is needed then within the 'do{if (scan_RQ){...}'
//      control block, otherwise.  Use swich case on R_subtype to process alerts.
//  2.  for non-reminder requirements, place code within the 'do{if (scan_RQ)..else {..}'
//      control block

void loop() {
  do {
    if (scan_RQ() != no_reminder_requests) {
      //  *******************************************************************************
      //  the following global data variables are available at this point relating to the
      //  reminder triggered/alerted:
      //  1.  R_type          - reminder type, ETRs: 1, 2, or 3, / RTRs: 4, 5, or 6
      //  2.  R_subtype       - reminder subtype (0 <= R_subtype <= 255)
      //  3.  R_status        - set to 1 if this is the FINAL reminder alert, 0 otherwise.
      //                        Only relevant for oneoff and repeat_duration reminder types.
      //  4.  R_user1-R_user4 - user data values set up when the reminder was created
      //
      //  The above variable are those defined when the timed reminder was created.
      //  These can be 'crafted' to control flow and decision processes.
      //
      //  Insert end user code here to process timed reminder alerts by R_subtype.
      //  *******************************************************************************
      switch (R_subtype) {
        case heart_beat:
          //  ********** provides a visual indication that the program is running
          analogWrite(heart_beat_pin, hb_intensity);
          // toggle heart beat output level for next pass
          if (hb_intensity == 255) {
            hb_intensity = 0;
          } else {
            hb_intensity = 255;
          }
          break;
        case midnight:
          // ********* midnight processing. Insert any code relevant for daily housekeeping
          today_day_number++;                              // day_number for today, a new day
          today_day_of_week = (today_day_of_week + 1) % 7; // next day of week value
          break;
        // **********************************************************
        // insert ETR/RTR alert switch case code handling here:



        // default switch value 'catcher'
        default:
          Serial.print("!Spurious switch value=");
          Serial.println(R_subtype);
          Serial.flush();
          display_now_date_time();
          break;
      }
    } else {
      //  ***************************************************************
      //  Reminder queue is currently empty, so do other things..
      //  Insert end user code here for processing non-reminder alerts
      //  ***************************************************************

    }
  } while (true);
}
