#ifndef DM_CRC_H
#define DM_CRC_H

#include <stdint.h>
#include <stdbool.h>
bool crc16ModbusCheck(const uint8_t* buf, int len);
uint16_t crc16Modbus(const uint8_t* buf, int len);

#endif
