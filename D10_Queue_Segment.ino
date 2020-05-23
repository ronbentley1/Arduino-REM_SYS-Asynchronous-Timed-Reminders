// Copyright (c) Ron D Bentley (UK), see copyright notice
//
//  This tab contains the routines to establish and maintain reminder
//  queue management.
//
//  A free chain of blocks is set up that are then allocated to reminder alert requests
//  as they decome due.
//
//  'RQ' is used as the blocks of memory to be allocated/deallocated on request.
//  'RQ' comprises a number of free blocks each of 5 integers, the first word of each block
//  contains a forward pointer to next block or end of chain value(free/used).

//  When a free block is allocated to a reminder request, it is inserted at the beginning of the
//  reminder queue with the first word of the block being a forward pointer to next allocated
//  block and the remainder of block containing user values defined as part of the remider set up.
//
//  Note:
//      1.  The number of free blocks in the RQ free chain is defined by 'max_RQ_free_chain_blocks'.
//      2.  The implementation is as a queue, first in first out (FIFO).
//

//  Set up the free RQ chain as defined by 'max_RQ_free__chain_blocks'
//  This is called just once per start/reset from te setup() process.
//
void  create_RQ_free_chain() {
  int  ptr, last_RQ_block;
  last_RQ_block = max_RQ_free_chain_blocks - 1;
  for (ptr = 0; ptr < max_RQ_free_chain_blocks; ptr++) {
    if (ptr == last_RQ_block) {
      RQ[ptr][0] = end_of_chain_value; // set end of chain
    }
    else                     {
      RQ[ptr][0] = ptr + 1; // set forward block pointer
    }
  }
  num_free_RQ_blocks = max_RQ_free_chain_blocks;
  start_of_free_RQ_chain = 0;
}
//
//  Allocates a block from the free chain, if one exists.
//  NOTE: assumes that interrupts are disabled by calling function(s).
//
int acquire_block_from_free_RQ() {
  int  block;
  if (num_free_RQ_blocks > 0) {
    //  There is at least 1 free block left.
    //  Take the next block off the free chain.
    num_free_RQ_blocks--;  //  reduce free blocks available.
    block = start_of_free_RQ_chain;
    start_of_free_RQ_chain = RQ[block][0];  //  take the first free block off the free chain.
    RQ[block][0] = end_of_chain_value;      //  set the forward pointer in this free block to out of range.
    return block;         //  return with the 'address' of the free block provided to calling routine.
  }
  return  fail;           //  no free blocks! Return error condition.
}

//
// Returns the given block back to the free chain.
// NOTE: assumes that reminders are disabled by calling function(s).
//
int relinquish_block_to_free_RQ(int block) {
  if (num_free_RQ_blocks < max_RQ_free_chain_blocks)  {
    //  there is space to add this block back to the free chain.
    num_free_RQ_blocks++;  //  increase number of free blocks on free chain by 1
    RQ[block][0] = start_of_free_RQ_chain;
    start_of_free_RQ_chain = block;
    return  success;      //  relinquish was successful.
  }
  return  fail;           //  free chain seems to be full of free blocks! No space to add another.
}

//
//  The function creates an entry in the interrupt queue for the given interrupt.
//
int insert_into_RQ(int R_status, int R_user1, int R_user2, int R_user3, int R_user4) {
  int  block;
  block = acquire_block_from_free_RQ();
  if (block == fail) {
    //  no free block available!
    return fail;
  }
  //  we have a free block, so chain it into the reminder queue
  //  placing it at the end of the current queue.
  if (num_RQ_reminders == 0) {
    // queue is empty, so set start pointer
    start_of_RQ_chain = block;
  }  else  {
    // at least on entry in RQ queue, so modify forward pointer of last block
    RQ[end_of_RQ_chain][0] = block;
  }
  //  now deal with rest of updates
  num_RQ_reminders++;
  end_of_RQ_chain = block;
  RQ[block][0] = end_of_chain_value;  // set new end of chain
  RQ[block][1] = R_status;
  RQ[block][2] = R_user1;
  RQ[block][3] = R_user2;
  RQ[block][4] = R_user3;
  RQ[block][5] = R_user4;
  return success;
}

