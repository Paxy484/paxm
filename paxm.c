/*
 * Copyright 2026 Pax Project
 *
 * Licensed under the Pax Public License, Version 5.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * SPDX-License-Identifier: LicenseRef-Pax-5.0
 *
 * You may obtain a copy of the full License text within the Source or Object 
 * (or Binary) form materials provided with this distribution (typically in a 
 * "LICENSE" file, or "LICENSE-PAX" if there is more than one file named "LICENSE").
 */

#include "paxm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ==========================================================================
 * LIFECYCLE MANAGEMENT
 * ========================================================================== */

PaxMidiSequence *paxm_create_sequence(uint8_t mode, uint16_t bpm, uint16_t ppq) {
    PaxMidiSequence *seq = (PaxMidiSequence *)malloc(sizeof(PaxMidiSequence));
    if (!seq) return NULL;

    /* Initialize fixed-size header fields close to the silicon hardware loop */
    memcpy(seq->header.magic, PAXM_MAGIC_STR, 4);
    seq->header.format_version = PAXM_VERSION_1;
    seq->header.initial_bpm    = bpm;
    seq->header.ticks_per_qnot = ppq;
    seq->header.total_events   = 0;
    seq->header.production_mode = mode;
    seq->header.alignment_pad  = 0;

    seq->event_arr = NULL;
    return seq;
}

void paxm_destroy_sequence(PaxMidiSequence *seq) {
    if (!seq) return;
    if (seq->event_arr) {
        free(seq->event_arr);
    }
    free(seq);
}

/* ==========================================================================
 * HIGH-PERFORMANCE ZERO-ALLOCATION DISK I/O ENGINE
 * ========================================================================== */

int paxm_save_file(const PaxMidiSequence *seq, const char *filename) {
    if (!seq || !filename) return PAXM_ERR_IO;

    FILE *fp = fopen(filename, "wb");
    if (!fp) return PAXM_ERR_IO;

    /* Write sequential fixed structures in single instructions to bypass runtime bloat */
    size_t header_written = fwrite(&(seq->header), sizeof(PaxMidiHeader), 1, fp);
    if (header_written != 1) {
        fclose(fp);
        return PAXM_ERR_IO;
    }

    if (seq->header.total_events > 0 && seq->event_arr) {
        size_t events_written = fwrite(seq->event_arr, sizeof(PaxMidiEvent), seq->header.total_events, fp);
        if (events_written != seq->header.total_events) {
            fclose(fp);
            return PAXM_ERR_IO;
        }
    }

    fclose(fp);
    return PAXM_OK;
}

int paxm_load_file(PaxMidiSequence **seq_out, const char *filename) {
    if (!seq_out || !filename) return PAXM_ERR_IO;

    FILE *fp = fopen(filename, "rb");
    if (!fp) return PAXM_ERR_IO;

    PaxMidiHeader header;
    if (fread(&header, sizeof(PaxMidiHeader), 1, fp) != 1) {
        fclose(fp);
        return PAXM_ERR_IO;
    }

    /* Strict byte verification protocol before granting heap space */
    if (memcmp(header.magic, PAXM_MAGIC_STR, 4) != 0) {
        fclose(fp);
        return PAXM_ERR_MAGIC;
    }
    if (header.format_version != PAXM_VERSION_1) {
        fclose(fp);
        return PAXM_ERR_VERSION;
    }

    PaxMidiSequence *seq = (PaxMidiSequence *)malloc(sizeof(PaxMidiSequence));
    if (!seq) {
        fclose(fp);
        return PAXM_ERR_MEMORY;
    }
    seq->header = header;
    seq->event_arr = NULL;

    if (seq->header.total_events > 0) {
        seq->event_arr = (PaxMidiEvent *)malloc(sizeof(PaxMidiEvent) * seq->header.total_events);
        if (!seq->event_arr) {
            free(seq);
            fclose(fp);
            return PAXM_ERR_MEMORY;
        }

        size_t events_read = fread(seq->event_arr, sizeof(PaxMidiEvent), seq->header.total_events, fp);
        if (events_read != seq->header.total_events) {
            free(seq->event_arr);
            free(seq);
            fclose(fp);
            return PAXM_ERR_IO;
        }
    }

    fclose(fp);
    *seq_out = seq;
    return PAXM_OK;
}

