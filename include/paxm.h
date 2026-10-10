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

#ifndef PAXM_H
#define PAXM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==========================================================================
 * 1. DESIGN SPECIFICATION & CONSTANTS
 * ========================================================================== */

#define PAXM_MAGIC_STR "PAXM"
#define PAXM_VERSION_1 0x0001

/* 
 * Native High-Speed / Production Automation Commands 
 * Packed directly into the upper 4 bits of the Event Status byte.
 */
#define PAXM_CMD_NOTE_OFF          0x80u  /* Kill voice playback loop */
#define PAXM_CMD_NOTE_ON           0x90u  /* Fire raw voice loop / soundcard chip */
#define PAXM_CMD_CC                0xB0u  /* Standard continuous controller hook */
#define PAXM_CMD_GROWL_FORMANT     0xC0u  /* Delay Lama Vowel Macro Array Switch */
#define PAXM_CMD_KICK_DISTORTION   0xD0u  /* Frenchcore Bass Saturation Automation */
#define PAXM_CMD_TEMPO_SHIFT       0xE0u  /* Mid-stream high-BPM temporal adjustment */
#define PAXM_CMD_SYSTEM            0xF0u  /* Engine specific framework controls */

/* 
 * Operational Pipeline Routing Modes
 * Determines how the underlying sequencer maps note commands.
 */
#define PAXM_MODE_NATIVE           0x00u  /* Pure Frenchcore / 8-String / Growl mode */
#define PAXM_MODE_VERNICHTUNG      0x01u  /* Active Multi-Track Metal Production Layer */

/* ==========================================================================
 * GENERAL PAX MIDI (GPM) PATCH ARRANGEMENT SCHEMAS
 * ========================================================================== */

/* 
 * Deathcore Quadrant (Heavy Sub-Register Wall)
 * Active when Mode == PAXM_MODE_VERNICHTUNG or custom GPM routing loops.
 */
#define GPM_PATCH_DC_RHYTHM_8STR   0      /* 8-String Rhythm Guitar: Triggers the Quad-Track Stereo Engine */
#define GPM_PATCH_DC_SAT_SUB_BASS  1      /* Saturated Sub-Bass: Overdriven sub-60Hz tone locking with chugs */
#define GPM_PATCH_DC_LEAD_AMBIENT  2      /* Lead / Ambient Guitar: High-register tracking patch for atmosphere */

/* 
 * Speed Metal Quadrant (High-Velocity Laser Attack)
 * Active when Mode == PAXM_MODE_VERNICHTUNG or custom GPM routing loops.
 */
#define GPM_PATCH_SM_TWIN_LEAD_A   8      /* Twin Lead Guitar A: Left channel harmony tracking solo path */
#define GPM_PATCH_SM_TWIN_LEAD_B   9      /* Twin Lead Guitar B: Right channel harmony tracking solo path */
#define GPM_PATCH_SM_RHYTHM_6STR   10     /* High-Speed 6-String Rhythm Guitar: Downpicking and gallop master */
#define GPM_PATCH_SM_FINGER_BASS   11     /* Aggressive Fingerstyle Bass: Heavy mid-range clank with string rattle */
#define GPM_PATCH_SM_ARCADE_SYNTH  12     /* Retro Arcade Synthesizer: Raw 8-bit square/triangle chip emulator */

/* Electronic Layer Legacy References */
#define GPM_PATCH_DELAY_LAMA       16     /* Delay Lama Vintage Monk Synth */
#define GPM_PATCH_VERN_MUTED_GTR   28     /* Muted Guitar: Used for tight, chugging palm-mutes */

/* ==========================================================================
 * CANONICAL DRUM TRANSMISSION PAYLOADS (CHANNEL 10 MATRICES)
 * ========================================================================== */

/* Deathcore Machine-Gun Alternate Percussion Pins */
#define GPM_DRUM_DC_KICK_ALT_A     35     /* Alternated kick trigger A to bypass chip sample choke */
#define GPM_DRUM_DC_KICK_ALT_B     36     /* Alternated kick trigger B to bypass chip sample choke */
#define GPM_DRUM_DC_SNARE_CRACK    38     /* High-transient crisp metal snare slice for breakdowns */

