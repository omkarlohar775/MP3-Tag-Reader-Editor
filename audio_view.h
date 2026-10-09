#ifndef AUDIO_H
#define AUDIO_H

#include <stdio.h>

typedef struct
{
    char tag[4];
    unsigned int size;
    unsigned char flags[2];
    unsigned char *data;
} mp3tag;

void read_mp3_tag(const char *filename);

#endif