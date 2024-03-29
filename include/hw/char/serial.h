/*
 * QEMU 16550A UART emulation
 *
 * Copyright (c) 2003-2004 Fabrice Bellard
 * Copyright (c) 2008 Citrix Systems, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef HW_SERIAL_H
#define HW_SERIAL_H 1

#include "hw/hw.h"
#include "sysemu/sysemu.h"
#include "exec/memory.h"
#include "qemu/fifo8.h"

#define UART_FIFO_LENGTH    16      /* 16550A Fifo Length */

struct SerialState {
    uint16_t divider;
    uint8_t rbr; /* receive register */
    uint8_t thr; /* transmit holding register */
    uint8_t tsr; /* transmit shift register */
    uint8_t ier;
    uint8_t iir; /* read only */
    uint8_t lcr;
    uint8_t mcr;
    uint8_t lsr; /* read only */
    uint8_t msr; /* read only */
    uint8_t scr;
    uint8_t fcr;
    uint8_t fcr_vmstate; /* we can't write directly this value
                            it has side effects */
    /* NOTE: this hidden state is necessary for tx irq generation as
       it can be reset while reading iir */
    int thr_ipending;
    qemu_irq irq;
    CharDriverState *chr;
    int last_break_enable;
    int it_shift;
    int baudbase;
    int tsr_retry;
    uint32_t wakeup;

    /* Time when the last byte was successfully sent out of the tsr */
    uint64_t last_xmit_ts;
    Fifo8 recv_fifo;
    Fifo8 xmit_fifo;
    /* Interrupt trigger level for recv_fifo */
    uint8_t recv_fifo_itl;

    QEMUTimer *fifo_timeout_timer;
    int timeout_ipending;           /* timeout interrupt pending state */

    uint64_t char_transmit_time;    /* time to transmit a char in ticks */
    int poll_msl;

    QEMUTimer *modem_status_poll;
    MemoryRegion io;
};

extern const VMStateDescription vmstate_serial;
extern const MemoryRegionOps serial_io_ops;

void serial_realize_core(SerialState *s, Error **errp);
void serial_exit_core(SerialState *s);
void serial_set_frequency(SerialState *s, uint32_t frequency);
void serial_change_char_driver(SerialState *s, CharDriverState *chr);
const MemoryRegionOps *serial_get_memops(enum device_endian end);
qemu_irq *serial_get_irq(SerialState *s);

unsigned serial_rx_fifo_count(SerialState *s);
unsigned serial_tx_fifo_count(SerialState *s);

/* legacy pre qom */
SerialState *serial_init(int base, qemu_irq irq, int baudbase,
                         CharDriverState *chr, MemoryRegion *system_io);
SerialState *serial_mm_init(MemoryRegion *address_space,
                            hwaddr base, int it_shift,
                            qemu_irq irq, int baudbase,
                            CharDriverState *chr, enum device_endian end);

/* serial-isa.c */
#define TYPE_ISA_SERIAL "isa-serial"
bool serial_isa_init(ISABus *bus, int index, CharDriverState *chr);

#endif
