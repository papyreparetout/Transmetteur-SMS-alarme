
#include <Arduino.h>
#include "utilalarm.h"
#include "Adafruit_FONA.h"

int DecodeScreen( char* msgbuf, int msgbufLen ) 
// sous programme de decodage du message envoye par la centrale 
{
  int retour = 0;

	msgbufLen-=2; //remove checksum and zero terminator

// controle de checksum 
		char cs = 0;
		for(int n=0;n<(msgbufLen+1);n++)
		{
			char rx = msgbuf[n];
		//	if (n<(msgbufLen+1)) //dont sum cs or terminator
				cs+=rx;
		}

		if (cs == 0)
			cs++; //protocol avoids 0 except for end marker- so will send cs 00 as 01
		if (cs != msgbuf[msgbufLen+1])
		{
			Serial.println("CS Fail calcul:"+ String(cs) + " lu " + String(msgbuf[msgbufLen+1]));
			return;
		}

// Detection LED défaut et LED Alarme
	bool bIsPanelWarning = (msgbuf[1] & 0x04) != 0;
	bool bIsPanelAlarm = (msgbuf[1] & 0x02) != 0;
 if (bIsPanelWarning == true)
   {retour = 1;}
  else if (bIsPanelAlarm == true )
    {retour = 2;}
  else retour = 0;

	return retour;
}
/*
uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout) {
  uint16_t buffidx = 0;
  boolean timeoutvalid = true;
  if (timeout == 0) timeoutvalid = false;

  while (true) {
    if (buffidx > maxbuff) {
      //Serial.println(F("SPACE"));
      break;
    }

    while (Serial.available()) {
      char c =  Serial.read();

      //Serial.print(c, HEX); Serial.print("#"); Serial.println(c);

      if (c == '\r') continue;
      if (c == 0xA) {
        if (buffidx == 0)   // the first 0x0A is ignored
          continue;

        timeout = 0;         // the second 0x0A is the end of the line
        timeoutvalid = true;
        break;
      }
      buff[buffidx] = c;
      buffidx++;
    }

    if (timeoutvalid && timeout == 0) {
      //Serial.println(F("TIMEOUT"));
      break;
    }
    delay(1);
  }
  buff[buffidx] = 0;  // null term
  return buffidx;
}
*/
void flushSerial() {
  while (Serial.available())
    Serial.read();
}
/*
void SendSMS(int recu, char* numtel)
{
	String messms[3] = {" Fin défaut ou alarme"," Défaut alarme", " Alarme intrusion"};
	char message[30];
	
	switch (recu) {
	case 1:
	Serial.println("recu "+ String(recu)+ messms[1]);
	break;
	case 2:
	Serial.println("recu "+ String(recu)+messms[2]);
	break;
	case 0:
	Serial.println("recu "+ String(recu)+messms[0]);
	}
	messms[recu].toCharArray(message,30);
	if (!fona.sendSMS(sendto, message)) {
    Serial.println(F("Failed"));
        }
	else {
          Serial.println(F("Sent!"));
        }
	
	}
	
}
*/
