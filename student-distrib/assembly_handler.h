#ifndef _ASSEMBLYHANDLER_H
#define _ASSEMBLYHANDLER_H
#include "keyboard.h"
#include "rtc.h"
#include "systemCall.h"
#include "scheduling.h"

//two linked function handlers to be set into the IDT
//keyboard handler set into assembly
extern void keyboard_translated();
//rtc handler set into assembly
extern void rtc_translated();
//pit handler set into assembly
extern void pit_translated();
#endif /* _ASSEMBLYHANDLER_H */
