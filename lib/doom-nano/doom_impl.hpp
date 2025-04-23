#pragma once 

#include <types.h>

void updateHud();
Coords translateIntoView(Coords *pos);
uint8_t getBlockAt(const uint8_t level[], uint8_t x, uint8_t y);
