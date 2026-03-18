#include "zrd.h"
#include "string.h"
#include "kprintf.h"

static zrd_header_t *zrd_header = NULL; // pointer to header
static zrd_entry_t *zrd_entries = NULL; // pointer to file entries

void zrd_init(void *addr) {
    // 1. validate magic && null check
    if (!addr) {
        kprintf("[ZRD] init: No address provided\n");
        return;
    }
    zrd_header = (zrd_header_t *)addr;
    if (strncmp(zrd_header->magic, "ZRD", 4) != 0) {
        kprintf("[ZRD] init: Invalid magic at %p (got %.4s)\n", addr, zrd_header->magic);
        zrd_header = NULL;
        return;
    }
    // 2. cast pointer to file entries and read into in-memory data structure
    zrd_entries = (zrd_entry_t *)((uint8_t *)addr + sizeof(zrd_header_t));
    kprintf("[ZRD] init: Found ramdisk at %p with %d files\n", addr, zrd_header->count);
    zrd_stat();
}

void *zrd_open(const char *name) {
    if (!zrd_header || !zrd_entries || !name) {
        return NULL;
    }
    for (uint32_t i = 0; i < zrd_header->count; i++) {
        if (strncmp(zrd_entries[i].name, name, 32) == 0) {
            return (void *)((uint8_t *)zrd_header + zrd_entries[i].offset);
        }
    }
    return NULL; // not found
}
uint32_t zrd_size(const char *name) {
    if (!zrd_header || !zrd_entries || !name) {
        return 0;
    }
    for (uint32_t i = 0; i < zrd_header->count; i++) {
        if (strncmp(zrd_entries[i].name, name, 32) == 0) {
            return zrd_entries[i].size;
        }
    }
    return 0; // not found
}

void zrd_stat(void) {
    if (!zrd_header) {
        return;
    }
    kprintf("ZRD files: %d\n", zrd_header->count);
    for (uint32_t i = 0; i < zrd_header->count; i++) {
        kprintf("  %s (%u bytes)\n", zrd_entries[i].name, zrd_entries[i].size);
    }
}