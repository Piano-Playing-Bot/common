This document collects the specifications of all protocols and formats that were created for this project.

# PIDI Format

This section specifies the format of PIDI-files. PIDI serves as a simple and relatively memory-efficient format for storing all data required to play a song automatically on a piano. The format aims to allow microcontrollers to play the music simply and performantly.

## RFCs

### Tempo

Should the format be extended to include a tempo? Currently, any time specifiers are given as a number of abstract, internal clock cycles. A tempo specification would allow explicitly setting the length of each internal clock cycle in real time.

### Velocity implying on/off state

The velocity could be used to imply the on/off state of a key (since the key is off if and only if the velocity is 0), allowing to discard that value. This would increase the format's memory efficiency and incidentally significantly improve its alignment.

As far as I can tell at the moment, this should not further complicate any other logic.

### Memory-Efficiency

The key can only take on values between 0 and 11. It requires thus at most 4 bits, takes up 8 bits under the current format. To save space (and incidentally also improve alignment somewhat), the `on` and `key` section of a chunk could be stored at different bit-offsets in a single byte instead of being stretched over 2 seperate bytes.

The downside of this approach would be the added complexity in en-/decoding the format.

An alternative approach could be to just leave out the `on` byte, as it can be inferred. If all notes are implied to be off at the beginning, a single chunk could simply mean to toggle the given key.

This might however significantly complicate logic for only allowing a certain number of keys to be played at any time.

## Specification

PIDI stands for `PIano Digital Interface`.

A PIDI file follows the following format:

```
<Magic Bytes: 4 bytes> <Commands Count: 4 bytes> <Commands>
```

Unless otherwise specified, small-endian encoding is used.

- **Magic Bytes:**
The format's Magic bytes are: `PIDI`. It is always written in big-endian format. It stands for 'Piano Digital Interface'.

- **Commands Count:**
The amount of commands is given as an unsigned 32-bit number. It should follow exactly as many commands as given here.

- **Commands:**
All commands are written here one after another. The amount of commands is given in `Commands Count`. A single command's format is given below.

### Command Format

Every command follows the following format.

```
<Time: 8 bytes> <Velocity: 1 byte> <Key: 1 byte> <Octave: 1 byte> <On: 1 byte>
```

- **Time:**
The time specifies after how many milliseconds this command should take effect. The time of a command is not allowed to be lower than the time of a previous command. All chunks are thus ordered by their respective time.

- **Velocity:**
The velocity specifies how strongly the note should be played.

- **Key:**
The key gives the specific musical note that is referred to in this command. The key and octave together specify a specific key on a piano. The following keys are encoded as follows:

```
C  = 0
C# = 1
D  = 2
D# = 3
E  = 4
F  = 5
F# = 6
G  = 7
G# = 8
A  = 9
A# = 10
B  = 11
```

- **Octave:**
The octave gives the octave offset of the musical note that this chunk refers to. The key and octave together specify a specific key on a piano.

The octave should be interpreted as a signed 8-bit integer using two's complement. 0 refers to the middle octave on a piano. Negative numbers refer to octaves below the middle one, while positive numbers refer to higher octaves.

- **On:**
This byte represents whether the note should start being played or stop being played. 0 means that the note should stop being played, while any other number means that the note should start being played.


# PDIL Format

This section specifies the format of PDIL-files.

The idea behind PDIL files is to provide a way to permamently store a collection of PIDI files. The UI uses this library to provide an overview of the available songs.

## Specification

PDIL stands for `Piano Digital Interface Library`.

A PDIL file follows the following format:

```
<Magic Bytes: 4 bytes> <Amount: 4 bytes> <Song-Infos>
```

Unless otherwise specified, small-endian encoding is used.

- **Magic Bytes:**
The format's Magic bytes are: `PDIL`. It is always written in big-endian format. It stands for 'Piano Digital Interface Library'.

- **Amount:**
The amount of song infos is given as an unsigned 32-bit number. There are exactly as many song infos as given here.

- **Song Infos:**
All song infos are written here one after another. The amount of song infos is given in `Amount`. A single song info's format is given below.

### Song Info Format

Every Song Info follows the following format:
```
<Name Length: 4 bytes> <Song Length: 8 bytes> <Name>
```

- **Name Length:**
The Length of the song's name. This specifies the amount of bytes that `Name` takes up.

- **Song Length:**
The length of the given song in milliseconds. This number can also be calculated dynamically from a PIDI file if required.

- **Name:**
The filename of the PIDI file, that contains the data for the given song. It should be provided as a relative path from the folder that this PDIL file is in.



# Self-Playing-Piano Protocol

