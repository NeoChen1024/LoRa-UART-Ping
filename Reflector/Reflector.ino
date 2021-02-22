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

uint8_t tx_packet[PACKET_SIZE];
uint8_t rx_packet[PACKET_SIZE];

void setup(void)
{
	Serial.begin(BAUD_RATE);
	XCVR.begin(BAUD_RATE);
	pinMode(LED_BUILTIN, OUTPUT);

	Serial.println("TRANSMITTER ON");
}

void loop(void)
{
	static int ctr = 0;
	char buf[128];
	int pktstate = PKT_NA;
	if(XCVR.available() > 0)
	{
		switch(pktstate = receive_packet(rx_packet, XCVR.read()))
		{
			digitalWrite(LED_BUILTIN, HIGH);
			case PKT_NA:	// Ignore it
				break;
			case PKT_OK:
				sprintf(buf, "op=%x seq=%lu received", get_op(rx_packet), get_serial(rx_packet));
				Serial.print(buf);
				if(get_op(rx_packet) == PING)
				{
					delay(50); // Wait for LoRa module RX/TX switch
					construct_packet(tx_packet, ACK, get_serial(rx_packet));
					sprintf(buf, ", ACK seq=%lu sent", get_serial(rx_packet));
					Serial.println(buf);
					XCVR.write(tx_packet, PACKET_SIZE);
				}
				break;
			case PKT_CORRUPT:
				sprintf(buf, "Corrupted, maybe op=%x seq=%lu", get_op(rx_packet), get_serial(rx_packet));
				Serial.println(buf);
				break;
		}
	}
	digitalWrite(LED_BUILTIN, LOW);
}
