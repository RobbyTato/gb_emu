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
#include <debug.h>
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
    bool debug_mode = false;
    int opt;

    while ((opt = getopt(argc, argv, "r:bhd")) != -1) {
        switch (opt) {
            case 'r':
                rom_path = optarg;
                break;
            case 'b':
                run_boot = true;
                break;
            case 'd':
                debug_mode = true;
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

    init_display();

    if (debug_mode) {
        init_debug();
    }

    bool quit = false;

    while (!quit) {
        double frame_start = (double)SDL_GetPerformanceCounter();

        // Handle SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    quit = true;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_A) {
                        b_a = true;
                        r_if |= 0x10; // request joypad interrupt
                        continue;
                    } else if (event.key.key == SDLK_S) {
                        b_b = true;
                        r_if |= 0x10; // request joypad interrupt
                        continue;
                    } else if (event.key.key == SDLK_ESCAPE) {
                        b_start = true;
                        r_if |= 0x10; // request joypad interrupt
                        continue;
                    } else if (event.key.key == SDLK_BACKSPACE) {
                        b_select = true;
                        r_if |= 0x10; // request joypad interrupt
                        continue;
                    } else if (event.key.key == SDLK_LEFT) {
                        b_left = true;
                        r_if |= 0x10; // request joypad interrupt
                        continue;
                    } else if (event.key.key == SDLK_RIGHT) {
                        b_right = true;
                        r_if |= 0x10; // request joypad interrupt
                        continue;
                    } else if (event.key.key == SDLK_UP) {
                        b_up = true;
                        r_if |= 0x10; // request joypad interrupt
                        continue;
                    } else if (event.key.key == SDLK_DOWN) {
                        b_down = true;
                        r_if |= 0x10; // request joypad interrupt
                        continue;
                    }
                    break;
                case SDL_EVENT_KEY_UP:
                    if (event.key.key == SDLK_A) {
                        b_a = false;
                        continue;
                    } else if (event.key.key == SDLK_S) {
                        b_b = false;
                        continue;
                    } else if (event.key.key == SDLK_ESCAPE) {
                        b_start = false;
                        continue;
                    } else if (event.key.key == SDLK_BACKSPACE) {
                        b_select = false;
                        continue;
                    } else if (event.key.key == SDLK_LEFT) {
                        b_left = false;
                        continue;
                    } else if (event.key.key == SDLK_RIGHT) {
                        b_right = false;
                        continue;
                    } else if (event.key.key == SDLK_UP) {
                        b_up = false;
                        continue;
                    } else if (event.key.key == SDLK_DOWN) {
                        b_down = false;
                        continue;
                    }
                    break;
            }
        }
        // printf("%d %d %d %d %d %d %d %d\n", b_a, b_b, b_start, b_select, b_left, b_right, b_up, b_down);
        // printf("%X %d %X %X\n", read_mem(0xFF00), ime, r_ie, r_if);

        // Frame loop
        while (!update_display(frame_start)) {
            execute();
            // dump_cpu_state();
        }
        if (debug_mode) {
            update_debug();
        }
    }

    if (debug_mode) {
        free_debug();
    }

    free_display();

    free_rom();

    return 0;
}
