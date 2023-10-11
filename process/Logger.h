/****************************************************************************************
  // The logger.cpp is the real client that communicate with the server:
  //    client process open it and use it to send log information.
  //    server can also send information to it to change the monitor level.
  // The user process will first need to initialized it with InitializedLog() function 
  //    so Logger can run and record the log information.
  // SetLogLevel() give the ability to controy which level of Log information need to be recording.
  // Log() is will initialized a message.
  // ExitLog() close the log.

**************************************************************************************/

#include <stdlib.h>
#include <string.h>

#ifndef LOGGER_H
#define LOGGER_H

typedef enum {
    DEBUG,
    WARNING,
    ERROR,
    CRITICAL
} LOG_LEVEL;


//Address of the server
const int PORT=1153;
const char IP_ADDR[]="192.168.1.31";

int InitializeLog();
void SetLogLevel(LOG_LEVEL level);
void Log(LOG_LEVEL level, const char* prog, const char* func, int line, const char* message);
void ExitLog();
#endif