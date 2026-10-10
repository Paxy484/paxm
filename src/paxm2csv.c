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

int paxm_to_mid2csv_format(const char *paxm_input, const char *csv_output) {
    PaxMidiSequence *seq = NULL;
    
    /* Load binary file using your robust engine core */
    int load_err = paxm_load_file(&seq, paxm_input);
    if (load_err != PAXM_OK) {
        return load_err;
    }

    FILE *csv = fopen(csv_output, "w");
    if (!csv) {
        paxm_destroy_sequence(seq);
        return PAXM_ERR_IO;
    }

    /* 
     * 1. THE HEADER ROW (1:1 mid2csv style)
     * Format: Track, Time, "Header", Format(1), PPQ, Initial_BPM, Production_Mode
     */
    fprintf(csv, "1, 0, Header, 1, %u, %u, %u\n", 
            seq->header.ticks_per_qnot,
            seq->header.initial_bpm,
            seq->header.production_mode);

    /* 2. START TRACK MARKER */
    fprintf(csv, "1, 0, Start_track\n");

    uint32_t absolute_time = 0;

    /* 3. CORE MATRIX PARSING LOOP */
    for (uint32_t i = 0; i < seq->header.total_events; i++) {
        PaxMidiEvent ev = seq->event_arr[i];
        
        /* Accumulate absolute timeline position from relative delta ticks */
        absolute_time += ev.delta_ticks;
        
        uint8_t cmd_type = ev.status & 0xF0;
        uint8_t channel  = ev.status & 0x0F;

        /* Map your native PAXM macros straight to mid2csv text signatures */
        if (cmd_type == PAXM_CMD_NOTE_ON) {
            /* Format: Track, Absolute Time, Note_on_c, Channel, Pitch, Velocity index */
            fprintf(csv, "1, %u, Note_on_c, %u, %u, 100\n", absolute_time, channel, ev.payload);
        } 
        else if (cmd_type == PAXM_CMD_NOTE_OFF) {
            /* Format: Track, Absolute Time, Note_off_c, Channel, Pitch, Velocity (0) */
            fprintf(csv, "1, %u, Note_off_c, %u, %u, 0\n", absolute_time, channel, ev.payload);
        } 
        else if (cmd_type == PAXM_CMD_CC) {
            /* Format: Track, Absolute Time, Control_c, Channel, Controller_Num, Value */
            fprintf(csv, "1, %u, Control_c, %u, %u, 127\n", absolute_time, channel, ev.payload);
        } 
        else if (cmd_type == PAXM_CMD_TEMPO_SHIFT) {
            /* Format: Track, Absolute Time, Tempo, Microseconds_Per_Quarter_Note */
            uint32_t us_per_qnot = 60000000 / ev.payload;
            fprintf(csv, "1, %u, Tempo, %u\n", absolute_time, us_per_qnot);
        }
    }

    /* 4. END TRACK AND FILE TERMINATORS */
    fprintf(csv, "1, %u, End_track\n", absolute_time);
    fprintf(csv, "1, 0, End_of_file\n");

    fclose(csv);
    paxm_destroy_sequence(seq);
    return PAXM_OK;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <input.pxm> <output.csv>\n", argv[0]);
        return 1;
    }
    
    if (paxm_to_mid2csv_format(argv[1], argv[2]) == PAXM_OK) {
        printf("Conversion successful! 1:1 mid2csv layout exported.\n");
        return 0;
    }
    return 1;
}