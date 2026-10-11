#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"enum.h"
#include"tagreader.h"


//help function
void help()
{
    printf("-t\tModifies a title tag\n-T\tModifies a track tag\n-a\tModifies an artist tag\n-A\tModifies an Album tag\n-m\tModifies a composer tag\n-y\tModifies a year tag\n-c\tModifies a comment tag\n-g\tModifies a genre tag\n-h\tDisplays this help info\n-v\tPrints version info\n");
}


//view function
void view(TagData *tagdata)
{
    if(tagdata->title_size)
    {
        printf("Title:\t\t%s\n",tagdata->title);
    }
    if(tagdata->artist_size)
    {
        printf("Artist:\t\t%s\n",tagdata->artist);
    }
    if(tagdata->album_size)
    {
        printf("Album:\t\t%s\n",tagdata->album);
    }
    if(tagdata->year_size)
    {
        printf("Year:\t\t%s\n",tagdata->year);
    }
    if(tagdata->genre_size)
    {
        printf("Genre:\t\t%s\n",tagdata->genre);
    }
    if(tagdata->composer_size)
    {
        printf("Composer:\t\t%s\n",tagdata->composer);
    }
    if(tagdata->comment_size)
    {
        printf("Comment:\t%s\n",tagdata->comment);
    }
} 


//edit function
void edit(TagData *tagdata,char*argv[],char* target_frame,int old_size)
{
    tagdata->OG_fname = argv[4];

    fseek(tagdata->fptr_OGmp3,0,SEEK_SET);
    fseek(tagdata->fptr_DUmp3,0,SEEK_SET);
    unsigned char header[10] = {0}; //10 bytes to store header files
    unsigned char frame_id[5] = {0};      
    unsigned char size[4];            
    unsigned char flags[2];
    int frame_size;
    unsigned char *data;

    
    fread(header,10,1,tagdata->fptr_OGmp3);   // read and write the header files to duplicate
    fwrite(header,10,1,tagdata->fptr_DUmp3);
    while(1)
    {
        fread(frame_id, 1, 4, tagdata->fptr_OGmp3);   //reading first 4 bytes 
        frame_id[4] = '\0';  //for later use in strcmp

        if(frame_id[0] == 0)  //checking if we reached the end 
        { 
            break; 
        }

        fread(size,1,4,tagdata->fptr_OGmp3);  //reading 4 bytes of size from original file
        fread(flags,1,2,tagdata->fptr_OGmp3);  //reading 2 bytes of flags from original file

        frame_size = (size[0] << 24) | (size[1] << 16) | (size[2] << 8) | size[3];  //stores it into integer in big endian format

        data = malloc(frame_size);
        fread(data,1,frame_size,tagdata->fptr_OGmp3);

        if(strcmp(frame_id,target_frame) != 0)  //if this is not the target frame copy everything to the duplicate file
        {
            fwrite(frame_id,1,4,tagdata->fptr_DUmp3);
            fwrite(size,1,4,tagdata->fptr_DUmp3);
            fwrite(flags,1,2,tagdata->fptr_DUmp3);
            fwrite(data,1,frame_size,tagdata->fptr_DUmp3);
        }
        else
        {
            printf("Found %s\n",frame_id);
            break;
        }
        free(data);
    }

    unsigned int new_size = 1 + strlen(argv[3]); //updated frametag size
    unsigned char new_size_bytes[4]; 

    new_size_bytes[0] = (new_size >> 24) & 0xFF;  //
    new_size_bytes[1] = (new_size >> 16) & 0xFF;  //  converting size to big endian
    new_size_bytes[2] = (new_size >> 8) & 0xFF;   //
    new_size_bytes[3] = new_size & 0xFF;          //

    fwrite(frame_id,1,4,tagdata->fptr_DUmp3);  //write frame id to duplicate
    fwrite(new_size_bytes,1,4,tagdata->fptr_DUmp3);  //write size to duplicate
    fwrite(flags,1,2,tagdata->fptr_DUmp3);  //write flags to duplicate (these wer all read before in the while loop)

    unsigned char encodingbyte = 0x00;  //the encoing byte
    fwrite(&encodingbyte,1,1,tagdata->fptr_DUmp3);  //copy encoding byte to duplicate file
    fwrite(argv[3],1,strlen(argv[3]),tagdata->fptr_DUmp3);
    // fseek(tagdata->fptr_OGmp3,frame_size,SEEK_CUR);

    unsigned char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), tagdata->fptr_OGmp3)) > 0) //fread returns number of bytes read so if it less that 1024 also no problem
    {
        fwrite(buffer, 1, bytes_read, tagdata->fptr_DUmp3); //reads the remaining datas and writes it to duplicate file
    }

    fseek(tagdata->fptr_DUmp3,6,SEEK_SET);
    int updated_total_tag_size = tagdata->size_of_all_frametags - old_size + (strlen(argv[3]) + 1);

    unsigned char new_tag_size_bytes[4];
    new_tag_size_bytes[0] = (updated_total_tag_size >> 21) & 0x7F ;
    new_tag_size_bytes[1] = (updated_total_tag_size >> 14) & 0x7F ; //converts the new size into syncsafe (msb = 0 for every byte)
    new_tag_size_bytes[2] = (updated_total_tag_size >> 7) & 0x7F ;
    new_tag_size_bytes[3] = updated_total_tag_size  & 0x7F ;

    fwrite(new_tag_size_bytes,1,4,tagdata->fptr_DUmp3); // writes the syncsafed size to the header

    fclose(tagdata->fptr_OGmp3);
    fclose(tagdata->fptr_DUmp3);
    remove(tagdata->OG_fname);
    rename("duplicate.mp3",tagdata->OG_fname);

}








