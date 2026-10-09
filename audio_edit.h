#ifndef EDIT_H
#define EDIT_H

#include <stdio.h>

typedef struct
{
    char tag[5];
    unsigned int size;
    unsigned char flags[2];
} EditFrame;

void edit_mp3_tag(const char *filename, const char *tag, const char *new_data);

#endif