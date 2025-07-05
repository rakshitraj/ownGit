#pragma once

#include <zlib.h>

#define CHUNK_SIZE 1024
// #define DEFLATE_LEVEL Z_BEST_SPEED
// #define DEFLATE_FLUSH Z_SYNC_FLUSH

int hash_object(const int argc, const char* const argv[]);