/* Speed Metal Blast Beat Percussion Pins */
#define GPM_DRUM_SM_KICK_BLAST     37     /* Hyper-Blast Kick: High-transient sample tuned for ultra-fast BPMs */
#define GPM_DRUM_SM_SNARE_BLAST    40     /* Hyper-Blast Snare: Cut-through acoustic snare for blast beats */
#define GPM_DRUM_SM_HIHAT_CLOSED   42     /* Sharp closed hi-hat slice for high velocities */

/* ==========================================================================
 * HARDWARE PLATFORM SPECIFIC OVERRIDES
 * ========================================================================== */

/* 8-String Guitar Register Overrides (Active when Mode == PAXM_MODE_NATIVE) */
#define PAXM_PITCH_8STR_LOW_D      14     /* Double Drop D sub-bass register chug */
#define PAXM_PITCH_8STR_LOW_E      16     /* Standard 8th string open chug (E1) */

/* Vowel positions for the Delay Lama automation macro (PAXM_CMD_GROWL_FORMANT) */
#define PAXM_VOWEL_OOH             0x00u  /* */
#define PAXM_VOWEL_OW              0x01u  /* */
#define PAXM_VOWEL_AH              0x02u  /* */
#define PAXM_VOWEL_AYH             0x03u  /* */
#define PAXM_VOWEL_EEH             0x04u  /* */

/* Error handling telemetry markers */
#define PAXM_OK                    0      /* */
#define PAXM_ERR_MAGIC            (-1)    /* */
#define PAXM_ERR_VERSION          (-2)    /* */
#define PAXM_ERR_MEMORY           (-3)    /* */
#define PAXM_ERR_IO               (-4)    /* */

/* ==========================================================================
 * 2. BINARY STRUCTURE MATRICES (MACHINE-ALIGNED, NO OVERHEAD)
 * ========================================================================== */

#pragma pack(push, 1)                     /* */

/* 
 * Fixed 16-Byte Global File Header Tracker
 * Can be decoded by an Open Watcom compiler in a single hardware cycle.
 */
typedef struct {
    char     magic[4];            /* Strict identifier: Must be "PAXM" */
    uint16_t format_version;      /* Target specification layout tracker (0x0001) */
    uint16_t initial_bpm;         /* Supports up to 65535 BPM (e.g. 220 pure Frenchcore) */
    uint16_t ticks_per_qnot;      /* Temporal grid precision metrics (PPQ) */
    uint32_t total_events;        /* Exact linear count of sequential event vectors */
    uint8_t  production_mode;     /* Holds PAXM_MODE_NATIVE or PAXM_MODE_VERNICHTUNG */
    uint8_t  alignment_pad;       /* Explicit zero-fill placeholder for memory security */
} PaxMidiHeader;

/* 
 * Fixed 4-Byte Event Array Layout
 * Eliminates variable-length quantity (VLQ) compression bloatware entirely.
 */
typedef struct {
    uint16_t delta_ticks;         /* Time offset count relative to previous active event */
    uint8_t  status;              /* Bit 4-7: PAXM_CMD_*, Bit 0-3: Target Track/Channel */
    uint8_t  payload;             /* Parameter value: Pitch, velocity index, or filter cutoff */
} PaxMidiEvent;

#pragma pack(pop)                         /* */

/* 
 * Runtime Sequence Container Context
 * Maps file data cleanly inside memory close to the silicon cache line.
 */
typedef struct {
    PaxMidiHeader header;         /* Cached metadata record */
    PaxMidiEvent *event_arr;      /* Linear stream of sequential timing bytes */
} PaxMidiSequence;

/* ==========================================================================
 * 3. ZERO-ALLOCATION PARSING & SEQUENCER CORE API
 * ========================================================================== */

/* Lifecycle and Disk File Decoding IO Engine */
PaxMidiSequence *paxm_create_sequence(uint8_t mode, uint16_t bpm, uint16_t ppq); /* */
void             paxm_destroy_sequence(PaxMidiSequence *seq); /* */
int              paxm_load_file(PaxMidiSequence **seq_out, const char *filename); /* */
int              paxm_save_file(const PaxMidiSequence *seq, const char *filename); /* */

/* Event Buffer Injection Utilities */
int              paxm_append_event(PaxMidiSequence *seq, uint16_t delta, uint8_t status, uint8_t payload); /* */
int              paxm_insert_note(PaxMidiSequence *seq, uint16_t delta, uint8_t track, uint8_t pitch); /* */

#ifdef __cplusplus
}
#endif

#endif /* PAXM_H */
