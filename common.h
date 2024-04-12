#ifndef COMMON_H_
#define COMMON_H_

#ifndef CONST_VAR
#define CONST_VAR
#endif // CONST_VAR

#define AIL_DA_IMPL
#define AIL_BUF_IMPL
#define AIL_RING_IMPL
#include "ail/ail.h"
#include "ail/ail_buf.h"
#include "ail/ail_ring.h"
#include <stdint.h>

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

typedef struct PidiCmd {
    u32 dt   : 12, // Time in ms since previous command
    velocity : 4,  // Strength with which the note should be pressed (0 means, the note will not be played)
    len      : 8,  // Length in centiseconds (1cs = 10ms) for which the note should be played
    octave   : 4,  // The octave of the note (between -8 and 7)
    key      : 4;  // The key of the note
} PidiCmd;
AIL_DA_INIT(PidiCmd);
#define MAX_VELOCITY (1<<4)
#define LEN_FACTOR 10 // Factor by which to multiply a PidiCmd's `len` with, to get the length in ms
#define ENCODED_CMD_LEN 4
AIL_STATIC_ASSERT(ENCODED_CMD_LEN == sizeof(PidiCmd));

static inline u16 pidi_dt(PidiCmd cmd)
{
    return (u16)cmd.dt;
}

static inline u8 pidi_velocity(PidiCmd cmd)
{
    return (u8)cmd.velocity;
}

static inline u8 pidi_len(PidiCmd cmd)
{
    return (u8)cmd.len;
}

static inline i8 pidi_octave(PidiCmd cmd)
{
    // Sign extending the 4-bit octave to 8 bit
    if (cmd.octave & 0x8) return (i8)(0xf0 | cmd.octave);
    else return cmd.octave;
}

static inline PianoKey pidi_key(PidiCmd cmd)
{
    return (PianoKey)cmd.key;
}

static inline void encode_cmd(AIL_Buffer *buf, PidiCmd cmd) {
    AIL_STATIC_ASSERT(ENCODED_CMD_LEN == 4);
    AIL_STATIC_ASSERT(sizeof(PidiCmd) == 4);
    ail_buf_write4lsb(buf, *(u32 *)&cmd);
}

static inline void encode_cmd_simple(u8 *buf, PidiCmd cmd) {
    AIL_STATIC_ASSERT(ENCODED_CMD_LEN == 4);
    AIL_STATIC_ASSERT(sizeof(PidiCmd) == 4);
    u32 c = *(u32 *)&cmd;
    buf[0]  = (c >> 0*8) && 0xff;
    buf[1]  = (c >> 1*8) && 0xff;
    buf[2]  = (c >> 2*8) && 0xff;
    buf[3]  = (c >> 3*8) && 0xff;
}

static inline PidiCmd decode_cmd(AIL_Buffer *buf) {
    AIL_STATIC_ASSERT(ENCODED_CMD_LEN == 4);
    AIL_STATIC_ASSERT(sizeof(PidiCmd) == 4);
    u32 cmd = ail_buf_read4lsb(buf);
    return *(PidiCmd *)&cmd;
}

PidiCmd decode_cmd_simple(u8 *buf) {
    AIL_STATIC_ASSERT(ENCODED_CMD_LEN == 4);
    AIL_STATIC_ASSERT(sizeof(PidiCmd) == 4);
    u32 cmd = ((u64)buf[3] << 3*8) | ((u64)buf[2] << 2*8) | ((u64)buf[1] << 1* 8) | ((u64)buf[0] <<  0*8);
    return *(PidiCmd *)&cmd;
}

typedef struct {
    char *name;  // Name of the Song, that is shown in the UI
    u64   len;   // Length in milliseconds of the entire Song
    AIL_DA(PidiCmd) cmds;
} Song;
AIL_DA_INIT(Song);


//////////////
//   SPPP   //
//////////////

#define BAUD_RATE 9600UL // 230400UL
#define MSG_TIMEOUT 3000 // timeout for reading messages in milliseconds

static const CONST_VAR u32 SPPP_MAGIC = (((u32)'S') << 24) | (((u32)'P') << 16) | (((u32)'P') << 8);

typedef enum ClientMsgType {
    CMSG_NONE      =  0,
    CMSG_PING      = 'P',
    CMSG_CONTINUE  = 'C',
    CMSG_SPEED     = 'S',
    CMSG_VOLUME    = 'V',
    CMSG_MUSIC     = 'M',
    CMSG_NEW_MUSIC = 'N',
} ClientMsgType;

typedef enum ServerMsgType {
    SMSG_NONE    =  0,
    SMSG_PONG    = 'p',
    SMSG_SUCCESS = 's',
    SMSG_REQUEST = 'r',
} ServerMsgType;

typedef struct PlayedKeySPPP {
    u8 len   : 8, // time in centiseconds for which the note should be played
    octave   : 4,
    key      : 4,
    velocity : 4;
} PlayedKeySPPP;
#define SPPP_PK_ENCODED_SIZE 3

typedef struct ClientMsgPidiData {
    u8 pks_count;
    PlayedKeySPPP *played_keys;
    u16 cmds_count;
    PidiCmd *cmds;
} ClientMsgPidiData;

typedef struct ClientMsg {
    ClientMsgType type;
    union {
        u8  b;
        f32 f;
        ClientMsgPidiData pidi;
    } data;
} ClientMsg;


static inline u8 sppp_pk_len(PlayedKeySPPP pk)
{
    return pk.len;
}

static inline i8 sppp_pk_octave(PlayedKeySPPP pk)
{
    if (pk.octave & 0x8) return (i8)(0xf0 | pk.octave); // Sign-extending 4-bit to 8-bit
    else return (i8)pk.octave;
}

static inline PianoKey sppp_pk_key(PlayedKeySPPP pk)
{
    return (PianoKey)pk.key;
}

static inline u8 sppp_pk_velocity(PlayedKeySPPP pk)
{
    return pk.velocity;
}

static inline void encode_played_key(PlayedKeySPPP pk, u8 *buffer)
{
    buffer[0] = pk.len;
    buffer[1] = (pk.octave << 4) | (pk.key);
    buffer[2] = pk.velocity;
}

static inline void encode_played_keys(PlayedKeySPPP pks[], u8 count, u8 *buffer)
{
    for (u8 i = 0; i < count; i++) {
        encode_played_key(pks[i], &buffer[i*SPPP_PK_ENCODED_SIZE]);
    }
}

static inline PlayedKeySPPP decode_played_key(AIL_RingBuffer *rb)
{
    AIL_ASSERT(ail_ring_len(*rb) >= SPPP_PK_ENCODED_SIZE);
    PlayedKeySPPP pk;
    pk.len      = ail_ring_read(rb);
    pk.octave   = ail_ring_peek(*rb) >> 4;
    pk.key      = ail_ring_read(rb) & 0xf;
    pk.velocity = ail_ring_read(rb);
    return pk;
}

#endif // COMMON_H_
