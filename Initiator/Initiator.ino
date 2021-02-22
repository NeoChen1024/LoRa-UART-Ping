#include <Arduino.h>
#include <SoftwareSerial.h>

#include "lib.h"

#define XCVR_RX	11
#define XCVR_TX	12
#define BAUD_RATE	9600

// Use this if hardware serial present
//#define XCVR	Serial1
// Use this if no hardware serial is available
SoftwareSerial XCVR(XCVR_RX, XCVR_TX);

uint32_t count = 0; // frame count
uint8_t tx_packet[PACKET_SIZE];
uint8_t rx_packet[PACKET_SIZE];

void setup(void)
{
	Serial.begin(BAUD_RATE);
	pinMode(LED_BUILTIN, OUTPUT);

	Serial.println("TRANSMITTER ON");
}

void loop(void)
{
	static int ctr = 0;
	char buf[128];
	int pktstate = PKT_NA;

	digitalWrite(LED_BUILTIN, HIGH);
	construct_packet(tx_packet, PING, count++);
	sprintf(buf, "seq=%lu sent", count);
	Serial.print(buf);
	XCVR.write(tx_packet, PACKET_SIZE);
	delay(500);
	digitalWrite(LED_BUILTIN, LOW);

	for(ctr = 0; ctr < 50; ctr++)
	{
		if(XCVR.available() > 0)
		{
			switch(pktstate = receive_packet(rx_packet, XCVR.read()))
			{
				case PKT_NA:	// Ignore it
					break;
				case PKT_OK:
					sprintf(buf, ", op=%x seq=%lu received", get_op(rx_packet), get_serial(rx_packet));
					Serial.print(buf);
					break;
				case PKT_CORRUPT:
					Serial.print(" -!");
					break;
			}
		}
		delay(10);
	}
	Serial.print("\r\n");
}
