#ifndef COMMON_H_
#define COMMON_H_

#ifndef CONST_VAR
#define CONST_VAR
#endif // CONST_VAR

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

static const CONST_VAR u32 PIDI_MAGIC = (((u32)'P') << 24) | (((u32)'I') << 16) | (((u32)'D') << 8) | (((u32)'I') << 0);

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

#define MAX_VELOCITY UINT8_MAX
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

// @TODO: Rename into ENCODED_CMD_LEN
#define ENCODED_MUSIC_CHUNK_LEN 13

void encode_cmd(AIL_Buffer *buf, PidiCmd cmd) {
    ail_buf_write8lsb(buf, cmd.time);
    ail_buf_write1(buf, cmd.velocity);
    ail_buf_write1(buf, cmd.key);
    ail_buf_write1(buf, (u8) cmd.octave);
    ail_buf_write1(buf, (u8) cmd.on);
}

void encode_cmd_simple(u8 *buf, PidiCmd cmd) {
    buf[0]  = (cmd.time >> 0*8) && 0xff;
    buf[1]  = (cmd.time >> 1*8) && 0xff;
    buf[2]  = (cmd.time >> 2*8) && 0xff;
    buf[3]  = (cmd.time >> 3*8) && 0xff;
    buf[4]  = (cmd.time >> 4*8) && 0xff;
    buf[5]  = (cmd.time >> 5*8) && 0xff;
    buf[6]  = (cmd.time >> 6*8) && 0xff;
    buf[7]  = (cmd.time >> 7*8) && 0xff;
    buf[8]  = cmd.velocity;
    buf[9]  = cmd.key;
    buf[10] = (u8) cmd.octave;
    buf[11] = (u8) cmd.on;
}

PidiCmd decode_cmd(AIL_Buffer *buf) {
    PidiCmd cmd;
    cmd.time     = ail_buf_read8lsb(buf);
    cmd.velocity = ail_buf_read1(buf);
    cmd.key      = ail_buf_read1(buf);
    cmd.octave   = (i8) ail_buf_read1(buf); // @TODO: Check if this cast actually works as expected
    cmd.on       = (bool) ail_buf_read1(buf);
    return cmd;
}

