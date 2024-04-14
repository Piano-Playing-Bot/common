This document collects the specifications of all protocols and formats that were created for this project.

# PIDI Format

This section specifies the format of PIDI-files. PIDI serves as a memory-efficient and simple-to-use format for storing all data required to play a song automatically on a piano.

## Specification

PIDI stands for `PIano Digital Interface`.

A PIDI file follows the following format:

```
<Magic Bytes: 4 bytes> <Commands Count: 4 bytes> <Commands>
```

Unless otherwise specified, small-endian encoding is used.

- **Magic Bytes:**
The format's Magic bytes are: `PIDI`. It is always written in big-endian format.

- **Commands Count:**
The amount of commands is given as an unsigned 32-bit number. It should follow exactly as many commands as given here.

- **Commands:**
All commands are written here one after another. The amount of commands is given in `Commands Count`. A single command's format is given below.

### Command Format

Every command is encoded as a 32-bit number. The following shows the bit field that makes up these 32 bits:

```c
delta time : 12 bits
velocity   : 4  bits
length     : 8  bits
octave     : 4  bits
key        : 4  bits
```

- **delta time:**
The delta time specifies the amount of milliseconds since the last command that the command should be applied at. This parameter implies that the commands are ordered by the time at which they should be played.

- **velocity:**
The velocity specifies the strength with which to play the note. Strength of playing a note is directly correlated with the volume of the played note.

- **len:**
The length specifies the amount of centiseconds for which the command should be active. A centisecond is 10 milliseconds.

- **octave:**
The octave gives the octave offset of the musical note that this chunk refers to. The key and octave together specify a specific key on a piano.
The octave should be interpreted as a signed 4-bit integer using two's complement. 0 refers to the middle octave on a piano. Negative numbers refer to octaves below the middle one, while positive numbers refer to higher octaves.

- **key:**
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

This section outlines the protocol used for the communication between the UI and the microcontroller (MC), that plays the music. The UI and MC are also referred to as nodes in this specifcation.

## Specification

Any message between the server and client has the following format:

```
<Magic Bytes: 3 bytes> <Message-Type: 1 byte> [<Payload: n bytes>]
```

Unless otherwise specified, small-endian encoding is used.

- **Magic Bytes:**
The protocol's Magic bytes are: `SPP`. It is always written in big-endian format. It stands for '**S**elf-**P**laying-**P**iano'.

- **Message-Type:**
The different types of messages that exist, are listed below under `Payload`. Each message type has its own 1 character signature, by which it is defined.
Should a node not recognize the provided message type, it should ignore all following bytes until it reads the Magic Bytes again. Version upgrades should easily stay backward-compatible that way.
The protocol uses the convention to use ASCII-encoded characters for the Message Type's identifier. Uppercase characters are used for messages sent by the UI and lowercase characters for messages sent by the MC.

- **Payload:**
How to interpret the payload depends on the `Message-Type`. The payload's format for each specified message type is given below. The title of each section provides the message's human-readable name and its 1-byte identifier.

### Ping: 'P'

The Ping message type exists to check whether the MC is available. It can also be used by the UI to initially identify the port, on which the MC is connected.

The initial message sent from the UI to the MC must be a Ping message.

There is no payload in this message.

### Pong: 'p'

This message should only be sent by the MC as a reply to the Ping message.

The payload should be an unsigned 16-bit number representing the maximum amount of `PidiCmd`s that may be sent in a Music or New-Music message.

### Success: 's'

The Successs message is only sent by the MC as a response to the UI. It indicates successfully receiving and executing the UI's last message.

The UI should wait until receiving a Success message before sending the next message. When no Success message is received by the UI before the timeout is reached, the same message must be sent again.

There is no payload in this message.

### Continue: 'C'

The Continue message is sent by the UI to pause or continue playing the song.

The payload should consist of a single byte. When the byte is 0, the song should be paused and continued otherwise.

The MC must respond with a Success message.

### Volume: 'V'

The Volume message is sent by the UI to set the factor by which the MC multiplies each note's velocity and thus its loudness.

The payload should consist of a single 32-bit IEEE-754 floating point number.

The MC must respond with a Success message.

### Speed: 'S'

The Speed message is sent by the UI to set the factor by which the MC's timer is multiplied.

The payload should consist of a single 32-bit IEEE-754 floating point number.

The MC must respond with a Success message.

### New-Music: 'N'

The New-Music message is meant for initiating the start of a song. It can be sent at any time by the UI.

Subsequent chunks of the song should be sent via the Music message instead, however.

The payload should be encoded as follows:

```
<PlayedKeys-Count: 1 byte> [<PlayedKey: 3 bytes>]+  <Commands-Count: 2 bytes> [<Command: 4 bytes>]+
```

There should be exactly as many PlayedKeys as provided in PlayedKeys-Count and as many Commands as provided in the Commands-Count.

Each PlayedKey is encoded as follows:

```
len:       8 bits
octave:    4 bits (signed)
key:       4 bits
velocity:  4 bits
<padding>: 4 bits
```

where all its parameters have the same meaning as in the [PIDI Format](#pidi-format).

New songs should be started as soon as the entire message was successfully received.

The MC has to respond with a Success message.

### Music: 'M'

The Music message exists for sending a list of commands for playing a Song to the MC. The Commands are given in the same format as in [PIDI](#pidi-format) files. Instead of sending the potentially very big PIDI file all at once, it is sent in chunks, thus effectively implementing a method of streaming the music.

A Music message may only be sent as a response to a Request message.

The payload should be encoded as follows:

```
<Commands-Count: 2 bytes> [<Command: 4 bytes>]+
```

There should be exactly as many Commands as provided in the Commands-Count.

Upon receiving the message, the MC must respond with a Success message.

### Request: 'r'

The Request message is used to tell the UI, that the MC is ready to receive the next chunk of the song.

There is no payload in this message.

The UI has to respond with a Music message. If the song is over, the Commands-Count in the Music message should be set to 0.
