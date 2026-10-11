#ifndef TAG_H
#define TAG_H

typedef struct tag_info
{
    FILE* fptr_OGmp3;
    FILE* fptr_DUmp3;
    char* OG_fname;
    char* DU_fname;
    char* title;
    int   title_size;
    char* track;
    int   track_size;
    char* artist;
    int   artist_size;
    char* album;
    int   album_size;
    char* year;
    int   year_size;
    char* composer;
    int   composer_size;
    char* genre;
    int   genre_size;
    char arr[5];
    int size_of_all_frametags;
    char* new_data;
    char* comment;
    int comment_size;
} TagData;




//_________________________________FUNCTION PROTOTYPES_______________________________

//checking operation
OperationType check_operation(char* argv[]);

//help function
void help();

//edit
void edit(TagData *tagdata,char* argv[],char* target_frame,int old_size);

//view
void view(TagData *tagdata);

//open files
void openfiles(TagData *tagdata,const char* filename);

//read tags
Status readtags(TagData *tagdata);

#endif 