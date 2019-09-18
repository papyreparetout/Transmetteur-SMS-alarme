/*
Recepteur de trame d'une alarme Aritech CD34 et decodage pour envoi SMS
lorsque l alarme a detecte des situations pre definies
Le circuit:
 * reception des messages emis par l alarme sur  pin 19 (1953 bauds)
pour utiliser le Serial1 de l Arduino Mega
 * lien avec carte SIM800 via software serial sur pin 10 et 11
 cree par J. ROGUIN 
*/
#include "utilalarm.h"
#include "Adafruit_FONA.h"
#include <SoftwareSerial.h>
// variables pour l envoi de SMS
#define FONA_RST 4
SoftwareSerial fonaSS = SoftwareSerial(10, 11);
SoftwareSerial *fonaSerial = &fonaSS;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);
uint8_t type;
unsigned long milliprec;  // pour test regulier

// numero de tel destinataire du SMS
char sendto[21] = "+33608731412";  // jean
char sendto1[21] = "+33674077232";  // regine 
char sendto2[21] = "+33608161222";  // flo

char PIN[5] = "1234";  // code PIN de la carte telephone du module SIM8000
char message[30];

// variables pour le dialogue avec l alarme
byte endFrame = 0x00; // indicateur de début et fin de trame
const int ixMaxPanel = 40; // doit etre const pour pouvoir dimensionner un tableau
int msgbufLen = 0;
char msgbuf[ixMaxPanel]; // longueur doit etre egale à ixMaxPanel
char charIn; // caractere recu
char lastcharIn = 0xFF; // caractere recu precedent
bool mbIsPanelWarning=false;
bool mbIsPanelAlarm=false;
// pour test
bool verbose = true;  // pour suivi du process

void setup()
{
	pinMode(13,OUTPUT);
	pinMode(4,OUTPUT);
	digitalWrite(4,LOW);
	//while (!Serial);
	Serial.begin(115200);
	if (verbose) { Serial.println(F("FONA basic test"));
		Serial.println(F("Initializing....(May take 3 seconds)"));
		}

	fonaSerial->begin(4800);
	if (! fona.begin(*fonaSerial)) {
		Serial.println(F("Couldn't find FONA"));
		digitalWrite(4,HIGH);
		while (1);
		}
	if (verbose) { Serial.println(F("FONA is OK"));
		Serial.print(F("Found "));
		}

	Serial1.begin(1953);  // communication avec l'alarme 

	// Print module IMEI number.
	char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
	uint8_t imeiLen = fona.getIMEI(imei);
	if (imeiLen > 0) {
		Serial.print("Module IMEI: "); Serial.println(imei);
		}
	
	if (verbose) { Serial.println(" code pin carte ");
		Serial.print(PIN);
		Serial.println(F("Unlocking SIM card: "));
		}
	if (! fona.unlockSIM(PIN)) {
		Serial.println(F("Failed"));
		digitalWrite(4,HIGH);
	} 
	else 
	{
		Serial.println(F("OK!"));
	}
	delay(5000);
	milliprec= millis();
	Serial.println("fin d'initialisation");
}

void loop() 
{	
	int retour = 0;
	static int retourprec = 0;
	String messtest = "Test envoi SMS";
// test regulier par envoi d un SMS tous les xx millisecondes
  unsigned long period = 86400000; // 1 jour en ms
	  if ( (millis()-milliprec)> period) {
           		if (! fona.unlockSIM(PIN)) {
				  Serial.println(F("Failed"));
				  digitalWrite(4,HIGH);
				} 
				else 
				{
				  Serial.println(F("OK!"));
				}
		  		messtest.toCharArray(message,30);
				if (!fona.sendSMS(sendto, message)) {
				  Serial.println(F("Failed"));
					  }
				else {
						Serial.println(F("Sent!"));
					  }
            milliprec = millis();
          
	  		}
	  if (Serial1.available()) 
	  { charIn = Serial1.read();
		if(lastcharIn == 0x00) // dernier caractere 0x00 donc fin de trame
		{
			if(charIn == 0x00) // debut de trame on reinitialise 
			{
			retour = DecodeScreen(msgbuf, msgbufLen );  // decodage du message envoye par l alarme
			if (retour != retourprec) {
				String messms[3] = {" Fin defaut ou alarme"," Defaut alarme", " Alarme intrusion"};
				switch (retour) {
				case 1:
				Serial.println("recu "+ String(retour)+ messms[1]);
				break;
				case 2:
				Serial.println("recu "+ String(retour)+messms[2]);
				break;
				case 0:
				Serial.println("recu "+ String(retour)+messms[0]);
				}
				messms[retour].toCharArray(message,30);
				if (verbose) { Serial.println(" code pin carte ");
					Serial.print(PIN);
					Serial.println(F("Unlocking SIM card: "));
					}
				if (! fona.unlockSIM(PIN)) {
				  Serial.println(F("Failed"));
				  digitalWrite(4,HIGH);
				} 
				else 
				{
				  Serial.println(F("OK!"));
				}
				if (!fona.sendSMS(sendto, message)) {
				  Serial.println(F("Failed"));
					  }
				else {
						Serial.println(F("Sent!"));
					  }
				}
			retourprec = retour;
			digitalWrite(13,HIGH);
			msgbufLen=0;
			for(int n=0;n<ixMaxPanel;n++) msgbuf[n]=0xFF; // remise à blanc du buffer 
			}
		lastcharIn = charIn;
		msgbuf[0] = charIn;
		}
		else
		{// on remplit le buffer de caracteres a partir du deuxieme
		digitalWrite(13,LOW);
		lastcharIn = charIn;
		msgbufLen++;
		msgbuf[msgbufLen] = charIn;
		if (charIn != 0)
			{//wasn't end of packet - is buffer full?
		if (msgbufLen>=ixMaxPanel)
				{//packet never terminated - bytes lost :(
				Serial.println("Buffer overflow");
				}
			}
		}
	 }

// fin loop
}
