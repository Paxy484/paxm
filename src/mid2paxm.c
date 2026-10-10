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

/* Utility to parse big-endian 16-bit integers from MIDI stream */
uint16_t read_be16(FILE *fp) {
    uint8_t b1 = fgetc(fp);
    uint8_t b2 = fgetc(fp);
    return (uint16_t)((b1 << 8) | b2);
}

/* Utility to parse big-endian 32-bit integers from MIDI stream */
uint32_t read_be32(FILE *fp) {
    uint8_t b1 = fgetc(fp);
    uint8_t b2 = fgetc(fp);
    uint8_t b3 = fgetc(fp);
    uint8_t b4 = fgetc(fp);
    return (uint32_t)((b1 << 24) | (b2 << 16) | (b3 << 8) | b4);
}

/* Decodes Variable-Length Quantities (VLQ) from standard MIDI */
uint32_t read_vlq(FILE *fp) {
    uint32_t value = 0;
    int b;
    do {
        b = fgetc(fp);
        if (b == EOF) return 0;
        value = (value << 7) | (b & 0x7F);
    } while (b & 0x80);
    return value;
}

int convert_midi0_to_paxm(const char *midi_input, const char *paxm_output) {
    FILE *mid = fopen(midi_input, "rb");
    if (!mid) return PAXM_ERR_IO;

    /* 1. VERIFY MIDI HEADER CHUNK (MThd) */
    char magic[4];
    if (fread(magic, 1, 4, mid) != 4 || memcmp(magic, "MThd", 4) != 0) {
        fclose(mid);
        return PAXM_ERR_MAGIC;
    }

    uint32_t header_len = read_be32(mid);
    uint16_t format     = read_be16(mid);
    uint16_t num_tracks = read_be16(mid);
    uint16_t division   = read_be16(mid);

    /* Enforce strict format constraints */
    if (format != 0) {
        fclose(mid);
        return PAXM_ERR_VERSION; /* Enforce Format 0 only */
    }

    /* Skip any extra header bytes if length > 6 */
    if (header_len > 6) {
        fseek(mid, header_len - 6, SEEK_CUR);
    }

    /* 2. FIND AND ENTER TRACK CHUNK (MTrk) */
    if (fread(magic, 1, 4, mid) != 4 || memcmp(magic, "MTrk", 4) != 0) {
        fclose(mid);
        return PAXM_ERR_MAGIC;
    }
    uint32_t track_len = read_be32(mid);
    long track_end = ftell(mid) + track_len;

    /* Initialize tracking variables */
    uint16_t initial_bpm = 120; /* Standard MIDI fallback */
    uint16_t ppq = division & 0x7FFF; /* Pull ticks per quarter note */

    /* Pre-create sequence to safely hold parsed contents */
    PaxMidiSequence *seq = paxm_create_sequence(PAXM_MODE_VERNICHTUNG, initial_bpm, ppq);
    if (!seq) {
        fclose(mid);
        return PAXM_ERR_MEMORY;
    }

    uint8_t running_status = 0;

    /* 3. CORE PARSING LOOP */
    while (ftell(mid) < track_end) {
        uint32_t delta = read_vlq(mid);
        int next_byte = fgetc(mid);
        if (next_byte == EOF) break;

        uint8_t status;
        uint8_t p1;

        if (next_byte & 0x80) {
            status = (uint8_t)next_byte;
            p1 = (uint8_t)fgetc(mid);
        } else {
            status = running_status; /* Apply standard MIDI running status shortcut */
            p1 = (uint8_t)next_byte;
        }

        uint8_t cmd = status & 0xF0;

        /* Isolate standard voice communications hooks */
        if (cmd == 0x80 || cmd == 0x90 || cmd == 0xA0 || cmd == 0xB0 || cmd == 0xE0) {
            running_status = status;
            uint8_t p2 = (uint8_t)fgetc(mid);

            /* Route standard notes and velocity structures straight to paxm */
            if (cmd == 0x80) {
                paxm_append_event(seq, (uint16_t)delta, PAXM_CMD_NOTE_OFF | (status & 0x0F), p1);
            } else if (cmd == 0x90) {
                /* MIDI Note On with 0 velocity is actually a Note Off */
                uint8_t final_cmd = (p2 == 0) ? PAXM_CMD_NOTE_OFF : PAXM_CMD_NOTE_ON;
                paxm_append_event(seq, (uint16_t)delta, final_cmd | (status & 0x0F), p1);
            } else if (cmd == 0xB0) {
                paxm_append_event(seq, (uint16_t)delta, PAXM_CMD_CC | (status & 0x0F), p1);
            }
        } 
        /* Isolate continuous program changes / channel pressure events (2-byte commands) */
        else if (cmd == 0xC0 || cmd == 0xD0) {
            running_status = status;
            /* We don't strictly need program changes right now, skip parameter byte */
        } 
        /* Isolate System Meta Events Hooks */
        else if (status == 0xFF) {
            uint8_t meta_type = (uint8_t)fgetc(mid);
            uint32_t meta_len = read_vlq(mid);

            /* Meta Tempo Event Marker Detected */
            if (meta_type == 0x51 && meta_len == 3) {
                uint8_t m1 = fgetc(mid);
                uint8_t m2 = fgetc(mid);
                uint8_t m3 = fgetc(mid);
                uint32_t microseconds_per_qnot = (m1 << 16) | (m2 << 8) | m3;
                
                /* Calculate BPM from microseconds layout */
                uint16_t target_bpm = (uint16_t)(60000000 / microseconds_per_qnot);
                
                if (seq->header.total_events == 0) {
                    seq->header.initial_bpm = target_bpm;
                } else {
                    paxm_append_event(seq, (uint16_t)delta, PAXM_CMD_TEMPO_SHIFT, (uint8_t)(target_bpm & 0xFF));
                }
            } else {
                /* Skip non-essential metadata arrays (text, tracks labels, etc) */
                fseek(mid, meta_len, SEEK_CUR);
            }
        } 
        /* Isolate SysEx Data Blocks */
        else if (status == 0xF0 || status == 0xF7) {
            uint32_t sysex_len = read_vlq(mid);
            fseek(mid, sysex_len, SEEK_CUR);
        }
    }

    fclose(mid);

    /* 4. FLUSH OUTPUT DATA STRATEGICALLY TO DISK */
    int write_status = paxm_save_file(seq, paxm_output);
    paxm_destroy_sequence(seq);

    return write_status;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <input.mid> <output.pxm>\n", argv[0]);
        return 1;
    }
    
    int err = convert_midi0_to_paxm(argv[1], argv[2]);
    if (err == PAXM_OK) {
        printf("Success! Formatted data written out safely.\n");
        return 0;
    } else {
        printf("Error compiled execution failed: Code %d\n", err);
        return 1;
    }
}