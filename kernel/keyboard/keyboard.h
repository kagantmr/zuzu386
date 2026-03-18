#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

void keyboard_init();
int keyboard_getchar();
int keyboard_haschar();

#endif