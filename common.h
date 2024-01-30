#ifndef COMMON_H_
#define COMMON_H_

#define AIL_DA_IMPL
#define AIL_BUF_IMPL
#include "ail/ail.h"
#include "ail/ail_buf.h"
#include <stdint.h>

#ifndef DBG_LOG
#ifdef UI_DEBUG
#include <stdio.h> // For printf - only used for debugging
#define DBG_LOG(...) printf(__VA_ARGS__)
#else
#define DBG_LOG(...) do { if (0) printf(__VA_ARGS__); } while(0)
#endif // UI_DEBUG
#endif // DBG_LOG

//////////////
//   PIDI   //
//////////////

// @Note on time: The idea is to use discretized clock-cycles for measuring time.
// The time parameter in MusicChunk represents at which millisecond the chunk should start playing
// The len paramter in MusicChunk represents how many milliseconds the chunk should take to be completed

u32 PIDI_MAGIC = ('P' << 24) | ('I' << 16) | ('D' << 8) | ('I' << 0);

typedef enum {
    PIANO_KEY_C = 0,
    PIANO_KEY_CS,
    PIANO_KEY_D,
    PIANO_KEY_DS,
    PIANO_KEY_E,
    PIANO_KEY_F,
    PIANO_KEY_FS,
    PIANO_KEY_G,
    PIANO_KEY_GS,
    PIANO_KEY_A,
    PIANO_KEY_AS,
    PIANO_KEY_B,
    PIANO_KEY_AMOUNT,
} PianoKey;

// A MusicChunk represents whether a note should be played or stopped being played, the time at which this should happen, how long the transition from not-playing to playing (or vice versa) should take and which note exactly should be played
typedef struct {
    u64  time;   // The clock-cycle on which to start playing this note
    u16  len;    // The amount of clock cycles, that playing this note should take
    PianoKey  key;    // The note's key
    i8   octave; // Which octave the key should be played on (zero is the middle octave)
    bool on;     // Whether the note should be played (true) or not (false)
} MusicChunk;
AIL_DA_INIT(MusicChunk);

#define ENCODED_MUSIC_CHUNK_LEN 13

void encode_chunk(AIL_Buffer *buf, MusicChunk chunk) {
    ail_buf_write8lsb(buf, chunk.time);
    ail_buf_write2lsb(buf, chunk.len);
    ail_buf_write1   (buf, chunk.key);
    ail_buf_write1   (buf, (u8) chunk.octave);
    ail_buf_write1   (buf, (u8) chunk.on);
}

MusicChunk decode_chunk(AIL_Buffer *buf) {
    MusicChunk chunk;
    chunk.time   = ail_buf_read8lsb(buf);
    chunk.len    = ail_buf_read2lsb(buf);
    chunk.key    = ail_buf_read1(buf);
    chunk.octave = (i8) ail_buf_read1(buf);
    chunk.on     = (bool) ail_buf_read1(buf);
    return chunk;
}

void print_chunk(MusicChunk c)
{
    static const char *key_strs[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    DBG_LOG("{ key: %2s, octave: %2d, on: %c, time: %lld, len: %d }\n", key_strs[c.key], c.octave, c.on ? 'y' : 'n', c.time, c.len);
}

typedef struct {
    char *name;  // Name of the Song, that is shown in the UI
    u64   len;   // Length in milliseconds of the entire Song
    AIL_DA(MusicChunk) chunks;
} Song;
AIL_DA_INIT(Song);

void print_song(Song song)
{
    DBG_LOG("{\n  name: %s\n  len: %lldms\n  chunks: [\n", song.name, song.len);
    for (u32 i = 0; i < song.chunks.len; i++) {
        DBG_LOG("  ");
        print_chunk(song.chunks.data[i]);
    }
    DBG_LOG("  ]\n}\n");
}


//////////////
//   PDIL   //
//////////////

u32 PDIL_MAGIC = ('P' << 24) | ('D' << 16) | ('I' << 8) | ('L' << 0);


//////////////
//   SPPP   //
//////////////

u32 SPPP_MAGIC = ('S' << 24) | ('P' << 16) | ('P' << 8) | ('P' << 0);

typedef enum ClientMsgType {
  MSG_NONE = 0, // If no message came in
  MSG_PING = ('P' << 24) | ('I' << 16) | ('N' << 8) | ('G' << 0),
  MSG_PIDI = ('P' << 24) | ('I' << 16) | ('D' << 8) | ('I' << 0),
  MSG_STOP = ('S' << 24) | ('T' << 16) | ('O' << 8) | ('P' << 0),
  MSG_CONT = ('C' << 24) | ('O' << 16) | ('N' << 8) | ('T' << 0),
  MSG_JUMP = ('J' << 24) | ('U' << 16) | ('M' << 8) | ('P' << 0),
} ClientMsgType;

typedef enum ServerMsgType {
  MSG_PONG = ('P' << 24) | ('O' << 16) | ('N' << 8) | ('G' << 0),
  MSG_SUCC = ('S' << 24) | ('U' << 16) | ('C' << 8) | ('C' << 0),
} ServerMsgType;

typedef struct ClientMsg {
  ClientMsgType type;
  u64 n; // Dependingon the message type, this is either the time or the length of the chunks array
  MusicChunk *chunks;
} ClientMsg;

#endif // COMMON_H_