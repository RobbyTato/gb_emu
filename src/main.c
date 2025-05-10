#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <getopt.h>
#include <rom.h>
#include <cpu.h>
#include <mem.h>
#include <util.h>
#include <display.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

void segfault_handler(int sig, siginfo_t *info, void *ucontext) {
    ssize_t sink;
    const char msg1[] = "\nSegmentation fault at address: 0x";
    sink = write(STDERR_FILENO, msg1, sizeof(msg1)-1);

    char hex_buf[16];
    int hex_len = ultoa_hex((unsigned long)info->si_addr, hex_buf);
    sink = write(STDERR_FILENO, hex_buf, hex_len);

    const char msg2[] = "\n";
    sink = write(STDERR_FILENO, msg2, sizeof(msg2)-1);

    signal_safe_dump_cpu_state();
    sink = sink;
    _exit(EXIT_FAILURE);
}

void usage(void) {
    printf("Usage: gbemu [OPTIONS]\n");
    printf("Options:\n");
    printf("  -r PATH    ROM path\n");
    printf("  -b         Run boot rom\n");
    printf("  -h         Display this help message\n");
}

int main(int argc, char *argv[])
{   
    struct sigaction sa;
    sa.sa_sigaction = segfault_handler;
    sa.sa_flags = SA_SIGINFO | SA_RESETHAND;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("sigaction");
        return 1;
    }

    char *rom_path = NULL;
    bool run_boot = false;
    int opt;

    while ((opt = getopt(argc, argv, "r:bh")) != -1) {
        switch (opt) {
            case 'r':
                rom_path = optarg;
                break;
            case 'b':
                run_boot = true;
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

    if (run_boot) {
        pc.r16 = 0;
        r_boot_rom_mapped = 0;
    } else {
        pc.r16 = 0x100;
        r_boot_rom_mapped = 1;
    }

    load_rom(rom_path);

    // memcpy(rom, dmg_boot_rom, DMG_BOOT_ROM_SIZE);

    init_display();

    // rom[0x104] = 0; // mess with boot
    
    // Main loop
    while (true) {
        double frame_start = (double)SDL_GetPerformanceCounter();
        // Frame loop
        while (!update_display(frame_start)) {
            execute();
            // dump_cpu_state();
        }
    }

    free_display();

    free_rom();

    return 0;
}
