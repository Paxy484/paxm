/*
 * Copyright 2026 Rafael Peixoto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "paxm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Utility to write 16-bit big-endian integers to a file stream */
void write_be16(FILE *fp, uint16_t val) {
    fputc((val >> 8) & 0xFF, fp);
    fputc(val & 0xFF, fp);
}

/* Utility to write 32-bit big-endian integers to a file stream */
void write_be32(FILE *fp, uint32_t val) {
    fputc((val >> 24) & 0xFF, fp);
    fputc((val >> 16) & 0xFF, fp);
    fputc((val >> 8) & 0xFF, fp);
    fputc(val & 0xFF, fp);
}

/* Emits a value as a standard MIDI Variable-Length Quantity (VLQ) */
void write_vlq(FILE *fp, uint32_t val) {
    uint32_t buffer = val & 0x7F;
    while ((val >>= 7) > 0) {
        buffer <<= 8;
        buffer |= 0x80 | (val & 0x7F);
    }
    while (1) {
        fputc(buffer & 0xFF, fp);
        if (buffer & 0x80) {
            buffer >>= 8;
        } else {
            break;
        }
    }
}

int convert_paxm_to_midi0(const char *paxm_input, const char *midi_output) {
    PaxMidiSequence *seq = NULL;
    
    /* Load your clean binary structure via the core API */
    int load_err = paxm_load_file(&seq, paxm_input);
    if (load_err != PAXM_OK) return load_err;

    FILE *mid = fopen(midi_output, "wb");
    if (!mid) {
        paxm_destroy_sequence(seq);
        return PAXM_ERR_IO;
    }

    /* 1. EMIT CANONICAL MIDI HEADER CHUNK (MThd) */
    fwrite("MThd", 1, 4, mid);
    write_be32(mid, 6);        /* Header length is always fixed at 6 bytes */
    write_be16(mid, 0);        /* Force Format 0 Single-Track standard */
    write_be16(mid, 1);        /* Track count is always 1 */
    write_be16(mid, seq->header.ticks_per_qnot);

    /* 2. EMIT TRACK HEADER SIGNATURE (MTrk) */
    fwrite("MTrk", 1, 4, mid);
    long size_offset = ftell(mid);
    write_be32(mid, 0);        /* Placeholder: Will rewrite size at end */

    /* Automatically write initial project tempo layout */
    write_vlq(mid, 0);         /* Tick offset 0 */
    fputc(0xFF, mid);          /* Meta Event marker */
    fputc(0x51, mid);          /* Tempo flag */
    fputc(0x03, mid);          /* Length field */
    uint32_t us_per_qnot = 60000000 / seq->header.initial_bpm;
    fputc((us_per_qnot >> 16) & 0xFF, mid);
    fputc((us_per_qnot >> 8) & 0xFF, mid);
    fputc(us_per_qnot & 0xFF, mid);

    /* 3. CORE TRANSLATION ENCODING LOOP */
    for (uint32_t i = 0; i < seq->header.total_events; i++) {
        PaxMidiEvent ev = seq->event_arr[i];
        
        /* Step A: Emit the compressed relative VLQ delta timing layout */
        write_vlq(mid, ev.delta_ticks);

        uint8_t cmd_type = ev.status & 0xF0;
        uint8_t channel  = ev.status & 0x0F;

        /* Step B: Re-map native commands directly back into standard MIDI syntax frames */
        if (cmd_type == PAXM_CMD_NOTE_ON) {
            fputc(0x90 | channel, mid);
            fputc(ev.payload, mid);
            fputc(100, mid); /* Re-establish basic fixed velocity footprint */
        } 
        else if (cmd_type == PAXM_CMD_NOTE_OFF) {
            fputc(0x80 | channel, mid);
            fputc(ev.payload, mid);
            fputc(0, mid);
        } 
        else if (cmd_type == PAXM_CMD_CC) {
            fputc(0xB0 | channel, mid);
            fputc(ev.payload, mid);
            fputc(127, mid);
        } 
        else if (cmd_type == PAXM_CMD_TEMPO_SHIFT) {
            fputc(0xFF, mid);
            fputc(0x51, mid);
            fputc(0x03, mid);
            uint32_t shift_us = 60000000 / ev.payload;
            fputc((shift_us >> 16) & 0xFF, mid);
            fputc((shift_us >> 8) & 0xFF, mid);
            fputc(shift_us & 0xFF, mid);
        }
    }

    /* 4. TRACK TERMINATOR SEQUENCE */
    write_vlq(mid, 0);
    fputc(0xFF, mid);
    fputc(0x2F, mid);
    fputc(0x00, mid);

    /* 5. DYNAMICALLY RE-WRITE TRACK CHUNK SIZE METRICS */
    long track_end = ftell(mid);
    uint32_t track_length = (uint32_t)(track_end - size_offset - 4);
    fseek(mid, size_offset, SEEK_SET);
    write_be32(mid, track_length);

    fclose(mid);
    paxm_destroy_sequence(seq);
    return PAXM_OK;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <input.pxm> <output.mid>\n", argv);
        return 1;
    }
    
    if (convert_paxm_to_midi0(argv, argv) == PAXM_OK) {
        printf("Success! Output translated seamlessly into standard MIDI format.\n");
        return 0;
    }
    return 1;
}