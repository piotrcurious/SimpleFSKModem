#include <stdlib.h>
#include <stdio.h>
#include <simavr/avr_ioport.h>
#include <simavr/sim_avr.h>
#include <simavr/sim_elf.h>
#include <simavr/sim_vcd_file.h>
#include <simavr/avr_uart.h>

avr_t * avr = NULL;
uint32_t last_edge_cycle = 0;
int last_val = -1;

void pin_changed_hook(struct avr_irq_t * irq, uint32_t value, void * param) {
    uint32_t current_cycle = avr->cycle;
    if (last_val != (int)value) {
        if (last_edge_cycle != 0) {
            uint32_t delta_cycles = current_cycle - last_edge_cycle;
            double us = (double)delta_cycles / (avr->frequency / 1000000.0);
            if (us > 650) { // Highlight potential gaps (> 619us pilot)
                printf("GAP DETECTED: %.2f us at cycle %u\n", us, current_cycle);
            } else {
                printf("EDGE: %d -> %d at cycle %u (delta %u, %.2f us)\n", last_val, value, current_cycle, delta_cycles, us);
            }
        }
        last_edge_cycle = current_cycle;
        last_val = value;
    }
}

void uart_output_hook(struct avr_irq_t * irq, uint32_t value, void * param) {
    if (value != 0) putchar(value);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <elf_file>\n", argv[0]);
        exit(1);
    }

    elf_firmware_t f;
    if (elf_read_firmware(argv[1], &f)) {
         fprintf(stderr, "Failed to read ELF %s\n", argv[1]);
         exit(1);
    }

    avr = avr_make_mcu_by_name("atmega328p");
    avr_init(avr);
    avr->frequency = 16000000;
    avr_load_firmware(avr, &f);

    printf("MCU Frequency: %lu Hz\n", avr->frequency);

    // Track Pin 13 (Port B, Bit 5 on Uno)
    avr_irq_t * irq = avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 5);
    avr_irq_register_notify(irq, pin_changed_hook, NULL);

    // Catch UART output
    avr_irq_t * uart_irq = avr_io_getirq(avr, AVR_IOCTL_UART_GETIRQ('0'), UART_IRQ_OUTPUT);
    avr_irq_register_notify(uart_irq, uart_output_hook, NULL);

    printf("Starting simulation of %s...\n", argv[1]);

    int state = cpu_Running;
    for (int i=0; i<400000000; i++) { // Run for 400M cycles (~25 seconds @ 16MHz)
        state = avr_run(avr);
        if (state == cpu_Done || state == cpu_Crashed) break;
    }

    printf("\nSimulation finished.\n");
    return 0;
}
