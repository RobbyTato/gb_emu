#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <cpu.h>

reg_t af = {0};
reg_t bc = {0};
reg_t de = {0};
reg_t hl = {0};
reg_t sp = {0};
reg_t pc = {0};

uint64_t dots = 0;

bool ime = false;

uint16_t read_imm16(uint16_t addr) {
    // Little endian
    return (((uint16_t)read_mem(addr + 1)) << 8) | (uint16_t)read_mem(addr);
}

void write_imm16(uint16_t addr, uint16_t val) {
    // Little endian
    write_mem(addr, (uint8_t)(val & 0xFF));
    write_mem(addr + 1, (uint8_t)(val >> 8));
}

uint8_t read_r8(uint8_t idx) {
    switch (idx) {
        case 0:
            return bc.r8.h;
        case 1:
            return bc.r8.l;
        case 2:
            return de.r8.h;
        case 3:
            return de.r8.l;
        case 4:
            return hl.r8.h;
        case 5:
            return hl.r8.l;
        case 6:
            return read_mem(hl.r16);
        case 7:
            return af.r8.h;
    }
    fprintf(stderr, "Invalid read to r8 index %d, exiting...\n", idx);
    exit(1);
}

void write_r8(uint8_t idx, uint8_t val) {
    switch (idx) {
        case 0:
            bc.r8.h = val;
            return;
        case 1:
            bc.r8.l = val;
            return;
        case 2:
            de.r8.h = val;
            return;
        case 3:
            de.r8.l = val;
            return;
        case 4:
            hl.r8.h = val;
            return;
        case 5:
            hl.r8.l = val;
            return;
        case 6:
            write_mem(hl.r16, val);
            return;
        case 7:
            af.r8.h = val;
            return;
    }
    fprintf(stderr, "Invalid write to r8 index %d, exiting...\n", idx);
    exit(1);
}

uint16_t read_r16(uint8_t idx) {
    switch (idx) {
        case 0:
            return bc.r16;
        case 1:
            return de.r16;
        case 2:
            return hl.r16;
        case 3:
            return sp.r16;
    }
    fprintf(stderr, "Invalid read to r16 index %d, exiting...\n", idx);
    exit(1);
}

void write_r16(uint8_t idx, uint16_t val) {
    switch (idx) {
        case 0:
            bc.r16 = val;
            return;
        case 1:
            de.r16 = val;
            return;
        case 2:
            hl.r16 = val;
            return;
        case 3:
            sp.r16 = val;
            return;
    }
    fprintf(stderr, "Invalid write to r16 index %d, exiting...\n", idx);
    exit(1);
}

uint16_t read_r16stk(uint8_t idx) {
    switch (idx) {
        case 0:
            return bc.r16;
        case 1:
            return de.r16;
        case 2:
            return hl.r16;
        case 3:
            return af.r16;
    }
    fprintf(stderr, "Invalid read to r16stk index %d, exiting...\n", idx);
    exit(1);
}

void write_r16stk(uint8_t idx, uint16_t val) {
    switch (idx) {
        case 0:
            bc.r16 = val;
            return;
        case 1:
            de.r16 = val;
            return;
        case 2:
            hl.r16 = val;
            return;
        case 3:
            af.r16 = val;
            af.r8.l &= 0xF0; // clear lower nibble of flags reg
            return;
    }
    fprintf(stderr, "Invalid write to r16stk index %d, exiting...\n", idx);
    exit(1);
}

uint8_t read_r16mem(uint8_t idx) {
    switch (idx) {
        case 0:
            return read_mem(bc.r16);
        case 1:
            return read_mem(de.r16);
        case 2:
            return read_mem(hl.r16++);
        case 3:
            return read_mem(hl.r16--);
    }
    fprintf(stderr, "Invalid read to r16mem index %d, exiting...\n", idx);
    exit(1);
}

void write_r16mem(uint8_t idx, uint8_t val) {
    switch (idx) {
        case 0:
            write_mem(bc.r16, val);
            return;
        case 1:
            write_mem(de.r16, val);
            return;
        case 2:
            write_mem(hl.r16++, val);
            return;
        case 3:
            write_mem(hl.r16--, val);
            return;
    }
    fprintf(stderr, "Invalid write to r16mem index %d, exiting...\n", idx);
    exit(1);
}

