#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <getopt.h>
#include <rom.h>
#include <cpu.h>
#include <util.h>

void usage(void) {
    printf("Usage: gbemu [OPTIONS]\n");
    printf("Options:\n");
    printf("  -r PATH    ROM path\n");
    printf("  -b         Run boot rom\n");
    printf("  -h         Display this help message\n");
}

int main(int argc, char *argv[])
{
    // char *rom_path = NULL;
    // // bool run_boot = false;
    // int opt;

    // while ((opt = getopt(argc, argv, "r:bh")) != -1) {
    //     switch (opt) {
    //         case 'r':
    //             rom_path = optarg;
    //             break;
    //         case 'b':
    //             // run_boot = true;
    //             break;
    //         case 'h':
    //             usage();
    //             return 0;
    //         case '?':
    //             fprintf(stderr, "Unknown option: %c\n", optopt);
    //             usage();
    //             return 1;
    //     }
    // }

    // if (rom_path == NULL) {
    //     fprintf(stderr, "ROM path is required\n");
    //     usage();
    //     return 1;
    // }

    // load_rom(rom_path);

    // rom = malloc(3);
    // rom[0] = 0x01;
    // rom[1] = 0x34;
    // rom[2] = 0x12;

    // execute();

    // dump_cpu_state();

    rom = malloc(4);
    rom[0] = 0x21; // LD HL, 0x0FFF
    rom[1] = 0xFF;
    rom[2] = 0x0F;
    rom[3] = 0x09; // ADD HL, BC

    // Initialize BC
    bc.r16 = 0x0001;

    execute();

    dump_cpu_state();

    execute();

    dump_cpu_state();

    return 0;
}
