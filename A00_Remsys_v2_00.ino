/*   version 1.00 rev 1, Ron D Bentley (Stafford UK) March 2020.
 *   version 1.00 rev 2, July 2020 - moved print strings to program storage space to reduce
 *                                   amount of dynamic storage used. Plus improved efficiency in 
 *                                   heart beat coding.
 *   version 2.0,        Aug 2020  - this version allows REM_SYS to be configured without the real-time clock                                
 *                                   module, so that ETR only applications can be designed.
This framework provides a basis for the development of applications that would benefit from timed 
reminder alerting.  Timed reminder alerts can be defined/created as elapsed time reminders (ETRs)or
as real-time reminders (RTRs) linked to a real-time clock (RTC).  Any number of reminders can be 
active at any one time, dependent on the size of the Reimder List initialised.
____________________________________________________________________________________________________
Copyright (c) Ron D Bentley (UK)
The extent this licence shall be limited to Non-profit Use.
Permission is hereby granted, free of charge, for non-profit purposes to any person obtaining
a copy of this software and associated documentation files (the "Software"), to deal in the 
Software without restriction, including without limitation the rights to use, copy, modify, merge, 
publish, distribute, sublicense copies of the Software, and to permit persons to whom the Software 
is furnished to do so, subject to the following conditions:

The above copyright notice, acknowledgements and this permission notice shall be included in all 
copies or substantial portions of the Software. 
 
THE SOFTWARE IS LIMITED TO NON-PROFIT USE AND IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
____________________________________________________________________________________________________

See User Guide for full capabilities and instructions for implementation.

 */ 