uint8_t get_z_flag(void) {
    return af.r8.l >> 7;
}

uint8_t get_n_flag(void) {
    return (af.r8.l >> 6) & 0x1;
}

uint8_t get_h_flag(void) {
    return (af.r8.l >> 5) & 0x1;
}

uint8_t get_c_flag(void) {
    return (af.r8.l >> 4) & 0x1;
}

typedef enum {
    SET_0 = false,
    SET_1 = true,
    LEAVE
} set_flag_t;

void update_flags(set_flag_t z, set_flag_t n, set_flag_t h, set_flag_t c) {
    uint8_t zb = 0, nb = 0, hb = 0, cb = 0;
    switch (z) {
        case SET_0:
            zb = 0;
            break;
        case SET_1:
            zb = 1;
            break;
        case LEAVE:
            zb = get_z_flag();
            break;
    }
    switch (n) {
        case SET_0:
            nb = 0;
            break;
        case SET_1:
            nb = 1;
            break;
        case LEAVE:
            nb = get_n_flag();
            break;
    }
    switch (h) {
        case SET_0:
            hb = 0;
            break;
        case SET_1:
            hb = 1;
            break;
        case LEAVE:
            hb = get_h_flag();
            break;
    }
    switch (c) {
        case SET_0:
            cb = 0;
            break;
        case SET_1:
            cb = 1;
            break;
        case LEAVE:
            cb = get_c_flag();
            break;
    }
    af.r8.l = (uint8_t)(((((zb << 1) | nb) << 2) | ((hb << 1) | cb)) << 4);
}

bool check_cond(uint8_t idx) {
    switch (idx) {
        case 0: // nz
            return !get_z_flag();
        case 1: // z
            return get_z_flag();
        case 2: // nc
            return !get_c_flag();
        case 3: // c
            return get_c_flag();
    }
    fprintf(stderr, "Invalid condition index %d, exiting...\n", idx);
    exit(1);
}

