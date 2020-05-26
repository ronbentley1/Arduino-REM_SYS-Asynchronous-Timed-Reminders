Ron D Bentley (UK)

# REM_SYS – asynchronous elapsed & real-time reminder alerting

Have you a need to develop Arduino applications that are based on time? For example, performing certain operations and tests at specific times of the day, repeating other actions at fixed intervals as a one off activity, a recurring activity or just for a fixed duration? If so, then perhaps REM_SYS is well worth considering as the basis within which to develop your applications

With the REM_SYS framework you are able to plug in your code and programmatically define and create reminders that will alert a) at specific times of the day, b) specific days of the week/month, or c) at fixed intervals. Furthermore, when defining reminders, end users are able to add their own parameters for return at the time of alerting for subsequent decision support and program flow.

REM_SYS supports two ‘flavours’ of reminder – elapsed time & real-time:

1. Elapsed time reminders (ETRs) operate without any awareness of real world real-time, rather like a stop watch – switched on and stopped at some point after a given time has elapsed.
2. Real-time reminders (RTRs) operate with full awareness of real world real-time, linked to a hardware Real Time Clock (RTC). For example, if you want an alert at 21:45 hours each day, then RTRs can support this.

ETRs and RTRs can each be specified as one of three different types:
1. One off alert - only one alert will be generated at the given time, or
2. Recurring alert - alerts will be generated indefinitely after the start time of the first alert, or
3. Recurring with duration alert - alerts will be generated as for the recurring type, but will cease after the specified duration period.

REM_SYS is able to support multiple and concurrent reminders of either type and is fully configurable to meet a wide range of needs.
In addition to functions to create each type of reminder, the framework also provides functionality to manage reminders, service reminder alerts and manipulate dates.

The REM_SYS User Guide provides a comprehensive appreciation of the capabilities of the framework.
Have a look!



 