//
//  See if there are any outstanding (unprocessed) reminder requests in the
//  reminder queue.  If so, take the first one on the queue and return it to the
//  free chain.  The answer to the function is either reminder found
//  or that no outstanding reminder requests exists.
//
//  Note that this function is not entered via any interrupt routine, so we do need to
//
int scan_RQ() {
  int RQ_block, interrupt;
  //noInterrupts();   //  ensure exclusive access of the RQ and its pointers
  if (num_RQ_reminders == 0)  {
    //  Clear down the end user reminder parameters, if no RQ entry to be processed.
    R_status  = 0;
    R_type    = 0;
    R_subtype = 0;
    R_user1   = 0;
    R_user2   = 0;
    R_user3   = 0;
    R_user4   = 0;
    //interrupts();
    return no_reminder_requests;
  }
  // take the first entry off the RQ as this is the next interrupt to be processed.
  noInterrupts();
  num_RQ_reminders--;
  RQ_block = start_of_RQ_chain;
  start_of_RQ_chain = RQ[RQ_block][0];    //  point to next interrupt to be processed, or end of chain
  if (num_RQ_reminders == 0)  {
    // last used block to be relinquished, so adjust end of chain pointer
    end_of_RQ_chain = end_of_chain_value;
  }
  //  set up the interrupt parameters for this interrupt,
  //  solely for end user reference if required
  R_status  = RQ[RQ_block][1];          // active/inactive, R_type and R_subtype
  R_type    = (R_status & 0x7ff) >> 8;  // the type of reminder
  R_subtype = R_status & 0xff;          // the reminder subtype value
  R_status  = bitRead(R_status, 15);    // indicates if last reminder alert recieved, or not
  R_user1   = RQ[RQ_block][2];
  R_user2   = RQ[RQ_block][3];
  R_user3   = RQ[RQ_block][4];
  R_user4   = RQ[RQ_block][5];
  relinquish_block_to_free_RQ(RQ_block);  //  put this block back on the free RQ chain
  interrupts();
  return success; // return with the internal value of the interrupt handler that was assigned to this pin
}
//
// Utility to print the reminder queue (RQ)
//
void print_RQ() {
  int ptr, count, i;
  if (diags_on) {
    noInterrupts();
    count = num_RQ_reminders;
    ptr = start_of_RQ_chain;
    Serial.println("_______________________________");
    Serial.println("       REMINDER QUEUE");
    Serial.print("          Num in RQ = ");
    Serial.println(count);
    Serial.print(" start of RQ chain = ");
    Serial.println(ptr);
    Serial.print("   end of RQ chain = ");
    Serial.println(end_of_RQ_chain);
    if (count > 0) {
      do {
        for (i = 0; i < free_chain_entry_size; i++) {
          Serial.print("RQ[");
          Serial.print(ptr);
          Serial.print("]");
          Serial.print(i);
          Serial.print("] = ");
          Serial.println(RQ[ptr][i]);
        }
        Serial.print("\n");
        ptr = RQ[ptr][0]; //  look at next entry/block
        count --;
      }
      while ((ptr != end_of_chain_value) && (count > 0));
    }
    Serial.println("");
    Serial.println("_______________________________\n");
    Serial.flush();
    interrupts();
  }
}
//
// Utility to print the free chain
//
void print_free_chain() {
  int ptr, count;
  if (diags_on) {
    noInterrupts();
    count = num_free_RQ_blocks;
    ptr = start_of_free_RQ_chain;
    Serial.println("");
    Serial.println("_______________________________");
    Serial.println("         FREE CHAIN");
    Serial.print("  Num in free chain = ");
    Serial.println(count);
    Serial.print("start of free chain = ");
    Serial.println(ptr);
    if (count > 0) {
      do {
        Serial.print("RQ[");
        Serial.print(ptr);
        Serial.print("][0] = ");
        Serial.println(RQ[ptr][0]);
        ptr = RQ[ptr][0]; //  look at next entry/block
        count --;
      }
      while ((ptr != end_of_chain_value) && (count > 0));
    }
    Serial.println("_______________________________\n");
    Serial.flush();
    interrupts();
  }
}
