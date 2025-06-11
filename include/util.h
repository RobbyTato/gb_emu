#ifndef UTIL_H
#define UTIL_H

int ultoa_hex(unsigned long value, char *buf);
int utoa_hex(unsigned int value, char *buf);
int ultoa_dec(unsigned long value, char *buf);
int utoa_dec(unsigned int value, char *buf);
void dump_cpu_state(void);
void signal_safe_dump_cpu_state(void);
void dump_cpu_state_gameboy_doctor(void);

#endif // UTIL_H