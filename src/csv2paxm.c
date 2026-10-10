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

int csv_to_paxm_compiler(const char *csv_input, const char *paxm_output) {
    FILE *csv = fopen(csv_input, "r");
    if (!csv) return PAXM_ERR_IO;

    char line[512];
    PaxMidiSequence *seq = NULL;
    uint32_t prev_absolute_time = 0;

    /* Read spreadsheet file line by line */
    while (fgets(line, sizeof(line), csv)) {
        /* Strip trailing newlines or spaces */
        line[strcspn(line, "\r\n")] = 0;

        /* Skip empty tracker lines */
        if (strlen(line) == 0) continue;

        int track = 0;
        uint32_t abs_time = 0;
        char cmd_name[64];

        /* Parse out standard foundational routing parameters shared by rows */
        int parsed = sscanf(line, "%d, %u, %63[^,]", &track, &abs_time, cmd_name);
        if (parsed < 3) continue;

        /* Clean string pointer by trimming whitespace around the parsed command token */
        char *cmd = cmd_name;
        while (*cmd == ' ') cmd++;
        char *end = cmd + strlen(cmd) - 1;
        while (end > cmd && *end == ' ') { *end = '\0'; end--; }

        /* 1. COMPILE THE HEADER MATRIX BLOCK */
        if (strcmp(cmd, "Header") == 0) {
            int format = 0;
            uint16_t ppq = 480;
            uint16_t bpm = 120;
            uint8_t mode = 0;

            sscanf(line, "%*d, %*u, Header, %d, %hu, %hu, %hhu", &format, &ppq, &bpm, &mode);
            
            /* Initialize your native public domain binary sequence layer */
            seq = paxm_create_sequence(mode, bpm, ppq);
            if (!seq) {
                fclose(csv);
                return PAXM_ERR_MEMORY;
            }
            continue;
        }

        /* Enforce that the sequence header must exist before processing data nodes */
        if (!seq) continue;

        /* Calculate your relative delta ticks step using absolute chronological boundaries */
        uint16_t delta_ticks = (uint16_t)(abs_time - prev_absolute_time);

        /* 2. PARSE AND MAP EVENT PARAMETERS */
        if (strcmp(cmd, "Note_on_c") == 0) {
            uint8_t chan = 0, pitch = 0;
            sscanf(line, "%*d, %*u, Note_on_c, %hhu, %hhu", &chan, &pitch);
            
            paxm_append_event(seq, delta_ticks, (PAXM_CMD_NOTE_ON | (chan & 0x0F)), pitch);
            prev_absolute_time = abs_time;
        } 
        else if (strcmp(cmd, "Note_off_c") == 0) {
            uint8_t chan = 0, pitch = 0;
            sscanf(line, "%*d, %*u, Note_off_c, %hhu, %hhu", &chan, &pitch);
            
            paxm_append_event(seq, delta_ticks, (PAXM_CMD_NOTE_OFF | (chan & 0x0F)), pitch);
            prev_absolute_time = abs_time;
        } 
        else if (strcmp(cmd, "Control_c") == 0) {
            uint8_t chan = 0, cc_num = 0;
            sscanf(line, "%*d, %*u, Control_c, %hhu, %hhu", &chan, &cc_num);
            
            paxm_append_event(seq, delta_ticks, (PAXM_CMD_CC | (chan & 0x0F)), cc_num);
            prev_absolute_time = abs_time;
        } 
        else if (strcmp(cmd, "Tempo") == 0) {
            uint32_t us_per_qnot = 500000;
            sscanf(line, "%*d, %*u, Tempo, %u", &us_per_qnot);
            
            /* Reverse the time calculation framework back into a BPM integer metric */
            uint16_t computed_bpm = (uint16_t)(60000000 / us_per_qnot);
            
            paxm_append_event(seq, delta_ticks, PAXM_CMD_TEMPO_SHIFT, (uint8_t)(computed_bpm & 0xFF));
            prev_absolute_time = abs_time;
        }
        else if (strcmp(cmd, "End_track") == 0) {
            /* End of linear file events block tracked, break loop safely */
            break;
        }
    }

    fclose(csv);

    if (!seq) return PAXM_ERR_IO;

    /* 3. FLUSH RAW COMPILED STRUCT DATA DIRECTLY TO DISK */
    int write_err = paxm_save_file(seq, paxm_output);
    paxm_destroy_sequence(seq);

    return write_err;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <input.csv> <output.pxm>\n", argv);
        return 1;
    }

    printf("Compiling data stream matrix: %s -> %s\n", argv[1], argv[2]);
    int err = csv_to_paxm_compiler(argv[1], argv[2]);

    if (err == PAXM_OK) {
        printf("Compilation complete! Zero-bloat binary file built successfully.\n");
        return 0;
    } else {
        printf("Compilation error encountered: Code %d\n", err);
        return 1;
    }
}