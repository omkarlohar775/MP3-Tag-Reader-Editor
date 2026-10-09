/*
Title       : MP3 Tag Reader and Editor
Name        : Omkar Lohar
Date        : 06/09/2026
ID          : 26010_037
Description : A C program to read and edit ID3 metadata tags of an MP3 file.
              The program supports reading information such as Title, Artist,
              Album, Year, Genre, and Comment, and allows the user to modify
              the supported MP3 tag fields.
*/

#include <stdio.h>
#include <string.h>
#include "audio_view.h"
#include "audio_edit.h"

int main(int argc, char *argv[])
{
    /* View operation */
    if (argc == 3 && strcmp(argv[1], "-v") == 0)
    {
        if(argc != 3)
        {
            printf("Enter the 4 arguments for viewing like this\n");
            printf("Usage : %s -v audio.mp3\n", argv[0]);
            return 0;
        }
        read_mp3_tag(argv[2]);
    }

    /* Edit operation */
    else if (strcmp(argv[1], "-e") == 0)
    {
        if(argc != 5)
        {
            printf("Enter the 5 arguments for editing like this\n");
            printf("Usage : %s -e TAG \"NEW DATA\" audio.mp3\n",argv[0]);
            return 0;
        }
        edit_mp3_tag(argv[4], argv[2], argv[3]);
    }

    else
    {
        printf("Invalid option\n");

        printf("\nView:\n");
        printf("Usage 3 arguments like: %s -v audio.mp3\n", argv[0]);

        printf("\nEdit:\n");
        printf("Usage 5 arguments like: %s -e TAG \"NEW DATA\" audio.mp3\n", argv[0]);
    }

    return 0;
}