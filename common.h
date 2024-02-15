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
// The time parameter in PidiCmd represents at which millisecond the cmd should start playing
// The len paramter in PidiCmd represents how many milliseconds the cmd should take to be completed

u32 PIDI_MAGIC = (((u32)'P') << 24) | (((u32)'I') << 16) | (((u32)'D') << 8) | (((u32)'I') << 0);

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

#define MAX_VELOCITY UINT16_MAX
// @Memory: Having a cmd struct for each on/off is useful for a simple implementation & translation from MIDI, but increases memory requirements by double
// Alternative approach would encode the length for playing the cmd
// A PidiCmd represents whether a note should be played or stopped being played, the time at which this should happen and which specific key on the piano this command refers to
typedef struct {
    u64  time;     // The millisecond after song-start on which to start playing this note
    u8   velocity; // The strength with which the note should be played
    i8   octave;   // Which octave the key should be played on (zero is the middle octave)
    bool on;       // Whether the note should be played (true) or not (false)
    PianoKey key;  // The note's key
} PidiCmd;
AIL_DA_INIT(PidiCmd);

#define ENCODED_MUSIC_CHUNK_LEN 13

void encode_cmd(AIL_Buffer *buf, PidiCmd cmd) {
    ail_buf_write8lsb(buf, cmd.time);
    ail_buf_write1(buf, cmd.velocity);
    ail_buf_write1(buf, cmd.key);
    ail_buf_write1(buf, (u8) cmd.octave);
    ail_buf_write1(buf, (u8) cmd.on);
}

PidiCmd decode_cmd(AIL_Buffer *buf) {
    PidiCmd cmd;
    cmd.time     = ail_buf_read8lsb(buf);
    cmd.velocity = ail_buf_read1(buf);
    cmd.key      = ail_buf_read1(buf);
    cmd.octave   = (i8) ail_buf_read1(buf);
    cmd.on       = (bool) ail_buf_read1(buf);
    return cmd;
}

void print_cmd(PidiCmd c)
{
    static const char *key_strs[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    DBG_LOG("{ key: %2s, octave: %2d, on: %c, time: %lld, velocity: %d }\n", key_strs[c.key], c.octave, c.on ? 'y' : 'n', c.time, c.velocity);
}

typedef struct {
    char *name;  // Name of the Song, that is shown in the UI
    u64   len;   // Length in milliseconds of the entire Song
    AIL_DA(PidiCmd) cmds;
} Song;
AIL_DA_INIT(Song);

void print_song(Song song)
{
    DBG_LOG("{\n  name: %s\n  len: %lldms\n  cmds: [\n", song.name, song.len);
    for (u32 i = 0; i < song.cmds.len; i++) {
        DBG_LOG("  ");
        print_cmd(song.cmds.data[i]);
    }
    DBG_LOG("  ]\n}\n");
}


//////////////
//   PDIL   //
//////////////

u32 PDIL_MAGIC = (((u32)'P') << 24) | (((u32)'D') << 16) | (((u32)'I') << 8) | (((u32)'L') << 0);


//////////////
//   SPPP   //
//////////////

#define BAUD_RATE 9600UL // 230400UL
#define CMDS_LIST_LEN (500 / ENCODED_MUSIC_CHUNK_LEN)
#define MAX_CLIENT_MSG_SIZE (12 + MAX_CHUNKS_AMOUNT)
#define MAX_SERVER_MSG_SIZE 12

u32 SPPP_MAGIC = (((u32)'S') << 24) | (((u32)'P') << 16) | (((u32)'P') << 8) | (((u32)'P') << 0);

typedef enum ClientMsgType {
  MSG_NONE = 0, // If no message came in
  MSG_PING = (((u32)'P') << 24) | (((u32)'I') << 16) | (((u32)'N') << 8) | (((u32)'G') << 0),
  MSG_PIDI = (((u32)'P') << 24) | (((u32)'I') << 16) | (((u32)'D') << 8) | (((u32)'I') << 0),
  MSG_STOP = (((u32)'S') << 24) | (((u32)'T') << 16) | (((u32)'O') << 8) | (((u32)'P') << 0),
  MSG_CONT = (((u32)'C') << 24) | (((u32)'O') << 16) | (((u32)'N') << 8) | (((u32)'T') << 0),
  MSG_LOUD = (((u32)'L') << 24) | (((u32)'O') << 16) | (((u32)'U') << 8) | (((u32)'D') << 0),
  MSG_SPED = (((u32)'S') << 24) | (((u32)'P') << 16) | (((u32)'E') << 8) | (((u32)'D') << 0),
} ClientMsgType;

typedef enum ServerMsgType {
  MSG_PONG = (((u32)'P') << 24) | (((u32)'O') << 16) | (((u32)'N') << 8) | (((u32)'G') << 0),
  MSG_SUCC = (((u32)'S') << 24) | (((u32)'U') << 16) | (((u32)'C') << 8) | (((u32)'C') << 0),
} ServerMsgType;

typedef struct ClientMsg {
  ClientMsgType type;
  u64 n; // Dependingon the message type, this is either the time or the length of the cmds array
  PidiCmd *cmds;
} ClientMsg;

#endif // COMMON_H_