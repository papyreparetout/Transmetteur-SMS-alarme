/*
Recepteur de trame d'une alarme Aritech CD34
The circuit:
 * RX is digital pin 19 (connect to TX of other device)
pour utiliser le Serial1 de l Arduino Mega

 cree par J. ROGUIN 
*/
byte endFrame = 0x00; // indicateur de début et fin de trame
const int ixMaxPanel = 40; // doit etre const pour pouvoir dimensionner un tableau
unsigned long temps_d_acquisition = 30000;
unsigned long tdeb;
boolean fini = false;
boolean finp = false;
int msgbufLen = 0;
char msgbuf[ixMaxPanel]; // longueur doit etre egale à ixMaxPanel
char charIn; // caractere recu
char lastcharIn = 0xFF; // caractere recu precedent
bool mbIsPanelWarning=false;
bool mbIsPanelAlarm=false;

void setup()
{
	Serial.begin(115200);     // Port serie pour liaison Arduino <-> PC
	Serial1.begin(1953);  // communication avec l'alarme 
	tdeb = millis();
	pinMode(13,OUTPUT);
	pinMode(4,OUTPUT);
	Serial.println("fin d'initialisation");
}

void loop() 
{
	if(!fini) 
	{
	  if (Serial1.available()) 
	  { charIn = Serial1.read();

		if(lastcharIn == 0x00) // dernier caractere 0x00 donc fin de trame
		{
			if(charIn == 0x00) // debut de trame on reinitialise 
			{
			Serial.println(" ");
			Serial.println(String(msgbufLen) + " Buffer : ");
			Serial.write(msgbuf,msgbufLen);
			DecodeScreen(msgbuf, msgbufLen );
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
/*

// identification de la console a qui est adresse le message
		byte idDev = (msgbuf[0] & 0xf0)>>4; //ID of remote that message is for 0x0-0xf are valid ids
*/
	
	}
// test sur fin de duree d acquisition
	if((millis()-tdeb) > temps_d_acquisition) { fini = true;
    if (!finp) Serial.println("fin de process");
    finp = true; }
}



void DecodeScreen( char* msgbuf, int msgbufLen ) 
// sous programme de decodage du message envoye par la centrale 
{
	byte bufix=0;
  const int DISP_BUF_LEN=16+1+2;
  byte dispBuffer[DISP_BUF_LEN + 1]="Not Connected";
  const char allmonths[] = {"JANFEVMARAVRMAIJUNJULAOUSEPOCTNOVDEC"};
  const char alldays[] = {"DIMLUNMARMERJEUVENSAM"};
  bool mbIsPanelWarning=false;
  bool mbIsPanelAlarm=false;

	//Checksum so can see if changes and need update client
	static int previousCS =-1;

	int ixMsgbuf=2;//skip 2 header bytes
	msgbufLen-=2; //remove checksum and zero terminator

	while(ixMsgbuf<msgbufLen)
	{
		uint8_t rx = msgbuf[ixMsgbuf++];
		if (rx>=0 && rx < 0x0f)
		{//not implemented
		}
		else if (rx == 0x13)
		{//not implemented
		}
		else if (rx == 0x1b)
		{//to do with foreign character set - not implemented
		}
		else if (rx>= 0x20 && rx <= 0x7F)
		{//Normal ASCII
			if (bufix==0)
				//Force Screen clear at start of each message
				for(int m=0;m<DISP_BUF_LEN;m++)
					dispBuffer[m]=' ';

			if (bufix < DISP_BUF_LEN)
				dispBuffer[bufix++]=(char)rx;
		}
		else if (rx>= 0x80 && rx <= 0x8F)
		{//Date in encoded format
			int b0=rx;
			int b1=msgbuf[ixMsgbuf++];
			int b2=msgbuf[ixMsgbuf++];
			int b3=msgbuf[ixMsgbuf++];

			byte nMonth= (b0 & 0x0f)-1;
			byte jour = (b1 & (128+64+32))>> 5;
			byte date = (b1 & (31));
			byte h1=(b2 & 0xf0)>>4; if(h1==0x0A) h1=0;
			byte h2=(b2 & 0x0f); if(h2==0x0A) h2=0;
			byte m1=(b3 & 0xf0)>>4; if(m1==0x0A) m1=0;
			byte m2=(b3 & 0x0f); if(m2==0x0A) m2=0;

			memcpy(dispBuffer+0,alldays+(jour*3),3);
			dispBuffer[3]=' ';
			dispBuffer[4]=('0'+(int)(date/10));
			dispBuffer[5]=('0'+(date%10));
			dispBuffer[6]=' ';

			memcpy(dispBuffer+7,allmonths+(nMonth*3),3);
			dispBuffer[10]=' ';
			dispBuffer[11]='0'+h1;
			dispBuffer[12]='0'+h2;
			//if (dateFlash)
			dispBuffer[13]= ':';
			//else
			//	buffer[13]= F(' ');
			//dateFlash=!dateFlash;
			//buffer[13]= ((millis()/500)&1) ==0? ':':' ';
			dispBuffer[14]='0'+m1;
			dispBuffer[15]='0'+m2;
			bufix=0;
		}
		else if (rx == 0x90)
		{//CLS
			bufix=0;
			for(int m=0;m<DISP_BUF_LEN;m++)
				dispBuffer[m]=' ';
		}
		else if (rx == 0x91)
		{//HOME
			bufix=0;
		}
		else if (rx >= 0xA0 && rx <= 0xAf)
		{//MOVE cursor to position x
			bufix = (rx & 0x0f); //-1 gives us 2 *'s  but without -1 we go off screen at Login ***
		}
		else if (rx >= 0xB0 && rx <= 0xBF)
		{//{BLINK_N}"	Bxh Blink x chars starting at current cursor position
		 //not implementing this as it will cause unnecessary traffic sending display each second
			//int nChars = (rx & 0x0f)-1;
			//if (dateFlash)
			//	buffer[i]= ':';
			//else
			//	buffer[i]= ' ';
			//dateFlash=!dateFlash;
		}
		else if (rx >= 0xC0 && rx <= 0xCf)
		{// Set position to x and clear all chars to right
			int i = (rx & 0x0f);
			if (i < DISP_BUF_LEN)
				bufix = i;
			for(int n=bufix;n<DISP_BUF_LEN;n++)
				dispBuffer[bufix++]=' ';
		}
		else if (rx>= 0xE0 && rx <= 0xFF)
		{// Special Characters Arrows and foreign chars
			int i = (rx & 0x0f);

			char c=0;
			if (i==4)	c= '*';
			else if (i==5)	c= '#';
			else if (i==7)	c= '>';

			if (c>0)
				if (bufix < DISP_BUF_LEN)
					dispBuffer[bufix++]=c;
		}
		else
		{//unknown command
			Serial.println("{"+String(rx)+"}");
		}

		//Note: there are quite a few codes in Engineer menu to deal with flashing cursors and characters - cannot do easily in html
	}

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

		//Send Email if alarm lights come on
			bool bIsPanelWarning = (msgbuf[1] & 0x04) != 0;
			if (bIsPanelWarning == true && mbIsPanelWarning == false)
//				SMTP::QueueEmail(WARNING);
			mbIsPanelWarning = bIsPanelWarning;

			bool bIsPanelAlarm = (msgbuf[1] & 0x02) != 0;
			if (bIsPanelAlarm == true && mbIsPanelAlarm == false)
//				SMTP::QueueEmail(ALARM);
			mbIsPanelAlarm = bIsPanelAlarm;
		
	dispBuffer[16]='|';	//this may overwrite a char sometimes...ok.
	//  x = (expression)? valeur_x_si_expression_vraie : valeur_x_si_expression_fausse
	dispBuffer[17]=(mbIsPanelAlarm)?'A':' ';
	dispBuffer[18]=(mbIsPanelWarning)?'W':' ';

	int csb=0;
	for(int n=0;n<DISP_BUF_LEN;n++)
		csb+=(char)dispBuffer[n];

	if (csb != previousCS)
	{
		previousCS=csb;
		//Signal new screen to send but dont send immediately- SendDisplayToBrowser can take time
//		bScreenHasUpdated = true;
	}

  Serial.write(dispBuffer,20);

}
