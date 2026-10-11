/*
NAME : NADEEM BASHEER USMAN
PROJECT : MP3 TAG READER
Description : MP3 TAGREADER
MP3 tag reader to view and edit audio metadata such as title, artist, album, and year.
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"enum.h"
#include"tagreader.h"

int main(int argc , char *argv[])
{
    if(argc < 2) //validating command line arguments
    {
        printf("Error : Invalid arguments!\nExpected arguments :\nFor help :\t./a.out --help\nFor viewing :\t./a.out -v  <file_name.mp3>\nFor editing :\t./a.out -e [option] updated_tag_content <file_name.mp3\n\nOptions for -e (edit mode):\n-t\tEdit title\n-a\tEdit artist\n-A\tEdit album\n-m\tEdit content\n-y\tEdit year\n-c\tEdit comments\n");
        return 0;
        
    }

    if(strcmp(argv[1],"--help") == 0) //validating if user enters --help
    {
        if(argc != 2)
        {
        printf("Error : Invalid arguments\nExpected arguments\nFor help : ./a.out --help\n");
        return 0;
        }
        else
        {
            help();
            return 0;
        }
    }

    else if(strstr(argv[1],"-v") != NULL) //validating if user enters -v
    {
        if(argc != 3)
        {
            printf("Error : Invalid arguments\nExpected arguments\nFor viewing : ./a.out -v  <file_name.mp3>\n");
            return 0;
        }
    }

    else if(strstr(argv[1],"-e") != NULL) //validating if user enters -e
    {
        if(argc != 5)
        {
            printf("Error : Invalid arguments\nExpected arguments\nFor editing : ./a.out -e [option] updated_tag_content <file_name.mp3\nOptions for -e (edit mode):\n-t\tEdit title\n-a\tEdit artist\n-A\tEdit album\n-m\tEdit content\n-y\tEdit year\n-c\tEdit comments\n");
            return 0;
        }
    }

    else
    {
        printf("Error : Invalid arguments\nExpected arguments\nFor help : ./a.out --help\nFor viewing : ./a.out -v  <file_name.mp3>\nFor editing : ./a.out -e [option] updated_tag_content <file_name.mp3\nOptions for -e (edit mode):\n-t      Edit title\n-a      Edit artist\n-A     Edit album\n-m      Edit content\n-y        Edit year\n-c       Edit comments\n");
        return 0;
    }

    TagData tagdata; //declaring the structure
    memset(&tagdata, 0, sizeof(TagData)); // setting everything to 0


    if (strstr(argv[1], "-v"))
    {
        openfiles(&tagdata, argv[2]);
    }
    else if (strstr(argv[1], "-e"))
    {
        openfiles(&tagdata, argv[4]);
    }


    if(readtags(&tagdata) == e_failure)
    {
        printf("Failed to read tags\n");
        return 0;
    }

    if(check_operation(argv) == e_view)
    {
       view(&tagdata);
    }

    if(check_operation(argv) == e_edit)
    {
       if(strcmp(argv[2],"-t") == 0)
       {
            edit(&tagdata,argv,"TIT2",tagdata.title_size);
       }
       else if(strcmp(argv[2],"-a") == 0)
       {
            edit(&tagdata,argv,"TPE1",tagdata.artist_size);
       }
       else if(strcmp(argv[2],"-A") == 0)
       {
            edit(&tagdata,argv,"TALB",tagdata.album_size);
       }
       else if(strcmp(argv[2],"-m") == 0)
       {
            edit(&tagdata,argv,"TCOM",tagdata.composer_size);
       }
       else if(strcmp(argv[2],"-y") == 0)
       {
            edit(&tagdata,argv,"TYER",tagdata.year_size);
       }
       else if(strcmp(argv[2],"-c") == 0)
       {
            edit(&tagdata,argv,"COMM",tagdata.comment_size);
       }
       else if(strcmp(argv[2],"-T") == 0)
       {
            edit(&tagdata,argv,"TRCK",tagdata.track_size);
       }
       else if(strcmp(argv[2],"-g") == 0)
       {
            edit(&tagdata,argv,"TCON",tagdata.genre_size);
       }
       else if(strcmp(argv[2],"-h") == 0)
       {
            help();
       }
    }
}
