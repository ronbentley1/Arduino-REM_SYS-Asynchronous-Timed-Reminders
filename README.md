Ron D Bentley (UK)
# Arduino-Asnchronous REM_SYS-Timed-Reminders
A framework supporting the creation and processing of elapsed and real-time reminder alerting asynchronously

From the REM_SY User Guide:

Introduction
This guide provides a quick and ready reference to understand and implement the REM_SYS framework for designing and building Arduino microcontroller applications based on the concept of asynchronous elapsed and real-time reminder alerting.  

That is, if you have a need for date/time based processing, either as elapsed time or real-world real-time, then REM_SYS is well worth a look.  REM_SYS can provide a different approach and thinking about application design and is well suited to multi-tasking solutions. 
The framework has been derived from an earlier version  that dealt exclusively with elapsed time reminders (ETRs).  In this version, real-time reminder functionality has been added to the earlier ETR framework, providing a comprehensive suite of capabilities for supporting applications that require asynchronous alerting over periods of time (elapsed and real-time).

The REM_SYS framework is built around the concept of defining reminder entities (which can be either of type elapsed or real-time) which are processed and handled asynchronously, leaving main line code to do ..... whatever it needs to do.  

Reminder alerts (ETRs/RTRs) can be considered to be analogous to interrupts, albeit they are produced at defined points in time.   Reminder alerts will be processed as a part of the main code loop and can provide comprehensive features that offer the designer a good deal of freedom in decision support and control.  The beauty of the framework is that it supports multiple ET and RT reminders concurrently, up to the number defined as part of framework configuration.

In producing this guide, an attempt to balance brevity and the degree of technical detail required to successfully implement the REM_SYS framework has been sought. Hopefully, the reader will find this balance struck?


 
