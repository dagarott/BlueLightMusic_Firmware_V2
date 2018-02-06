/* Copyright (c) 2016 Takafumi Naka. All Rights Reserved.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

#include "SongPatterns.h"

/**
 *  \brief Brief description
 *
 *  \return Return description
 *
 *  \details More details
 */
void InitSongStruct(void)
{
    record[0].num_notes = 14;
    record[0].ptr_song = song1;
    record[0].tempo_song = 400;
    record[0].ptr_length_song = length_song1;

    record[1].num_notes = 26;
    record[1].ptr_song = song2;
    record[1].tempo_song = 400;
    record[1].ptr_length_song = length_song2;

    record[2].num_notes = 25;
    record[2].ptr_song = song3;
    record[2].tempo_song = 400;
    record[2].ptr_length_song = length_song3;
}

/**
 *  \brief Brief description
 *
 *  \return Return description
 *
 *  \details More details
 */
uint16_t PlayMusic(song_param_t* ptr_array_struct)
{

    static uint8_t j = 0;
    uint8_t tmp;

    if(j < ptr_array_struct->num_notes) {

        tmp = *(ptr_array_struct->ptr_length_song++);
        drv_speaker_tone_start(*(ptr_array_struct->ptr_song++), (200 * tmp), 50);
        j++;
        return (ptr_array_struct->tempo_song);

    } else {
        j = 0;
        return (END_SONG);
    }
}