PidiCmd decode_cmd_simple(u8 *buf) {
    PidiCmd cmd;
    cmd.time     = ((u64)buf[7] << 7*8) | ((u64)buf[6] << 6*8) | ((u64)buf[5] << 5*8) | ((u64)buf[4] << 4*8) | ((u64)buf[3] << 3*8) | ((u64)buf[2] << 2*8) | ((u64)buf[1] << 1* 8) | ((u64)buf[0] <<  0*8);
    cmd.velocity = buf[8];
    cmd.key      = buf[9];
    cmd.octave   = *(i8*)(&buf[10]);
    cmd.on       = (bool) buf[11];
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

static const CONST_VAR u32 PDIL_MAGIC = (((u32)'P') << 24) | (((u32)'D') << 16) | (((u32)'I') << 8) | (((u32)'L') << 0);


//////////////
//   SPPP   //
//////////////

#define BAUD_RATE 9600UL // 230400UL
#define MSG_TIMEOUT 2000 // timeout for reading messages in milliseconds

#define KEYS_AMOUNT 88           // Amount of keys on the piano
#define STARTING_KEY PIANO_KEY_A // Lowest key on the piano we use
#define MAX_KEYS_AT_ONCE 10      // The maximum amount of keys to play at once
#define FULL_OCTAVES_AMOUNT ((88 - (PIANO_KEY_AMOUNT - STARTING_KEY))/PIANO_KEY_AMOUNT) // Amount of full octaves (containing all 12 keys) on our piano
#define LAST_OCTAVE_LEN (KEYS_AMOUNT - (FULL_OCTAVES_AMOUNT*PIANO_KEY_AMOUNT + (PIANO_KEY_AMOUNT - STARTING_KEY))) // Amount of keys in the highest (none-full) octave
#define MID_OCTAVE_START_IDX ((PIANO_KEY_AMOUNT - STARTING_KEY) + PIANO_KEY_AMOUNT*(FULL_OCTAVES_AMOUNT/2)) // Number of keys before the frst key in the middle octave on our piano
#define CMDS_LIST_LEN (500 / ENCODED_MUSIC_CHUNK_LEN)
#define MAX_CLIENT_MSG_SIZE (16 + KEYS_AMOUNT + CMDS_LIST_LEN*ENCODED_MUSIC_CHUNK_LEN)
#define MAX_SERVER_MSG_SIZE 12

static const CONST_VAR u32 SPPP_MAGIC = (((u32)'S') << 24) | (((u32)'P') << 16) | (((u32)'P') << 8) | (((u32)'P') << 0);

typedef enum ClientMsgType {
    CMSG_NONE = 0, // If no message came in
    CMSG_PING = (((u32)'P') << 24) | (((u32)'I') << 16) | (((u32)'N') << 8) | (((u32)'G') << 0),
    CMSG_PIDI = (((u32)'P') << 24) | (((u32)'I') << 16) | (((u32)'D') << 8) | (((u32)'I') << 0),
    CMSG_STOP = (((u32)'S') << 24) | (((u32)'T') << 16) | (((u32)'O') << 8) | (((u32)'P') << 0),
    CMSG_CONT = (((u32)'C') << 24) | (((u32)'O') << 16) | (((u32)'N') << 8) | (((u32)'T') << 0),
    CMSG_LOUD = (((u32)'L') << 24) | (((u32)'O') << 16) | (((u32)'U') << 8) | (((u32)'D') << 0),
    CMSG_SPED = (((u32)'S') << 24) | (((u32)'P') << 16) | (((u32)'E') << 8) | (((u32)'D') << 0),
} ClientMsgType;

typedef enum ServerMsgType {
    SMSG_NONE = 0,
    SMSG_REQP = (((u32)'R') << 24) | (((u32)'E') << 16) | (((u32)'Q') << 8) | (((u32)'P') << 0),
    SMSG_PONG = (((u32)'P') << 24) | (((u32)'O') << 16) | (((u32)'N') << 8) | (((u32)'G') << 0),
    SMSG_SUCC = (((u32)'S') << 24) | (((u32)'U') << 16) | (((u32)'C') << 8) | (((u32)'C') << 0),
} ServerMsgType;

typedef struct ClientMsgPidiData {
    u64 time;
    u64 cmds_count;
    PidiCmd *cmds;
    u32 idx;
    u8 *piano;
} ClientMsgPidiData;

typedef struct ClientMsg {
    ClientMsgType type;
    union {
        f32 f;
        ClientMsgPidiData pidi;
    } data;
} ClientMsg;

static inline u8 get_key(PidiCmd cmd)
{
    i16 key = MID_OCTAVE_START_IDX + PIANO_KEY_AMOUNT*(i16)cmd.octave + (i16)cmd.key;
    if (key < 0) key = (cmd.key < STARTING_KEY)*(PIANO_KEY_AMOUNT) + cmd.key - STARTING_KEY;
    else if (key >= KEYS_AMOUNT) key = KEYS_AMOUNT + cmd.key - LAST_OCTAVE_LEN - (cmd.key >= LAST_OCTAVE_LEN)*PIANO_KEY_AMOUNT;
    AIL_ASSERT(key >= 0);
    AIL_ASSERT(key < KEYS_AMOUNT);
    AIL_STATIC_ASSERT(KEYS_AMOUNT <= UINT8_MAX);
    return (u8) key;
}

static inline void apply_pidi_cmd(u8 piano[KEYS_AMOUNT], PidiCmd *cmds, u32 idx, u32 len, u8 *active_keys_count)
{
    PidiCmd cmd = cmds[idx];
    u8 key = get_key(cmd);
    if      ( cmd.on && !piano[key]) *active_keys_count += 1;
    else if (!cmd.on &&  piano[key]) *active_keys_count -= 1;
    piano[key] = cmd.velocity;

    // Prevention-Strategy for exceeding MAX_KEYS_AT_ONCE:
    // Find the next key that would end playing and turn it off already now
    // If no such key can be found (which shouldn't actually happen),
    // then we turn the current key off again
    if (AIL_UNLIKELY(*active_keys_count >= MAX_KEYS_AT_ONCE)) {
        while (++idx < len) {
            u8 k = get_key(cmds[idx]);
            if (!cmds[idx].on && piano[k]) {
                piano[k] = 0;
                return;
            }
        }
        piano[key] = 0;
    }
}

#endif // COMMON_H_