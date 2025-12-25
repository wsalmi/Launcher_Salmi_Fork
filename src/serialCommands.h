#ifndef SERIALCOMMANDS_H
#define SERIALCOMMANDS_H

#include <Arduino.h>

void processSerialCommand();
void cmd_help();
void cmd_wifi(String args);
void cmd_webui();
void cmd_status();

#endif // SERIALCOMMANDS_H
