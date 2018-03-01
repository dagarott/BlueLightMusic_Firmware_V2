#ifndef SONGPATTERNS_H__
#define SONGPATTERNS_H__

#include "stdlib.h"
//#include <stdio.h>
#include "stdlib.h"
//#include "app_error.h"
#include "app_util_platform.h"
#include "drv_Buzzer.h"

// define timer scaling value for each note
#define C 2441
#define D 2741
#define E 3048
#define F 3255
#define G 3654
#define A 4058
#define B 4562
#define C2 4882

#define NUM_SONGS 3
#define END_SONG 1000

#define num_notes_song1 14 // total number of notes in song to be played - modify for specific song
#define tempo_song1 400
#define num_notes_song2 26
#define tempo_song2 400
#define num_notes_song3 25
#define tempo_song3 400

#define SONG1 '8' //following led patterns enumeration
#define SONG2 '9' //following led patterns enumeration
#define SONG3 'A' //following led patterns enumeration


//NOTE:
//tO AVOID ERROR WITH LINKER, ARRAYs MUST BE DECLARED AS "STATIC CONST"
static const uint16_t song1[num_notes_song1] = { C, C, G, G, A, A, G, F, F, E, E, D, D,
    C }; // insert notes of song in array
static const uint8_t length_note_song1[num_notes_song1] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1,
    2 }; // relative length of each note
static const uint16_t song2[num_notes_song2] = { B, A, G, A, B, B, B, A, A, A, B, B, B, B, A, G, A, B, B, B, A, A, B, A,
    G, G };
static const uint8_t length_note_song2[num_notes_song2] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 1,
    1, 1, 2, 2 };
static const uint16_t song3[num_notes_song3] = { E, E, E, E, E, E, E, G, C, D, E, F, F, F, F, F, E, E, E, E, D, D, E, D,
    G };
static const uint8_t length_note_song3[num_notes_song3] = { 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 2, 2 };

typedef struct {
    uint8_t num_notes;
    // uint8_t song[50];
    const uint16_t* ptr_song;
    uint16_t tempo_song;
    const uint8_t* ptr_length_song;
} song_param_t;

extern song_param_t record[NUM_SONGS];
extern song_param_t (*ptr_songs);


void InitSongStruct(void);
//uint16_t PlayMusic(song_param_t* ptr_array_struct);
uint16_t PlayMusic(const uint16_t p_sound[], uint8_t size, const uint8_t p_length_note[]);

#endif // SONGPATTERNS_H
