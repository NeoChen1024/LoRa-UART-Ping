#ifndef __COMMON_LIBS__

#define PACKET_SIZE	8

#define START	0xAA
#define PING	0x80
#define ACK	0xC0

typedef uint16_t crc_t;

crc_t crc_update(crc_t crc, const void *data, size_t data_len)
{
	const unsigned char *d = (const unsigned char *)data;
	unsigned int i;
	int bit;
	unsigned char c;

	while (data_len--) {
		c = *d++;
		for (i = 0; i < 8; i++) {
			bit = !!(crc & 0x8000);
			crc = (crc << 1) | ((c >> (7 - i)) & 0x01);
			if (bit) {
				crc ^= 0x1021;
			}
		}
		crc &= 0xffff;
	}
	return crc & 0xffff;
}

crc_t crc_finalize(crc_t crc)
{
	unsigned int i;
	int bit;

	for (i = 0; i < 16; i++) {
		bit = !!(crc & 0x8000);
		crc <<= 1;
		if (bit) {
			crc ^= 0x1021;
		}
	}
	return crc & 0xffff;
}

void construct_packet(uint8_t *packet, uint8_t opcode, uint32_t serial)
{
	int i = 0;
	crc_t crc16 = 0;
	packet[0] = START;
	packet[1] = opcode;
	memcpy(packet + 2, &serial, 4);

	// Calculate checksum
	crc16 = crc_update(crc16, packet, 6);
	crc16 = crc_finalize(crc16);
	memcpy(packet + 6, &crc16, 2);
}

enum pktstate
{
	PKT_NA,	// No valid START yet
	PKT_OK,	// Packet valid, CRC ok
	PKT_CORRUPT	// Have valid START, CRC invalid
};

// Rolling-Window Decoding
enum pktstate receive_packet(uint8_t *packet, uint8_t data)
{
	int i;
	crc_t crc16 = 0;
	crc_t pktcrc;

	// Shift backwards
	for(i = 1; i < PACKET_SIZE; i++)
	{
		packet[i - 1] = packet[i];
	}
	packet[PACKET_SIZE - 1] = data;

	// Validate Packet
	
	if(packet[0] != 0xAA)
		return PKT_NA;

	crc16 = crc_update(crc16, packet, PACKET_SIZE - 2);
	crc16 = crc_finalize(crc16);

	memcpy(&pktcrc, packet + 6, 2); // Copy CRC value from packet

	if(pktcrc == crc16)
		return PKT_OK;
	else
		return PKT_CORRUPT;
}

uint32_t get_serial(uint8_t *packet)
{
	uint32_t a;
	memcpy(&a, packet + 2, sizeof(uint32_t));

	return a;
}

uint8_t get_op(uint8_t *packet)
{
	return packet[1];
}

#  define __COMMON_LIBS__
#endif // __COMMON_LIBS__
