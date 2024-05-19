#pragma once

#define ANM_FILE_TEXT 0
#define ANM_FILE_ASCII 1
#define ANM_FILE_ASCIIS 2
#define ANM_FILE_CAPTURE 3
#define ANM_FILE_PLAYER 5
#define ANM_FILE_ENEMY 8
#define ANM_FILE_ENEMY2 9
#define ANM_FILE_EFFECTS 11
#define ANM_FILE_TITLE01 21
#define ANM_FILE_TITLE01S 22
#define ANM_FILE_TITLE04S 23
#define ANM_FILE_TITLE02 24
#define ANM_FILE_TITLE03 25
#define ANM_FILE_TITLE04 26
#define ANM_FILE_REPLAY 36

#define ANM_OFFSET_ASCII 0x000
#define ANM_OFFSET_ASCIIS 0x077
#define ANM_OFFSET_ENEMY 0x100
#define ANM_OFFSET_TITLE01 0x100
#define ANM_OFFSET_TITLE02 0x11b
#define ANM_OFFSET_TITLE03 0x11f
#define ANM_OFFSET_TITLE04 0x122
#define ANM_OFFSET_REPLAY 0x160
#define ANM_OFFSET_TITLE01S 0x17a
#define ANM_OFFSET_TITLE04S 0x195
#define ANM_OFFSET_EFFECTS 0x2b3
#define ANM_OFFSET_PLAYER 0x400
#define ANM_OFFSET_TEXT 0x700
#define ANM_OFFSET_CAPTURE 0x718

#define ANM_SPRITE_TITLE01_START ANM_OFFSET_TITLE01
#define ANM_SPRITE_TITLE01_END 0x11a

#define ANM_SCRIPT_ASCII_START ANM_OFFSET_ASCII
#define ANM_SCRIPT_ASCII_END 0x009

#define ANM_SCRIPT_ENEMY_START ANM_OFFSET_ENEMY
#define ANM_SCRIPT_ENEMY_END 0x1a5

#define ANM_SCRIPT_TITLE01_START ANM_OFFSET_TITLE01
#define ANM_SCRIPT_TITLE01_END 0x11a

#define ANM_SCRIPT_TITLE02_START ANM_OFFSET_TITLE02
#define ANM_SCRIPT_TITLE02_END 0x11e

#define ANM_SCRIPT_TITLE03_START ANM_OFFSET_TITLE03
#define ANM_SCRIPT_TITLE03_END 0x121

#define ANM_SCRIPT_TITLE04_START ANM_OFFSET_TITLE04
#define ANM_SCRIPT_TITLE04_END 0x14f

#define ANM_SCRIPT_REPLAY_START ANM_OFFSET_REPLAY
#define ANM_SCRIPT_REPLAY_END 0x179

#define ANM_SCRIPT_EFFECTS_START ANM_OFFSET_EFFECTS
#define ANM_SCRIPT_EFFECTS_END 0x2b3

#define ANM_SCRIPT_PLAYER_START ANM_OFFSET_PLAYER
#define ANM_SCRIPT_PLAYER_IDLE (ANM_SCRIPT_PLAYER_START + 0)
#define ANM_SCRIPT_PLAYER_ORB_LEFT (ANM_SCRIPT_PLAYER_START + 128)
#define ANM_SCRIPT_PLAYER_ORB_RIGHT (ANM_SCRIPT_PLAYER_START + 129)
#define ANM_SCRIPT_PLAYER_END 0x48c

#define ANM_SCRIPT_TEXT_START ANM_OFFSET_TEXT
#define ANM_SCRIPT_TEXT_END 0x717

#define ANM_SCRIPT_CAPTURE_START ANM_OFFSET_CAPTURE
#define ANM_SCRIPT_CAPTURE_END 0x718
