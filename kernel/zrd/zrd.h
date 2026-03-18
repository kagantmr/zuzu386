#ifndef ZRD_H
#define ZRD_H

#include "stdint.h"

/**
 * Zuzu ramdisk is a simple filesystem with the following layout:
 * [header]
 * magic:   4 bytes  "ZRD\0"
 * count:   4 bytes  number of files
 * [file entry] × count
 * name:    32 bytes  null-terminated filename
 * offset:  4 bytes   offset from start of ramdisk
 * size:    4 bytes   file size in bytes
 * [file data]
 * raw bytes of each file, packed sequentially
 */

 typedef struct {
    char     magic[4];   // "ZRD\0"
    uint32_t count;      // number of files
} __attribute__((packed)) zrd_header_t;

typedef struct {
    char     name[32];   // null-terminated filename
    uint32_t offset;     // offset from start of ramdisk
    uint32_t size;       // file size in bytes
} __attribute__((packed)) zrd_entry_t;

void  zrd_init(void *addr);           // init with address where ramdisk was loaded
void *zrd_open(const char *name);     // returns pointer to file data
uint32_t zrd_size(const char *name);  // returns file size
void  zrd_stat(void);                 // print ramdisk file listing


#endif /* ZRD_H */