This section outlines the protocol used for the communication between the UI and the microcontroller, that plays the music.

The protocol will refer to the microcontroller as the 'server' and the UI application as the 'client'. The reasoning behind this naming is as follows: The server will idly wait until it receives a message. As in general client-server architectures, the client has to initiate the communication.

When talking about either the client or the server, the term 'node' may also be used in the protocol's specification.

## RFCs

## Specification

Any message between the server and client has the following format:

```
<Magic Bytes: 4 bytes> <Message-Type: 4 bytes> <Size: 4 bytes> <Message: Size bytes>
```

Unless otherwise specified, small-endian encoding is used.

- **Magic Bytes:**
The protocol's Magic bytes are: `SPPP`. It is always written in big-endian format. It stands for 'Self-Playing-Piano Protocol'.

- **Message-Type:**
The different types of messages that exist, are listed below under `Message`. Each message type has its own 4 character signature, by which it is defined. Should a node not recognize the provided message type, it should read the amount of given bytes and skip the message. Version upgrades should easily stay backward-compatible that way.

- **Size:**
The size of the message is given as an unsigned 32-bit number. This size does not include the 4 bytes of the size or the 8 bytes that come before the size.

- **Message:**
There are exactly as many bytes for the message as given in `Size`. How to interpret the message depends on the `Message-Type`. The message's format for each possible type is given below.

### 'PING'

The ping message type exists to check whether the server is available. It can also be used by the client to initially identify the port, on which the server is connected.

Only the client can send this message. The server has to respond with a `PONG` message. The message should be empty or ignored.

### 'PONG'

This message should only be sent by the server as a reply to the `PING` message. The message bytes should be empty or ignored.

### 'SUCC'

The succ message is only sent by the server as a response to the client. It indicates successfully receiving and executing the client's last message. The message bytes should be empty or ignored.

### 'PIDI'

The pidi message type is used for sending a list of commands for playing a Song to the Arduino. The Commands are given in the same format as in [PIDI](#pidi-format) files. Instead of sending the potentially very big PIDI file all at once, we send it in chunks to allow for streaming. The client can initiate sending the initial chunk of a song at any time. The next chunks of the song should however not be sent until the server requests them via the 'REQP' message.

The message should follow this format:

```
<Index: 4 bytes> [<time: 8 bytes> <piano: 88 bytes>] <Commands>
```

The index indicates how many chunks preceeded this one in the song. If and only if the index is 0, the chunk begins a new song.

Only if a new song is started, should the 'time' and 'piano' bytes be given. 'time' should be interpreted as a 64bit unsigned integer giving the time in milliseconds that should be assumed to have already passed before the first command is applied. The piano array of 88 bytes provides the initial configuration of the velocities with which each key of the piano should be played, before the first command should be applied. Usually th piano array will be set completely set to 0.

This additional information provided for new songs is specifically useful when jumping to specific timestamps in a song. In that case, a 'PIDI' message with index=0 should be sent.

New songs should be started as soon as the entire message was successfully received. Otherwise the commands should be stored by the server in a second commands buffer that is only applied after the first commands buffer was applied fully.

The Commands are given in the same format as in PIDI files. The amount of provided commands can be inferred through the size of the entire message.

Only the client can send this message. The server has to respond with a `SUCC` message.

### 'REQP'

The reqp message is used to request the next pidi-chunks and is only sent by the server. The message should be exactly 4 bytes, that should be interpreted as an unsigned integer, giving the next chunk index that the server is expecting.

The Client has to respond with a 'PIDI' message. If the song is over, the commands count in the 'PIDI' message should be 0.

### 'STOP'

The stop message is only sent by the client. It tells the server to stop the current song. The message bytes should be empty or ignored.

The server has to respond with a 'SUCC' message.

### 'CONT'

The cont message is only sent by the client. It tells the server to continue the current song. The message bytes should be empty or ignored.

The server has to respond with a 'SUCC' message.

### 'LOUD'

The 'loud' message is only sent by the client. It tells the server to adjust the volume by a given value. Adjusting volume is done on the server side by de-/increasing the velocity levels of each command for the song. The volume levels should stay as given until the next 'loud' message.

The message should be exactly 4 bytes, that should be interpreted as a 32bit floating point number, giving the factor by which to multiply all velocity values.

The server has to respond with a 'SUCC' message.

### 'SPED'

The 'sped' message is only sent by the client. It tells the server the speed factor with which to continue playing the song. The message should consist of 4 bytes, that should be interpreted as a floating point number, giving the speed factor. The speed levels should stay as given until the next 'sped' message.

The server has to respond with a 'SUCC' message.
