#ifndef utilalarm_h
#define utilalarm_h
#include <Arduino.h>
#include "Adafruit_FONA.h"

// sous programme de decodage du message envoye par la centrale
int DecodeScreen( char* msgbuf, int msgbufLen ); 

// uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout);

void flushSerial();

// sous programme d'envoi du sms
//void SendSMS(int recu, char* numtel);

#endif
