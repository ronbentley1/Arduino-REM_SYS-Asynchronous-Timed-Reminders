// Copyright (c) Ron D Bentley (UK), see copyright notice

// This segment comprises the timer setups and ISRs that underpin this framework.
// They are used to provide the timimg for both ETRs and RTRs.
//
// timer0 and timer2 are used as the source timer for processing ETRs, and
// timer1 is used as the source timer for processing RTRs.
//
// Note that the timer frequency for RTR processing is fixed (timer1).
// However, the frequency for processing ETRs is end user configurable (see User Guide).
//
// **************************************************************************************
// timer initialisation routines for timer0, timer1 and timer2:
//
// ETR timer:
// Set up timer0 interrupt for processimg ETRs for 1000 cycles per second,
// 1kHz interrupts, ie for 1 millisecond interrupts
void initialise_timer0() {
  //set timer0 interrupt at 1kHz
  noInterrupts();
  TCCR0A = 0;
  TCCR0B = 0;
  TCNT0  = 0;
  // set compare match register for 1khz increments
  OCR0A = 249;            // (16*10^6) / (1000*64) - 1 (must be <256)
  // turn on CTC mode
  TCCR0A |= (1 << WGM01);
  // Set CS01 and CS00 bits for 64 prescaler
  TCCR0B |= (1 << CS01) | (1 << CS00);
  // enable timer compare interrupt
  TIMSK0 |= (1 << OCIE0A);
  interrupts();
}

// RTR timer:
// Set up timer1 interrupt for processing RTRs for 2 cycles per second,
// 2Hz interrupts, ie for 1/2 second interrupts
//
void initialise_timer1() {
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  // set Compare Match Register (CMR) for 1msec (2hz) increments
  OCR1A = 31249;                          // (16,000,000)/(256*2)) - 1
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);                 // WGM12 = 3
  // Set CS11 bit for 256 prescaler
  //TCCR1B |= (1 << CS11) | (1 << CS10);  // CS11 = 1 and CS10 = 0
  TCCR1B |= (1 << CS12);                  // CS12 = 2
  // enable timer1 compare interrupt
  TIMSK1 |= (1 << OCIE1A);                // OCIE1A = 1
  interrupts();
}
//
// ETR timer:
// Alternative source timer for processimg ETRs for 1000 cycles per second,
// 1kHz interrupts, ie for 1 millisecond interrupts.
// This is provided if end user needs to use delay() function which makes
// use of timer0, hence a possible conflict.  End user encouraged not to
// use delay()!
void initialise_timer2() {
  noInterrupts();
  // Clear registers
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
  // 1000 Hz (16000000/((124+1)*128))
  OCR2A = 124;
  // CTC
  TCCR2A |= (1 << WGM21);
  // Prescaler 128
  TCCR2B |= (1 << CS22) | (1 << CS20);
  // Output Compare Match A Interrupt Enable
  TIMSK2 |= (1 << OCIE2A);
  interrupts();
}
//
//  ***************************************************************************
//  timer0, 1 and 2 ISRs:
//
//  timer0 is used as the timing source for ETR processing exclusively.
//  timer0 interrupt routine will be entered at ETR_timer_freq interrupt frequency
//  defined during the initialisation of timer0.
//  However, processing of the Remind List will only occur every R_List_scan_feq 'ticks'
//  of timer0.
//
ISR(TIMER0_COMPA_vect) {
  static int timer0_tick = 0;   // counts out the defined R_list scan frequency for ETRs.
  if (!reminders_suspended)
  {
    if (timer0_tick == ETR_R_list_scan_freq) {
      timer0_tick = 0;          // reset for next timer1 interrupt
      // scan ETR entries in reminder list
      scan_R_list(ETR);
    }
    else timer0_tick++;
  }
}

//  Timer1 is used as the timing source for RTR processing exclusively.
//  Timer1 is initialised for 2 cycles per second (2hz) so is entered
//  every 1/2 second.
//
ISR(TIMER1_COMPA_vect) {
  if (!reminders_suspended)
  {
    // scan RTR entries in reminder list
    interrupts(); // not normally the right thing, but this works and allows RTC lib to be accessed
    RTR_Processor();
  }
}

//  timer2 is an alternative timer interrupt routine to timer0.
//  It is offered to replace timer0 if end user wishes to use the delay() function!
//  It is used as the timing source for ETR processing exclusively.
//  timer2 interrupt routine will be entered at the ETR_timer_freq interrupt frequency
//  defined during the initialisation of timer2.
//  However, processing of the Remind List will only occur every R_List_scan_feq 'ticks'
//  of timer2.
//
ISR(TIMER2_COMPA_vect) {
  static int timer2_tick = 0;   // counts out the defined R_list scan frequency for ETRs.
  if (!reminders_suspended)
  {
    if (timer2_tick == ETR_R_list_scan_freq) {
      timer2_tick = 0;          // reset for next timer1 interrupt
      // scan ETR entries in reminder list
      scan_R_list(ETR);
    }
    else timer2_tick++;
  }
}

// The framework supports one of two timers for processing ETRs, either timer0 or timer2.
// Both timers are initialised for 1khz (1000 cycles per second, or ETR_timer_freq).
// If end user wishes to use delay() function, choose to initialise timer2 not timer0.
void initialise_timers() {
  if (ETR_timer_number == 0) initialise_timer0();
  else initialise_timer2();
  initialise_timer1();
}

//
//  Routine sets the reminders_suspended flag to halt reminder list scans by ISR.
//
void suspend_reminders() {
  noInterrupts();
  reminders_suspended = true;
  interrupts();
}

//
//  Routine clears the reminders_suspended flag to resume reminder list scans by ISR.
//
void resume_reminders() {
  noInterrupts();
  reminders_suspended = false;
  interrupts();
}
