#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio_edit.h"

/* Read 4-byte Big Endian size */
unsigned int read_big_endian(unsigned char size_buf[4])
{
    return ((unsigned int)size_buf[0] << 24) |
           ((unsigned int)size_buf[1] << 16) |
           ((unsigned int)size_buf[2] << 8)  |
           (unsigned int)size_buf[3];
}

/* Convert integer to 4-byte Big Endian */
void write_big_endian(unsigned char size_buf[4], unsigned int size)
{
    size_buf[0] = (size >> 24) & 0xFF;
    size_buf[1] = (size >> 16) & 0xFF;
    size_buf[2] = (size >> 8) & 0xFF;
    size_buf[3] = size & 0xFF;
}

/* Read ID3 header size - Sync Safe */
unsigned int read_tag_size(unsigned char header[10])
{
    return ((unsigned int)header[6] << 21) |
           ((unsigned int)header[7] << 14) |
           ((unsigned int)header[8] << 7) |
           (unsigned int)header[9];
}

/* Write ID3 header size - Sync Safe */
void write_tag_size(unsigned char header[10], unsigned int size)
{
    header[6] = (size >> 21) & 0x7F;
    header[7] = (size >> 14) & 0x7F;
    header[8] = (size >> 7) & 0x7F;
    header[9] = size & 0x7F;
}


void edit_mp3_tag(const char *filename, const char *tag, const char *new_data)
{
    unsigned int old_tag_size;
    unsigned int new_tag_size;

    int tag_found = 0;

    /* Open original & temporary file */
    FILE *src = fopen(filename, "rb");
    FILE *temp = fopen("temp.mp3", "wb");

    if (src == NULL || temp == NULL)
    {
        printf("Error: file mot found \n");
        return;
    }

    //STEP 1: Read ID3 header
    unsigned char header[10];
    if (fread(header, 1, 10, src) != 10)
    {
        printf("Error: Cannot read ID3 header\n");
        fclose(src);
        fclose(temp);
        remove("temp.mp3");
        return;
    }
    /* Check ID3 */
    if (header[0] != 'I' || header[1] != 'D' || header[2] != '3')
    {
        printf("Error: ID3 tag not found\n");
        fclose(src);
        fclose(temp);
        remove("temp.mp3");
        return;
    }
    /* Get original ID3 tag size */
    old_tag_size = read_tag_size(header);

    /* Initially same as old size */
    new_tag_size = old_tag_size;

    /* Write original header */
    fwrite(header, 1, 10, temp);

    //STEP 2: Read frames

    unsigned int bytes_read = 0;

    while (bytes_read < old_tag_size)
    {
        EditFrame frame;
        unsigned char size_buf[4];

        /* Read frame tag & check empty Frame*/
        frame.tag[4] = '\0';
        if (fread(frame.tag, 1, 4, src) != 4 || frame.tag[0] == '\0')
            break;

        /* Read frame size */
        if (fread(size_buf, 1, 4, src) != 4)
            break;

        frame.size = read_big_endian(size_buf);

        /* Read flags */
        if (fread(frame.flags, 1, 2, src) != 2)
            break;

        /* Frame header = 10 bytes */
        bytes_read += 10;

    //STEP 3: Matching tag

        if (!tag_found && strcmp(frame.tag, tag) == 0)
        {
            unsigned int new_size;
            unsigned char new_size_buf[4];

            printf("Tag found: %s\n", frame.tag);

            /* Text frame: 1 byte encoding + text */
            new_size = strlen(new_data) + 1;

            /* Convert size to Big Endian */
            write_big_endian(new_size_buf, new_size);

            /* Write frame tag */
            fwrite(frame.tag, 1, 4, temp);

            /* Write new frame size */
            fwrite(new_size_buf, 1, 4, temp);

            /* Write original flags */
            fwrite(frame.flags, 1, 2, temp);

            /* Encoding byte */
            unsigned char encoding = 0;

            fwrite(&encoding, 1, 1, temp);

            /* Write new text */
            fwrite(new_data, 1, strlen(new_data), temp);

            /* Skip old frame data */
            if (fseek(src, frame.size, SEEK_CUR) != 0)
            {
                printf("Error: Cannot skip old frame\n");

                fclose(src);
                fclose(temp);
                remove("temp.mp3");
                return;
            }
            /* Update ID3 tag size */
            new_tag_size = old_tag_size - frame.size + new_size;
            bytes_read += frame.size;
            tag_found = 1;
            printf("New value: %s\n", new_data);
        }

        //STEP 4: Non-matching frame

        else
        {
            unsigned char buffer[4096];
            unsigned int remaining = frame.size;

            /* Write frame header */
            fwrite(frame.tag, 1, 4, temp);
            fwrite(size_buf, 1, 4, temp);
            fwrite(frame.flags, 1, 2, temp);

            /* Copy frame data in chunks */
            while (remaining > 0)
            {
                unsigned int chunk;
                if (remaining > sizeof(buffer))
                    chunk = sizeof(buffer);
                else
                    chunk = remaining;

                if (fread(buffer, 1, chunk, src) != chunk)
                {
                    printf("Error: Cannot read frame data\n");
                    fclose(src);
                    fclose(temp);
                    remove("temp.mp3");
                    return;
                }
                fwrite(buffer, 1, chunk, temp);
                remaining -= chunk;
            }
            bytes_read += frame.size;
        }
    }

//STEP 5: Tag not found

    if (!tag_found)
    {
        printf("Tag %s not found\n", tag);
        fclose(src);
        fclose(temp);
        remove("temp.mp3");
        return;
    }

//STEP 6: Copy remaining MP3 data

    unsigned char buffer[4096];
    size_t count;

    while ((count = fread(buffer, 1, sizeof(buffer), src)) > 0)
    {
        fwrite(buffer, 1, count, temp);
    }

//STEP 7: Update ID3 header size

    write_tag_size(header, new_tag_size);
    /* Go to beginning */
    fseek(temp, 0, SEEK_SET);
    /* Rewrite header */
    fwrite(header, 1, 10, temp);

//STEP 8: Close files
    fclose(src);
    fclose(temp);

//STEP 9: Replace original
    if (remove(filename) != 0)
    {
        perror("remove");
        remove("temp.mp3");
        return;
    }
    if (rename("temp.mp3", filename) != 0)
    {
        perror("rename");
        return;
    }
    printf("MP3 tag updated successfully\n");
}