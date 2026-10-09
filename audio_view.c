#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio_view.h"

void read_mp3_tag(const char *filename)
{
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL)
    {
        printf("Error: Cannot open file\n");
        return;
    }

    unsigned char header[10];

    /* Read ID3 header */
    if (fread(header, 1, 10, fp) != 10)
    {
        printf("Error: Cannot read ID3 header\n");
        fclose(fp);
        return;
    }

    /* Check ID3 signature */
    if (header[0] != 'I' ||
        header[1] != 'D' ||
        header[2] != '3')
    {
        printf("Error: Not an ID3 MP3 file\n");
        fclose(fp);
        return;
    }

    /* Move after ID3 header */
    fseek(fp, 10, SEEK_SET);

    while (1)
    {
        mp3tag frame;

        unsigned char size_buf[4];
        unsigned char flags[2];

        /* Read tag */
        if (fread(frame.tag, 1, 4, fp) != 4)
            break;

        frame.tag[4] = '\0';

        /* Check only required tags */
        if (strcmp(frame.tag, "TIT2") != 0 &&
            strcmp(frame.tag, "TPE1") != 0 &&
            strcmp(frame.tag, "TALB") != 0 &&
            strcmp(frame.tag, "TYER") != 0 &&
            strcmp(frame.tag, "TCON") != 0 &&
            strcmp(frame.tag, "COMM") != 0)
        {
            /*
             * Still need to read size and flags
             * to move to next frame.
             */
            if (fread(size_buf, 1, 4, fp) != 4)
                break;

            if (fread(flags, 1, 2, fp) != 2)
                break;

            frame.size = ((unsigned int)size_buf[0] << 21) |
                         ((unsigned int)size_buf[1] << 14) |
                         ((unsigned int)size_buf[2] << 7) |
                         size_buf[3];

            fseek(fp, frame.size, SEEK_CUR);

            continue;
        }

        /* Read size */
        if (fread(size_buf, 1, 4, fp) != 4)
            break;

        frame.size = ((unsigned int)size_buf[0] << 21) |
                     ((unsigned int)size_buf[1] << 14) |
                     ((unsigned int)size_buf[2] << 7) |
                     size_buf[3];

        /* Read flags but DON'T DISPLAY */
        if (fread(flags, 1, 2, fp) != 2)
            break;

        /* Allocate memory */
        frame.data = malloc(frame.size + 1);

        if (frame.data == NULL)
        {
            printf("Memory allocation failed\n");
            fclose(fp);
            return;
        }

        /* Read data */
        if (fread(frame.data, 1, frame.size, fp) != frame.size)
        {
            free(frame.data);
            break;
        }

        frame.data[frame.size] = '\0';

        /* Display only required information */
        printf("\nTag  : %s\n", frame.tag);
        printf("Size : %u bytes\n", frame.size);

        printf("Data : ");

        /*
         * Text frames have 1 byte encoding information
         * before actual text.
         */
        if (frame.size > 1)
        {
            printf("%s", &frame.data[1]);
        }

        printf("\n");

        free(frame.data);
    }

    fclose(fp);
}