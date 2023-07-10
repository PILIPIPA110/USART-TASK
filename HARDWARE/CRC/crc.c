#include "crc.h"
#include <stdio.h>
#include <stdbool.h>
uint16_t crc16Modbus(const uint8_t* buf, int len) 
{
    uint8_t resL = 0xFF;
    uint8_t resH = 0xFF;
		int i,j;
    for (i = 0; i < len; i++) 
		{
        resL ^= *(buf + i);
        for (j = 0; j < 8; j++) 
				{
            uint8_t tmpH = resH;
            uint8_t tmpL = resL;
            resH >>= 1;
            resL >>= 1;
            if ((tmpH & 0x01) == 0x01) 
						{
                resL |= 0x80;
            }
            if ((tmpL & 0x01) == 0x01) 
						{
                resH ^= 0xa0;
                resL ^= 0x01;
            }
				}
    }
    return (resH << 8) + resL;
}
bool crc16ModbusCheck(const uint8_t* buf, int len) 
{
    uint16_t res = crc16Modbus(buf, len - 2);
    if ((res >> 8) == buf[len - 1] && (res & 0xff) == buf[len - 2])
		{
        return true;
    }

    return false;
}