void execute(void) {
    // Service Interrupts

    if (ime) {
        for (int i = 1; i != 0x20; i <<= 1) { // loop over bitmasks
            if ((r_ie & i) && (r_if & i)) {
                r_if &= ~i;
                ime = false;
                sp.r16 -= 2;
                write_imm16(sp.r16, pc.r16);
                dots += 20;
                switch (i) {
                    case 0x01: // VBlank
                        pc.r16 = 0x40; 
                        break;
                    case 0x02: // STAT
                        pc.r16 = 0x48; 
                        break; 
                    case 0x04: // Timer
                        pc.r16 = 0x50; 
                        break; 
                    case 0x08: // Serial
                        pc.r16 = 0x58; 
                        break; 
                    case 0x10: // Joypad
                        pc.r16 = 0x60; 
                        break; 
                }
                return;
            }
        }
    }

    uint8_t inst = read_mem(pc.r16);

    switch (inst & 0xC0) {
    
    case 0x0:
        switch (inst & 0xF) {
            case 0x1: // ld r16, imm16
                write_r16((inst & 0x30) >> 4, read_imm16(pc.r16 + 1));
                dots += 12;
                pc.r16 += 3;
                return;
            case 0x2: // ld [r16mem], a
                write_r16mem((inst & 0x30) >> 4, af.r8.h);
                dots += 8;
                pc.r16++;
                return;
            case 0xA: // ld a, [r16mem]
                af.r8.h = read_r16mem((inst & 0x30) >> 4);
                dots += 8;
                pc.r16++;
                return;
            case 0x3: { // inc r16
                uint8_t idx = (inst & 0x30) >> 4;
                write_r16(idx, read_r16(idx) + 1);
                dots += 8;
                pc.r16++;
                return;
            }
            case 0xB: { // dec r16
                uint8_t idx = (inst & 0x30) >> 4;
                write_r16(idx, read_r16(idx) - 1);
                dots += 8;
                pc.r16++;
                return;
            }
            case 0x9: { // add hl, r16
                uint8_t idx = (inst & 0x30) >> 4;
                uint16_t r16 = read_r16(idx);
                uint16_t res = hl.r16 + r16;
                update_flags(
                    LEAVE,
                    SET_0,
                    ((res ^ hl.r16 ^ r16) & 0x1000) != 0,
                    (res < hl.r16) || (res < r16)
                );
                hl.r16 = res;
                dots += 8;
                pc.r16++;
                return;
            }
        }
        switch (inst & 0x7) {
            case 0x4: { // inc r8
                uint8_t idx = (inst & 0x38) >> 3;
                uint8_t r8 = read_r8(idx);
                uint8_t res = r8 + 1;
                update_flags(
                    res == 0,
                    SET_0,
                    ((res ^ r8 ^ 1) & 0x10) != 0,
                    LEAVE
                );
                write_r8(idx, res);
                if (idx == 6) {
                    dots += 12;
                } else {
                    dots += 4;
                }  
                pc.r16++;
                return;
            }
            case 0x5: { // dec r8
                uint8_t idx = (inst & 0x38) >> 3;
                uint8_t r8 = read_r8(idx);
                uint8_t res = r8 - 1;
                update_flags(
                    res == 0,
                    SET_1,
                    ((res ^ r8 ^ 1) & 0x10) != 0,
                    LEAVE
                );
                write_r8(idx, res);
                if (idx == 6) {
                    dots += 12;
                } else {
                    dots += 4;
                }  
                pc.r16++;
                return;
            }
            case 0x6: // ld r8, imm8
                write_r8((inst & 0x38) >> 3, read_mem(pc.r16 + 1));
                if (((inst & 0x38) >> 3) == 6) {
                    dots += 12;
                } else {
                    dots += 8;
                }
                pc.r16 += 2;
                return;
        }
        if ((inst & 0x27) == 0x20) { // jr cond, imm8
            uint8_t idx = (inst & 0x18) >> 3;
            if (check_cond(idx)) {
                int16_t jump_offset = (int16_t)(int8_t)read_mem(pc.r16 + 1);
                dots += 12;
                pc.r16 += (uint16_t)jump_offset;
                pc.r16 += 2;
            } else {
                dots += 8;
                pc.r16 += 2;
            }
            return;
        }
        switch (inst) {
            case 0x0: // nop
                dots += 4;
                pc.r16++;
                return;
            case 0x8: // ld [imm16], sp
                write_imm16(read_imm16(pc.r16 + 1), sp.r16);
                dots += 20;
                pc.r16 += 3;
                return;
            case 0x7: { // rlca
                uint8_t left_set = af.r8.h >> 7;
                af.r8.h = (af.r8.h << 1) | left_set;
                update_flags(SET_0, SET_0, SET_0, left_set);
                dots += 4;
                pc.r16++;
                return;
            }
            case 0xF: { // rrca
                uint8_t right_set = af.r8.h & 1;
                af.r8.h = (af.r8.h >> 1) | (right_set << 7);
                update_flags(SET_0, SET_0, SET_0, right_set);
                dots += 4;
                pc.r16++;
                return;
            }
            case 0x17: { // rla
                uint8_t left_set = af.r8.h >> 7;
                af.r8.h = (af.r8.h << 1) | get_c_flag();
                update_flags(SET_0, SET_0, SET_0, left_set);
                dots += 4;
                pc.r16++;
                return;
            }
            case 0x1F: { // rra
                uint8_t right_set = af.r8.h & 1;
                af.r8.h = (af.r8.h >> 1) | (get_c_flag() << 7);
                update_flags(SET_0, SET_0, SET_0, right_set);
                dots += 4;
                pc.r16++;
                return;
            }
            case 0x27: { // daa
                uint8_t res = af.r8.h;
                uint8_t adj = 0;
                bool carry = false;

                if (!get_n_flag()) {
                    if (get_h_flag() || (res & 0xF) > 0x9) {
                        adj |= 0x06;
                    }
                    if (get_c_flag() || res > 0x99) {
                        adj |= 0x60;
                        carry = true;
                    }
                    res += adj;
                } else {
                    if (get_h_flag()) {
                        adj |= 0x06;
                    }
                    if (get_c_flag()) {
                        adj |= 0x60;
                        carry = true;
                    }
                    res -= adj;
                }

                af.r8.h = res;
                update_flags(
                    res == 0,
                    LEAVE,
                    SET_0,
                    carry ? SET_1 : LEAVE
                );

                dots += 4;
                pc.r16++;
                return;
            }
            case 0x2F: // cpl
                af.r8.h = ~af.r8.h;
                update_flags(LEAVE, SET_1, SET_1, LEAVE);
                dots += 4;
                pc.r16++;
                return;
            case 0x37: // scf
                update_flags(LEAVE, SET_0, SET_0, SET_1);
                dots += 4;
                pc.r16++;
                return;
            case 0x3F: // ccf
                update_flags(LEAVE, SET_0, SET_0, !get_c_flag());
                dots += 4;
                pc.r16++;
                return;
            case 0x18: { // jr imm8
                int16_t jump_offset = (int16_t)(int8_t)read_mem(pc.r16 + 1);
                dots += 12;
                pc.r16 += (uint16_t)jump_offset;
                pc.r16 += 2;
                return;
            }
            case 0x10: { // stop
                fprintf(stderr, "Reached STOP opcode!\n");
                fprintf(stderr, "Unimplemented instruction, exiting...\n");
                exit(1);
            }
        }
        break;
    
    case 0x40: {
        if (inst == 0x76) { // halt
            fprintf(stderr, "Reached HALT opcode!\n");
            fprintf(stderr, "Unimplemented instruction, exiting...\n");
            exit(1);
        }
        // ld r8, r8
        uint8_t dst = (inst >> 3) & 0x7;
        uint8_t src = inst & 0x7;
        write_r8(dst, read_r8(src));
        if (dst == 6 || src == 6) {
            dots += 8;
        } else {
            dots += 4;
        }
        pc.r16++;
        return;
    }

    case 0x80: {
        switch (inst & 0x38) {
            case 0x0: { // add a, r8
                uint8_t idx = inst & 0x7;
                uint8_t r8 = read_r8(idx);
                uint8_t res = af.r8.h + r8;
                update_flags(
                    res == 0,
                    SET_0,
                    ((res ^ af.r8.h ^ r8) & 0x10) != 0,
                    (res < af.r8.h) || (res < r8)
                );
                af.r8.h = res;
                if (idx == 6) {
                    dots += 8;
                } else {
                    dots += 4;
                }
                pc.r16++;
                return;
            }
            case 0x8: { // adc a, r8
                uint8_t idx = inst & 0x7;
                uint8_t r8 = read_r8(idx);
                uint8_t c = get_c_flag();
                uint8_t res = af.r8.h + (uint8_t)(r8 + c);
                update_flags(
                    res == 0,
                    SET_0,
                    ((res ^ af.r8.h ^ (r8 + c)) & 0x10) != 0,
                    (res < af.r8.h) || (res < (r8 + c))
                );
                af.r8.h = res;
                if (idx == 6) {
                    dots += 8;
                } else {
                    dots += 4;
                }              
                pc.r16++;
                return;
            }
            case 0x10: { // sub a, r8
                uint8_t idx = inst & 0x7;
                uint8_t r8 = read_r8(idx);
                uint8_t res = af.r8.h - r8;
                update_flags(
                    res == 0,
                    SET_1,
                    ((res ^ af.r8.h ^ r8) & 0x10) != 0,
                    r8 > af.r8.h
                );
                af.r8.h = res;
                if (idx == 6) {
                    dots += 8;
                } else {
                    dots += 4;
                }  
                pc.r16++;
                return;
            }
            case 0x18: { // sbc a, r8
                uint8_t idx = inst & 0x7;
                uint8_t r8 = read_r8(idx);
                uint8_t c = get_c_flag();
                uint8_t res = af.r8.h - (uint8_t)(r8 + c);
                update_flags(
                    res == 0,
                    SET_1,
                    ((res ^ af.r8.h ^ (r8 + c)) & 0x10) != 0,
                    (r8 + c) > af.r8.h
                );
                af.r8.h = res;
                if (idx == 6) {
                    dots += 8;
                } else {
                    dots += 4;
                }  
                pc.r16++;
                return;
            }
            case 0x20: { // and a, r8
                uint8_t idx = inst & 0x7;
                uint8_t r8 = read_r8(idx);
                af.r8.h &= r8;
                update_flags(af.r8.h == 0, SET_0, SET_1, SET_0);
                if (idx == 6) {
                    dots += 8;
                } else {
                    dots += 4;
                }  
                pc.r16++;
                return;
            }
            case 0x28: { // xor a, r8
                uint8_t idx = inst & 0x7;
                uint8_t r8 = read_r8(idx);
                af.r8.h ^= r8;
                update_flags(af.r8.h == 0, SET_0, SET_0, SET_0);
                if (idx == 6) {
                    dots += 8;
                } else {
                    dots += 4;
                }  
                pc.r16++;
                return;
            }
            case 0x30: { // or a, r8
                uint8_t idx = inst & 0x7;
                uint8_t r8 = read_r8(idx);
                af.r8.h |= r8;
                update_flags(af.r8.h == 0, SET_0, SET_0, SET_0);
                if (idx == 6) {
                    dots += 8;
                } else {
                    dots += 4;
                }  
                pc.r16++;
                return;
            }
            case 0x38: { // cp a, r8
                uint8_t idx = inst & 0x7;
                uint8_t r8 = read_r8(idx);
                uint8_t res = af.r8.h - r8;
                update_flags(
                    res == 0,
                    SET_1,
                    ((res ^ af.r8.h ^ r8) & 0x10) != 0,
                    r8 > af.r8.h
                );
                if (idx == 6) {
                    dots += 8;
                } else {
                    dots += 4;
                }  
                pc.r16++;
                return;
            }
        }
        break;
    }

    case 0xC0: {
        switch (inst & 0x27) {
            case 0x0: { // ret cond
                uint8_t idx = (inst & 0x18) >> 3;
                if (check_cond(idx)) {
                    dots += 20;
                    pc.r16 = read_imm16(sp.r16);
                    sp.r16 += 2;
                } else {
                    dots += 8;
                    pc.r16++;
                }
                return;
            }
            case 0x2: { // jp cond, imm16
                uint8_t idx = (inst & 0x18) >> 3;
                if (check_cond(idx)) {
                    dots += 16;
                    pc.r16 = read_imm16(pc.r16 + 1);
                } else {
                    dots += 12;
                    pc.r16 += 3;
                }
                return;
            }
            case 0x4: { // call cond, imm16
                uint8_t idx = (inst & 0x18) >> 3;
                if (check_cond(idx)) {
                    uint16_t ret_addr = pc.r16 + 3;
                    sp.r16 -= 2;
                    write_imm16(sp.r16, ret_addr);
                    dots += 24;
                    pc.r16 = read_imm16(pc.r16 + 1);
                } else {
                    dots += 12;
                    pc.r16 += 3;
                }
                return;
            }
        }
        if ((inst & 0x7) == 0x7) { // rst tgt3
            uint16_t ret_addr = pc.r16 + 1;
            sp.r16 -= 2;
            write_imm16(sp.r16, ret_addr);
            dots += 16;
            pc.r16 = inst & 0x38;
            return;
        }
        switch (inst & 0xF) {
            case 0x1: { // pop r16stk
                uint8_t idx = (inst & 0x30) >> 4;
                write_r16stk(idx, read_imm16(sp.r16));
                sp.r16 += 2;
                dots += 12;
                pc.r16++;
                return;
            }
            case 0x5: { // push r16stk
                uint8_t idx = (inst & 0x30) >> 4;
                sp.r16 -= 2;
                write_imm16(sp.r16, read_r16stk(idx));
                dots += 16;
                pc.r16++;
                return;
            }
        }
        switch (inst) {
            case 0xC6: { // add a, imm8
                uint8_t imm8 = read_mem(pc.r16 + 1);
                uint8_t res = af.r8.h + imm8;
                update_flags(
                    res == 0,
                    SET_0,
                    ((res ^ af.r8.h ^ imm8) & 0x10) != 0,
                    (res < af.r8.h) || (res < imm8)
                );
                af.r8.h = res;
                dots += 8;
                pc.r16 += 2;
                return;
            }
            case 0xCE: { // adc a, imm8
                uint8_t imm8 = read_mem(pc.r16 + 1);
                uint8_t c = get_c_flag();
                uint8_t res = af.r8.h + (uint8_t)(imm8 + c);
                update_flags(
                    res == 0,
                    SET_0,
                    ((res ^ af.r8.h ^ (imm8 + c)) & 0x10) != 0,
                    (res < af.r8.h) || (res < (imm8 + c))
                );
                af.r8.h = res;
                dots += 8;
                pc.r16 += 2;
                return;
            }
            case 0xD6: { // sub a, imm8
                uint8_t imm8 = read_mem(pc.r16 + 1);
                uint8_t res = af.r8.h - imm8;
                update_flags(
                    res == 0,
                    SET_1,
                    ((res ^ af.r8.h ^ imm8) & 0x10) != 0,
                    imm8 > af.r8.h
                );
                af.r8.h = res;
                dots += 8;
                pc.r16 += 2;
                return;
            }
            case 0xDE: { // sbc a, imm8
                uint8_t imm8 = read_mem(pc.r16 + 1);
                uint8_t c = get_c_flag();
                uint8_t res = af.r8.h - (uint8_t)(imm8 + c);
                update_flags(
                    res == 0,
                    SET_1,
                    ((res ^ af.r8.h ^ (imm8 + c)) & 0x10) != 0,
                    (imm8 + c) > af.r8.h
                );
                af.r8.h = res;
                dots += 8;
                pc.r16 += 2;
                return;
            }
            case 0xE6: { // and a, imm8
                uint8_t imm8 = read_mem(pc.r16 + 1);
                af.r8.h &= imm8;
                update_flags(af.r8.h == 0, SET_0, SET_1, SET_0);
                dots += 8;
                pc.r16 += 2;
                return;
            }
            case 0xEE: { // xor a, imm8
                uint8_t imm8 = read_mem(pc.r16 + 1);
                af.r8.h ^= imm8;
                update_flags(af.r8.h == 0, SET_0, SET_0, SET_0);
                dots += 8;
                pc.r16 += 2;
                return;
            }
            case 0xF6: { // or a, imm8
                uint8_t imm8 = read_mem(pc.r16 + 1);
                af.r8.h |= imm8;
                update_flags(af.r8.h == 0, SET_0, SET_0, SET_0);
                dots += 8;
                pc.r16 += 2;
                return;
            }
            case 0xFE: { // cp a, imm8
                uint8_t imm8 = read_mem(pc.r16 + 1);
                uint8_t res = af.r8.h - imm8;
                update_flags(
                    res == 0,
                    SET_1,
                    ((res ^ af.r8.h ^ imm8) & 0x10) != 0,
                    imm8 > af.r8.h
                );
                dots += 8;
                pc.r16 += 2;
                return;
            }
            case 0xC9: // ret
                dots += 16;
                pc.r16 = read_imm16(sp.r16);
                sp.r16 += 2;
                return;
            case 0xD9: // reti
                dots += 16;
                pc.r16 = read_imm16(sp.r16);
                sp.r16 += 2;
                ime = true;
                return;
            case 0xC3: // jp imm16
                dots += 16;
                pc.r16 = read_imm16(pc.r16 + 1);
                return;
            case 0xE9: // jp hl
                dots += 4;
                pc.r16 = hl.r16;
                return;
            case 0xCD: { // call imm16
                uint16_t ret_addr = pc.r16 + 3;
                sp.r16 -= 2;
                write_imm16(sp.r16, ret_addr);
                dots += 24;
                pc.r16 = read_imm16(pc.r16 + 1);
                return;
            }
            case 0xE2: // ldh [c], a
                write_mem(0xFF00 | bc.r8.l, af.r8.h);
                dots += 8;
                pc.r16++;
                return;
            case 0xE0: // ldh [imm8], a
                write_mem(0xFF00 | read_mem(pc.r16 + 1), af.r8.h);
                dots += 12;
                pc.r16 += 2;
                return;
            case 0xEA: // ld [imm16], a
                write_mem(read_imm16(pc.r16 + 1), af.r8.h);
                dots += 16;
                pc.r16 += 3;
                return;
            case 0xF2: // ldh a, [c]
                af.r8.h = read_mem(0xFF00 | bc.r8.l);
                dots += 8;
                pc.r16++;
                return;
            case 0xF0: // ldh a, [imm8]
                af.r8.h = read_mem(0xFF00 | read_mem(pc.r16 + 1));
                dots += 12;
                pc.r16 += 2;
                return;
            case 0xFA: // ld a, [imm16]
                af.r8.h = read_imm16(pc.r16 + 1);
                dots += 16;
                pc.r16 += 3;
                return;
            case 0xE8: { // add sp, imm8
                uint8_t imm8 = read_mem(pc.r16 + 1);
                uint16_t offset = (uint16_t)(int16_t)(int8_t)imm8;
                uint16_t res = sp.r16 + offset;
                update_flags(
                    SET_0,
                    SET_0,
                    ((res ^ sp.r16 ^ offset) & 0x10) != 0,
                    ((res ^ sp.r16 ^ offset) & 0x100) != 0
                );
                sp.r16 = res;
                dots += 16;
                pc.r16 += 2;
                return;
            }
            case 0xF8: { // ld hl, sp + imm8
                uint8_t imm8 = read_mem(pc.r16 + 1);
                uint16_t offset = (uint16_t)(int16_t)(int8_t)imm8;
                uint16_t res = sp.r16 + offset;
                update_flags(
                    SET_0,
                    SET_0,
                    ((res ^ sp.r16 ^ offset) & 0x10) != 0,
                    ((res ^ sp.r16 ^ offset) & 0x100) != 0
                );
                hl.r16 = res;
                dots += 12;
                pc.r16 += 2;
                return;
            }
            case 0xF9: // ld sp, hl
                sp.r16 = hl.r16;
                dots += 8;
                pc.r16++;
                return;
            case 0xF3: // di
                ime = false;
                dots += 4;
                pc.r16++;
                return;
            case 0xFB: // ei
                ime = true; // TODO: somehow find a way to set this after the
                            // TODO: next instruction runs
                dots += 4;
                pc.r16++;
                return;
            case 0xCB: { // prefix
            uint8_t inst2 = read_mem(pc.r16 + 1);
            switch (inst2 & 0xC0) {
            case 0x00: {
                switch (inst2 & 0x38) {
                    case 0x0: { // rlc r8
                        uint8_t idx = inst2 & 0x7;
                        uint8_t r8 = read_r8(idx);
                        uint8_t left_set = r8 >> 7;
                        uint8_t res = (r8 << 1) | left_set;
                        update_flags(res == 0, SET_0, SET_0, left_set);
                        write_r8(idx, res);
                        if (idx == 6) {
                            dots += 16;
                        } else {
                            dots += 8;
                        }  
                        pc.r16 += 2;
                        return;
                    }
                    case 0x8: { // rrc r8
                        uint8_t idx = inst2 & 0x7;
                        uint8_t r8 = read_r8(idx);
                        uint8_t right_set = r8 & 1;
                        uint8_t res = (r8 >> 1) | (right_set << 7);
                        update_flags(res == 0, SET_0, SET_0, right_set);
                        write_r8(idx, res);
                        if (idx == 6) {
                            dots += 16;
                        } else {
                            dots += 8;
                        }  
                        pc.r16 += 2;
                        return;
                    }
                    case 0x10: { // rl r8
                        uint8_t idx = inst2 & 0x7;
                        uint8_t r8 = read_r8(idx);
                        uint8_t carry = get_c_flag();
                        uint8_t left_set = r8 >> 7;
                        uint8_t res = (r8 << 1) | carry;
                        update_flags(res == 0, SET_0, SET_0, left_set);
                        write_r8(idx, res);
                        if (idx == 6) {
                            dots += 16;
                        } else {
                            dots += 8;
                        }  
                        pc.r16 += 2;
                        return;
                    }
                    case 0x18: { // rr r8
                        uint8_t idx = inst2 & 0x7;
                        uint8_t r8 = read_r8(idx);
                        uint8_t carry = get_c_flag();
                        uint8_t right_set = r8 & 1;
                        uint8_t res = (r8 >> 1) | (carry << 7);
                        update_flags(res == 0, SET_0, SET_0, right_set);
                        write_r8(idx, res);
                        if (idx == 6) {
                            dots += 16;
                        } else {
                            dots += 8;
                        }  
                        pc.r16 += 2;
                        return;
                    }
                    case 0x20: { // sla r8
                        uint8_t idx = inst2 & 0x7;
                        uint8_t r8 = read_r8(idx);
                        uint8_t left_set = r8 >> 7;
                        uint8_t res = r8 << 1;
                        update_flags(res == 0, SET_0, SET_0, left_set);
                        write_r8(idx, res);
                        if (idx == 6) {
                            dots += 16;
                        } else {
                            dots += 8;
                        }  
                        pc.r16 += 2;
                        return;
                    }
                    case 0x28: { // sra r8
                        uint8_t idx = inst2 & 0x7;
                        uint8_t r8 = read_r8(idx);
                        uint8_t right_set = r8 & 1;
                        uint8_t res = (uint8_t)(((int8_t)r8) >> 1);
                        update_flags(res == 0, SET_0, SET_0, right_set);
                        write_r8(idx, res);
                        if (idx == 6) {
                            dots += 16;
                        } else {
                            dots += 8;
                        }  
                        pc.r16 += 2;
                        return;
                    }
                    case 0x30: { // swap r8
                        uint8_t idx = inst2 & 0x7;
                        uint8_t r8 = read_r8(idx);
                        uint8_t up = r8 & 0xF0;
                        uint8_t lo = r8 & 0x0F;
                        uint8_t res = (up >> 4) | (lo << 4);
                        update_flags(res == 0, SET_0, SET_0, SET_0);
                        write_r8(idx, res);
                        if (idx == 6) {
                            dots += 16;
                        } else {
                            dots += 8;
                        }  
                        pc.r16 += 2;
                        return;
                    }
                    case 0x38: { // srl r8
                        uint8_t idx = inst2 & 0x7;
                        uint8_t r8 = read_r8(idx);
                        uint8_t right_set = r8 & 1;
                        uint8_t res = r8 >> 1;
                        update_flags(res == 0, SET_0, SET_0, right_set);
                        write_r8(idx, res);
                        if (idx == 6) {
                            dots += 16;
                        } else {
                            dots += 8;
                        }  
                        pc.r16 += 2;
                        return;
                    }
                }
                break;
            }
            case 0x40: { // bit b3, r8
                uint8_t idx = inst2 & 0x7;
                uint8_t bit_idx = (inst2 >> 3) & 0x7;
                uint8_t r8 = read_r8(idx);
                update_flags(((r8 >> bit_idx) & 1) == 0, SET_0, SET_1, LEAVE);
                if (idx == 6) {
                    dots += 12;
                } else {
                    dots += 8;
                }  
                pc.r16 += 2;
                return;
            }
            case 0x80: { // res b3, r8
                uint8_t idx = inst2 & 0x7;
                uint8_t bit_idx = (inst2 >> 3) & 0x7;
                uint8_t r8 = read_r8(idx);
                uint8_t mask = 1 << bit_idx;
                uint8_t res = r8 & ~(mask);
                write_r8(idx, res);
                if (idx == 6) {
                    dots += 16;
                } else {
                    dots += 8;
                }  
                pc.r16 += 2;
                return;
            }
            case 0xC0: { // set b3, r8
                uint8_t idx = inst2 & 0x7;
                uint8_t bit_idx = (inst2 >> 3) & 0x7;
                uint8_t r8 = read_r8(idx);
                uint8_t mask = 1 << bit_idx;
                uint8_t res = r8 | mask;
                write_r8(idx, res);
                if (idx == 6) {
                    dots += 16;
                } else {
                    dots += 8;
                }  
                pc.r16 += 2;
                return;
            }
            }
            }
        }
        break;
    }
    }

    fprintf(stderr, "Invalid opcode 0x%X, exiting...\n", inst);
    exit(1);
}
