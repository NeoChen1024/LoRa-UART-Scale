#include <Arduino.h>
#include "HX711.h"
#include "lib.h"

HX711 loadcell;

#define XCVR_RX	11
#define XCVR_TX	12
#define BAUD_RATE	9600

// Use this if hardware serial present
#define XCVR	Serial2
// Use this if no hardware serial is available
//SoftwareSerial XCVR(XCVR_RX, XCVR_TX);

uint8_t tx_packet[PACKET_SIZE];
uint8_t rx_packet[PACKET_SIZE];

void setup(void)
{
	Serial.begin(115200);
	XCVR.begin(BAUD_RATE);
	pinMode(LED_BUILTIN, OUTPUT);
	loadcell.begin(25, 33);
	loadcell.set_scale(478.66);
	loadcell.tare();

	Serial.println(">> POWER ON <<");
}

void loop(void)
{
	static int ctr = 0;
	char buf[128];
	int pktstate = PKT_NA;
	float weight;
	uint32_t tx_data;
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
					//delay(100); // Wait for LoRa module RX/TX switch
					weight = (float)loadcell.get_units(10);
					memcpy(&tx_data, &weight, sizeof(float));
					construct_packet(tx_packet, ACK, tx_data);
					sprintf(buf, ", ACK value=%lu sent", tx_data);
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
