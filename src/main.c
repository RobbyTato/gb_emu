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
#include <display.h>

void usage(void) {
    printf("Usage: gbemu [OPTIONS]\n");
    printf("Options:\n");
    printf("  -r PATH    ROM path\n");
    printf("  -b         Run boot rom\n");
    printf("  -h         Display this help message\n");
}

int main(int argc, char *argv[])
{   
    char *rom_path = NULL;
    // bool run_boot = false;
    int opt;

    while ((opt = getopt(argc, argv, "r:bh")) != -1) {
        switch (opt) {
            case 'r':
                rom_path = optarg;
                break;
            case 'b':
                // run_boot = true;
                break;
            case 'h':
                usage();
                return 0;
            case '?':
                fprintf(stderr, "Unknown option: %c\n", optopt);
                usage();
                return 1;
        }
    }

    if (rom_path == NULL) {
        fprintf(stderr, "ROM path is required\n");
        usage();
        return 1;
    }

    load_rom(rom_path);

    memcpy(rom, dmg_boot_rom, DMG_BOOT_ROM_SIZE);

    init_display();

    while (true) {
        execute();
        update_display();
        dump_cpu_state();
    }

    free_display();

    free_rom();

    return 0;
}