/* ==========================================================================
 * LINEAR TRACK BUFFER INJECTION UTILITIES
 * ========================================================================== */

int paxm_append_event(PaxMidiSequence *seq, uint16_t delta, uint8_t status, uint8_t payload) {
    if (!seq) return PAXM_ERR_IO;

    uint32_t new_count = seq->header.total_events + 1;
    PaxMidiEvent *new_arr = (PaxMidiEvent *)realloc(seq->event_arr, sizeof(PaxMidiEvent) * new_count);
    if (!new_arr) return PAXM_ERR_MEMORY;

    seq->event_arr = new_arr;
    seq->event_arr[seq->header.total_events].delta_ticks = delta;
    seq->event_arr[seq->header.total_events].status      = status;
    seq->event_arr[seq->header.total_events].payload     = payload;
    seq->header.total_events = new_count;

    return PAXM_OK;
}

/* ==========================================================================
 * NATIVE PRODUCTION ENGINE ROUTING PARSER
 * ========================================================================== */

int paxm_insert_note(PaxMidiSequence *seq, uint16_t delta, uint8_t track, uint8_t pitch) {
    if (!seq) return PAXM_ERR_IO;
    uint8_t chan = track & 0x0Fu;

    /* 
     * Mode 1: Vernichtung Multi-Track Production Automation 
     * Automatically maps specialized sub-genre routing architectures.
     */
    if (seq->header.production_mode == PAXM_MODE_VERNICHTUNG) {
        
        /* ------------------------------------------------------------------
         * DEATHCORE QUADRANT AUTOMATION
         * ------------------------------------------------------------------ */
        
        /* 8-String Rhythm Guitar: Triggers automatic quad-tracking and stereo spread */
        if (pitch == GPM_PATCH_DC_RHYTHM_8STR) {
            paxm_append_event(seq, delta, (PAXM_CMD_NOTE_ON | 0x01u), pitch); /* Channel 1: Left Wall */
            paxm_append_event(seq, 0,     (PAXM_CMD_NOTE_ON | 0x02u), pitch); /* Channel 2: Right Wall */
            paxm_append_event(seq, 0,     (PAXM_CMD_NOTE_ON | 0x03u), pitch); /* Channel 3: Mid-Left */
            return paxm_append_event(seq, 0, (PAXM_CMD_NOTE_ON | 0x04u), pitch); /* Channel 4: Mid-Right */
        }
        
        /* Saturated Sub-Bass: Automatically couples with a double-octave foundation layer */
        if (pitch == GPM_PATCH_DC_SAT_SUB_BASS) {
            paxm_append_event(seq, delta, (PAXM_CMD_NOTE_ON | chan), pitch);       /* Base sub-bass drive */
            return paxm_append_event(seq, 0, (PAXM_CMD_NOTE_ON | chan), pitch + 12); /* Harmonic upper saturation octave */
        }

        /* ------------------------------------------------------------------
         * SPEED METAL QUADRANT AUTOMATION
         * ------------------------------------------------------------------ */

        /* Twin Leads: Automatically copies notes to a secondary channel to form instant stereo harmonizations */
        if (pitch == GPM_PATCH_SM_TWIN_LEAD_A) {
            paxm_append_event(seq, delta, (PAXM_CMD_NOTE_ON | 0x05u), pitch); /* Channel 5: Solo Lead A (Left-leaning) */
            return paxm_append_event(seq, 0, (PAXM_CMD_NOTE_ON | 0x06u), pitch); /* Channel 6: Solo Lead B Mirror (Right-leaning) */
        }

        /* 6-String High-Speed Rhythm: Duplicates track payload across separate channels to simulate double-tracked thrash gallops */
        if (pitch == GPM_PATCH_SM_RHYTHM_6STR) {
            paxm_append_event(seq, delta, (PAXM_CMD_NOTE_ON | 0x07u), pitch); /* Channel 7: Rhythm Left */
            return paxm_append_event(seq, 0, (PAXM_CMD_NOTE_ON | 0x08u), pitch); /* Channel 8: Rhythm Right */
        }
    }

    /* Mode 0: Native Single-Voice Fallback Loop */
    return paxm_append_event(seq, delta, (PAXM_CMD_NOTE_ON | chan), pitch);
}
