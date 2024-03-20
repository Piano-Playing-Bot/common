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

// @Memory: Having a cmd struct for each on/off is useful for a simple implementation & translation from MIDI, but increases memory requirements by double
// Alternative approach would encode the length for playing the cmd
// A PidiCmd represents whether a note should be played or stopped being played, the time at which this should happen and which specific key on the piano this command refers to
typedef struct {
    u16 dt       : 12; // Time in ms since previous command
    u8  velocity : 4;  // Strength with which the note should be pressed (0 means, the note will not be played)
    u8  len      : 8;  // Length in 10ms for which the note should be played
    i8  octave   : 4;  // The octave of the note (between -8 and 7)
    PianoKey key : 4;  // The key of the note
} PidiCmd;
AIL_DA_INIT(PidiCmd);
#define MAX_VELOCITY 1<<4
#define LEN_FACTOR 10 // Factor by which to multiply a PidiCmd's `len` with, to get the length in ms
#define ENCODED_CMD_LEN 4
// AIL_STATIC_ASSERT(ENCODED_CMD_LEN == sizeof(PidiCmd));

static inline void encode_cmd(AIL_Buffer *buf, PidiCmd cmd) {
    AIL_STATIC_ASSERT(ENCODED_CMD_LEN == 4);
    ail_buf_write4lsb(buf, *(u32 *)&cmd);
}

static inline void encode_cmd_simple(u8 *buf, PidiCmd cmd) {
    AIL_STATIC_ASSERT(ENCODED_CMD_LEN == 4);
    u32 c = *(u32 *)&cmd;
    buf[0]  = (c >> 0*8) && 0xff;
    buf[1]  = (c >> 1*8) && 0xff;
    buf[2]  = (c >> 2*8) && 0xff;
    buf[3]  = (c >> 3*8) && 0xff;
}

static inline PidiCmd decode_cmd(AIL_Buffer *buf) {
    AIL_STATIC_ASSERT(ENCODED_CMD_LEN == 4);
    u32 cmd = ail_buf_read4lsb(buf);
    return *(PidiCmd *)&cmd;
}

PidiCmd decode_cmd_simple(u8 *buf) {
    AIL_STATIC_ASSERT(ENCODED_CMD_LEN == 4);
    u32 cmd = ((u64)buf[3] << 3*8) | ((u64)buf[2] << 2*8) | ((u64)buf[1] << 1* 8) | ((u64)buf[0] <<  0*8);
    return *(PidiCmd *)&cmd;
}


void print_cmd(PidiCmd c)
{
    static const char *key_strs[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    DBG_LOG("{ key: %2s, octave: %2d, dt: %d, len: %d, velocity: %d }\n", key_strs[c.key], c.octave, c.dt, c.len*LEN_FACTOR, c.velocity);
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
#define CMDS_LIST_LEN (300 / sizeof(PidiCmd))
#define MAX_CLIENT_MSG_SIZE (12 + 12 + KEYS_AMOUNT + CMDS_LIST_LEN*ENCODED_CMD_LEN)
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
    u32 time;
    u32 cmds_count;
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

typedef struct PlayedKey {
    u8  len;  // time in ms*LEN_FACTOR for which the note should still be played
    u8  idx;  // index of the note in the `piano` array.
} PlayedKey;
AIL_STATIC_ASSERT(KEYS_AMOUNT < UINT8_MAX);

typedef struct PlayedKeyList {
    u32 start_time; // time in ms to which counting the all PlayedKeys lengths is relative to
    u8  count;      // amount of currently played keys
    PlayedKey keys[MAX_KEYS_AT_ONCE];
} PlayedKeyList;
AIL_STATIC_ASSERT(MAX_KEYS_AT_ONCE < UINT8_MAX);


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

#define ARR_UNORDERED_RM(arr, idx, len) (arr)[(idx)] = (arr)[--(len)]

static inline void apply_pidi_cmd(u32 cur_time, PidiCmd cmd, u8 piano[KEYS_AMOUNT], PlayedKeyList *played_keys)
{
    u8 idx = get_key(cmd);
    piano[idx] = cmd.velocity;

    i8 played_idx = -1;
    u8 next_off_key_idx = 0;
    u8 time_offset = cur_time - played_keys->start_time;
    played_keys->start_time = cur_time;
    for (u8 i = 0; i < played_keys->count; i++) {
        if (played_keys->keys[i].len < time_offset) {
            ARR_UNORDERED_RM(played_keys, i, played_keys->count);
            i--;
            continue;
        }
        played_keys->keys[i].len -= time_offset;
        if (played_keys->keys[i].idx == idx) {
            played_idx = i;
        }
        if (played_keys->keys[i].len < played_keys->keys[next_off_key_idx].len) {
            next_off_key_idx = i;
        }
    }

    if (cmd.velocity) {
        if (played_idx < 0) {
            if (played_keys->count == MAX_KEYS_AT_ONCE) {
                piano[played_keys->keys[next_off_key_idx].idx] = 0;
                played_idx = next_off_key_idx;
            } else {
                played_idx = played_keys->count++;
            }
            played_keys->keys[played_idx].idx = idx;
            played_keys->keys[played_idx].len = cmd.len;
        } else {
            played_keys->keys[played_idx].len = cmd.len;
        }
    } else if (played_idx >= 0) {
        ARR_UNORDERED_RM(played_keys->keys, played_idx, played_keys->count);
    }
}

#endif // COMMON_H_