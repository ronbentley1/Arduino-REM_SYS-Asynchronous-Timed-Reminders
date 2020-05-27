// Copyright (c) Ron D Bentley (UK), see copyright notice
//
//  ****************************************************************************************************
//  USER CONFIGURABLE PARAMETERS ARE REFERENCED HERE (see User Guide):
//
#define diags_on                        true  // leave set to true whilst developing/testing

int ETR_timer_number                   = 0;   // 0 for timer0, 2 for timer2

#define ETR_R_list_scan_freq            100   //time (msecs) between scans of the ETR list (see User Guide)
//
//  The timer_drift_adjustment variable allows the inaccuracy of timer0/2 to be
//  compensated for, but only as far as the drift per hour in seconds.
//  This is ONLY relevant for ETRs, see User Guide.
long signed int timer_drift_adjustment =  1;  // number of seconds (+/-) per hour to adjust elapsed times

#define max_RQ_free_chain_blocks          16  // size of the RQ free chain in blocks

#define max_R_list_entries                10  // number of reminder list entries
//
// END OF USER CONFIGURABLE DATA/PARAMETERS.
// *******************************************************************************************************
//
#define ETR_timer_freq         1000                            // 1 msec timer interrupt frequency
#define scans_per_sec   ETR_timer_freq / ETR_R_list_scan_freq  // number of R_list scans per second
#define scans_per_min            60 * scans_per_sec            // number of R_list scans per minute
#define scans_per_hr             60 * scans_per_min            // number of R_list scans per hour

//
//  The timer_drift_adjustment variable allows the inaccuracy of timer1 to be
//  compensated for, but only as far as the drift per hour.  See User Guide.
//

volatile long unsigned int Seconds_since_midnight;        // used by RTR scan process

//  ************************************
//  Reminder List variables/definitions:

volatile boolean reminders_suspended = true;  //  allows reminder list scans to be halted/started

#define success                    1
#define fail                      -1

#define inactive                   0          // if a reminder entry R_type is 0 it is inactive
#define final_alert                1
//Reminder types:
//   1-3 are elapsed remider types and
//   4-6 are real-time reminder times
#define ET_oneoff_type              1  // Elapsed time (ETR) R_type values
#define ET_recurring_type           2  //     ....
#define ET_repeat_duration_type     3  //     ....

#define RT_oneoff_type              4  // Real-time R_type (RTR) valiues
#define RT_recurring_type           5  //      ....
#define RT_repeat_duration_type     6  //      .... 

#define ETR                         0  // ETR = Elapsed Time Reminder
#define RTR                         1  // RTR = Real Time Reminder

// definitions used by the create reminder routines
#define invalid_R_type            -1
#define invalid_R_subtype         -2
#define invalid_R_start_in        -3
#define invalid_start_in_mins     -4
#define invalid_start_in_secs     -5
#define invalid_start_in_subsecs  -6
#define invalid_duration_mins     -7
#define invalid_duration_secs     -8
#define invalid_duration_subsecs  -9
#define invalid_freq_mins         -10
#define invalid_freq_secs         -11
#define invalid_freq_subsecs      -12
#define invalid_freq              -13
#define invalid_RT_hrs            -14  // pertinent to real-time reminders only
#define invalid_RT_mins           -15  // ...
#define invalid_RT_secs           -16  // ...
#define reminder_list_full        -99

volatile struct Reminder_List {
  // Reminder List variables for managing reminder entries
  int R_type;     //  R_type = 0 if entry unused,
  int R_subtype;  //  end user definable
  union {
    long unsigned int R_remind_at;  // used for RTR processing
    long unsigned int R_start_in;   // used for ETR processing
  };
  long unsigned int R_freq;
  long unsigned int R_duration;
  long unsigned int R_count_down;
  //  End user variables held in a reminder entry
  int R_user1;
  int R_user2;
  int R_user3;
  int R_user4;
} R_list[max_R_list_entries];

//  ****************************************
//  Reminder Queue variables
#define free_chain_entry_size          6
#define end_of_chain_value            -1

//  When allocated to a triggered/elapsed reminder,a block from the free chain will be allocated
//  to the active RQ and be set up as follows:
//  word [0]          - forward chain pointer, or end of chain value
//  word [1]          - a compound integer that contains:
//                          bit 15      - set to 1 if this entry represents the last reminder alert for the
//                                        triggered/elapsed reminder or,
//                                        set to 0 otherwise
//                          bits 11-14  - not used
//                          bits 8-10   - the reminder type that was triggered/elasped
//                          bits 0-7    - the reminder subtype that was triggered/elapsed
//   words [2] to [5] - the end user parameters defined at reminder creation time

volatile int RQ[max_RQ_free_chain_blocks][free_chain_entry_size];

//  Pointers to manage the free blocks in the RQ
volatile int start_of_free_RQ_chain;
volatile int num_free_RQ_blocks;

//  Pointers to manage the allocated blocks in the RQ
volatile int start_of_RQ_chain =  -1;
volatile int end_of_RQ_chain =    -1;
volatile int num_RQ_reminders =    0;

//  general values to manage RQ processes
#define no_reminder_requests  -1
#define no_entry              -1

//  The following R_xxx variables are set up by scan_RQ so that they can be referenced from the main
//  end user code following a timed reminder alert being taken off the queue.
volatile int  R_status;      // used to indicate if the reminder alert is a final one or otherwise
volatile int  R_type;        // used to indicate what the reminder type was that triggered/elapsed
volatile int  R_subtype;     // used to indicate what the reminder subtype was that triggered/elapsed
volatile int  R_user1;       // end user parameters defined at reminder creation time
volatile int  R_user2;       // ditto
volatile int  R_user3;       // ditto
volatile int  R_user4;       // ditto