//--------------------------------------------------------------------------------------------


//check operation function
OperationType check_operation(char* argv[])
{
    if(strcmp(argv[1],"-v") == 0) //check second command line argument is -v
    {
        return e_view;
    }

    if(strcmp(argv[1],"-e") == 0) //check second command line argument is -e
    {
        return e_edit;
    }
}

//opening files
void openfiles(TagData *tagdata,const char* filename)
{
    tagdata->fptr_OGmp3 = fopen(filename,"rb");
    if(tagdata->fptr_OGmp3 == NULL)
    {
        printf("failed to open mp3 file\n");
    }

    tagdata->fptr_DUmp3 = fopen("duplicate.mp3","wb");
    if(tagdata->fptr_DUmp3 == NULL)
    {
        printf("failed to open mp3 file\n");
    }
}

Status readtags(TagData *tagdata)
{
    fseek(tagdata->fptr_OGmp3,6,SEEK_SET); //moving the offset to the 6
    unsigned char size1[4];
    fread(size1,1,4,tagdata->fptr_OGmp3);  //reading 4 bytes (the size of all frame tags) and storing in a string
    tagdata->size_of_all_frametags = (size1[0] << 21) |
                                     (size1[1] << 14) | 
                                     (size1[2] << 7) | 
                                     (size1[3]); //decoding because it is in synchsafe format and storing it in the structure

    fseek(tagdata->fptr_OGmp3,10,SEEK_SET);
    int bytes_read = 0;

    while(bytes_read < tagdata->size_of_all_frametags)
    {
        fread(tagdata->arr,1,4,tagdata->fptr_OGmp3);
        tagdata->arr[4] = '\0';

        if(strcmp(tagdata->arr,"TIT2") == 0) //checking if first 4 bytes is "TIT2"
        {  
            unsigned char size[4];

            fread(size,4,1,tagdata->fptr_OGmp3); //read 4 bytes from mp3 file (size of tag) to the charecter array
            tagdata->title_size = ((size[0] << 24) | (size[1] << 16) | (size[2] << 8) | (size[3])); //changing it from charecter array to integer also because it is big endian

            fseek(tagdata->fptr_OGmp3,3,SEEK_CUR); //skipping the 2 flag bytes and 1 encoding bytes
            tagdata->title = malloc(tagdata->title_size);  //allocating the read size bytes to the title string 
            fread(tagdata->title,1,tagdata->title_size - 1,tagdata->fptr_OGmp3); //read the title and store to the structure

            tagdata->title[tagdata->title_size - 1] = '\0'; //put null for the last byte
            bytes_read += tagdata->title_size + 10; //increment bytes read
        }
        
  
        else if(strcmp(tagdata->arr,"TRCK") == 0)
        {
            unsigned char size[4];

            fread(size,4,1,tagdata->fptr_OGmp3);
            tagdata->track_size = ((size[0] << 24) | (size[1] << 16) | (size[2] << 8) | (size[3]));

            fseek(tagdata->fptr_OGmp3,3,SEEK_CUR);
            tagdata->track = malloc(tagdata->track_size);
                fread(tagdata->track,1,tagdata->track_size - 1,tagdata->fptr_OGmp3);

            tagdata->track[tagdata->track_size - 1] = '\0';
            bytes_read += tagdata->track_size + 10;
        }

        else if(strcmp(tagdata->arr,"TPE1") == 0)
        {
            unsigned char size[4];

            // tagdata->artist_size_offset = ftell(tagdata->fptr_OGmp3);

            fread(size,4,1,tagdata->fptr_OGmp3);
            tagdata->artist_size = ((size[0] << 24) | (size[1] << 16) | (size[2] << 8) | (size[3]));

            fseek(tagdata->fptr_OGmp3,3,SEEK_CUR);
            // tagdata->artist_offset = ftell(tagdata->fptr_OGmp3);
            tagdata->artist = malloc(tagdata->artist_size);
            fread(tagdata->artist,1,tagdata->artist_size - 1,tagdata->fptr_OGmp3);

            tagdata->artist[tagdata->artist_size - 1] = '\0';
            bytes_read += tagdata->artist_size + 10;
        }

        else if(strcmp(tagdata->arr,"TALB") == 0)
        {
            unsigned char size[4];

            // tagdata->album_size_offset = ftell(tagdata->fptr_OGmp3);

            fread(size,4,1,tagdata->fptr_OGmp3);
            tagdata->album_size = ((size[0] << 24) | (size[1] << 16) | (size[2] << 8) | (size[3]));

            fseek(tagdata->fptr_OGmp3,3,SEEK_CUR);
            // tagdata->album_offset = ftell(tagdata->fptr_OGmp3);
            tagdata->album = malloc(tagdata->album_size);
            fread(tagdata->album,1,tagdata->album_size - 1,tagdata->fptr_OGmp3);

            tagdata->album[tagdata->album_size - 1] = '\0';
            bytes_read += tagdata->album_size + 10;
        }

        else if(strcmp(tagdata->arr,"TYER") == 0)
        {
            unsigned char size[4];

                // tagdata->year_size_offset = ftell(tagdata->fptr_OGmp3);

            fread(size,4,1,tagdata->fptr_OGmp3);
            tagdata->year_size = ((size[0] << 24) | (size[1] << 16) | (size[2] << 8) | (size[3]));

            fseek(tagdata->fptr_OGmp3,3,SEEK_CUR);
            // tagdata->year_offset = ftell(tagdata->fptr_OGmp3);
            tagdata->year = malloc(tagdata->year_size);
            fread(tagdata->year,1,tagdata->year_size - 1,tagdata->fptr_OGmp3);

            tagdata->year[tagdata->year_size - 1] = '\0';
            bytes_read += tagdata->year_size + 10;
        }

        else if(strcmp(tagdata->arr,"TCON") == 0)
        {
            unsigned char size[4];

                // tagdata->genre_size_offset = ftell(tagdata->fptr_OGmp3);

            fread(size,4,1,tagdata->fptr_OGmp3);
            tagdata->genre_size = ((size[0] << 24) | (size[1] << 16) | (size[2] << 8) | (size[3]));

            fseek(tagdata->fptr_OGmp3,3,SEEK_CUR);
            // tagdata->genre_offset = ftell(tagdata->fptr_OGmp3);
            tagdata->genre = malloc(tagdata->genre_size);
            fread(tagdata->genre,1,tagdata->genre_size - 1,tagdata->fptr_OGmp3);

            tagdata->genre[tagdata->genre_size -1] = '\0';
            bytes_read += tagdata->genre_size + 10;
        }

        else if(strcmp(tagdata->arr,"TCOM") == 0)
        {
            unsigned char size[4];

            // tagdata->composer_size_offset = ftell(tagdata->fptr_OGmp3);

            fread(size,4,1,tagdata->fptr_OGmp3);
            tagdata->composer_size = ((size[0] << 24) | (size[1] << 16) | (size[2] << 8) | (size[3]));

            fseek(tagdata->fptr_OGmp3,3,SEEK_CUR);
            // tagdata->composer_offset = ftell(tagdata->fptr_OGmp3);
            tagdata->composer = malloc(tagdata->composer_size);
            fread(tagdata->composer,1,tagdata->composer_size - 1,tagdata->fptr_OGmp3);
                
            tagdata->composer[tagdata->composer_size - 1] = '\0';
            bytes_read += tagdata->composer_size + 10;
        }

        else if(strcmp(tagdata->arr,"COMM") == 0)
        {
            unsigned char size[4];

            fread(size,4,1,tagdata->fptr_OGmp3);
            tagdata->comment_size = ((size[0] << 24) | (size[1] << 16) | (size[2] << 8) | (size[3]));

            fseek(tagdata->fptr_OGmp3,3,SEEK_CUR);
            tagdata->comment = malloc(tagdata->comment_size);
            fread(tagdata->comment,1,tagdata->comment_size - 1,tagdata->fptr_OGmp3);

            tagdata->comment[tagdata->comment_size -1 ] = '\0';
            bytes_read += tagdata->comment_size + 10;
        }

        else
        {
            unsigned char size[4];
            fread(size,4,1,tagdata->fptr_OGmp3);
            int size1 = (size[0] << 24) | (size[1] << 16) | (size[2] << 8) | (size[3]);
            fseek(tagdata->fptr_OGmp3, 2 + size1,SEEK_CUR);
            bytes_read += size1 + 10;
        }
    }
}