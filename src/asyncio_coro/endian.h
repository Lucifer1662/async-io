#pragma once
#include <stdint.h>

bool is_littleEndian();
uint64_t to_big_endian64(uint64_t a);
uint64_t from_big_endian64(uint64_t a);
uint64_t big_to_machine64(uint64_t a);
uint64_t host_to_network64(uint64_t a);
uint64_t network_to_host64(uint64_t a);
