// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments

struct __attribute__((__packed__)) DirectoryEntry {
  char DIR_Name[11];
  uint8_t DIR_Attr;
  uint8_t Unused1[8];
  uint16_t DIR_FirstCluserHigh;
  uint8_t Unused2[4];
  uint16_t DIR_FirstCluserLow;
  uint32_t DIR_FileSize;
};

// From fat32 pdf
struct FAT32 {
  char BS_OEMNAME[8];
  int16_t BPB_BytsPerSec;
  int8_t BPB_SecPerClus;
  int16_t BPB_RsvdSecCnt;
  int8_t BPB_NumFATS;
  int16_t BPB_RootEntCnt;
  char BS_VolLab[11];
  int32_t BPB_FATSz32;
  int32_t BPB_RootClus;

  int32_t RootDirSectors;
  int32_t FirstDataSector;
  int32_t FirstSectorofCluser;
};

FILE* openFile(char *fileName, struct FAT32 *img);

int main()
{
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
  char * currentFile = NULL;
  int hasFileClosed = 0;
  //Creating file pointer
  FILE *IMG = NULL;

  struct DirectoryEntry dir[16];

  // Create a single instance of image struct
  struct FAT32 *fat = malloc(sizeof(struct FAT32));

  while( 1 )
  {
    // Print out the msh prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Checking first token to carry out
    // which required functionality
    if ( token[0] == NULL){
      
    }

    else if ( strcmp(token[0], "open") == 0){
      if(currentFile == NULL && IMG == NULL){
        IMG = openFile(token[1], fat);
        // we have an open file, set it as our current file
        if(IMG != NULL){
          currentFile = (char *)malloc(sizeof(token[1]));
          strcpy(currentFile, token[1]);
        }
      }else{
        if(strcmp(currentFile, token[1]) == 0){
          printf("Error: File system image is already open.\n");
        }
      }
    }
    else if ( strcmp(token[0], "close") == 0){
      // 0 is success
      // -1 is failed
      // If the file pointer is not null
      if(IMG != NULL){
        int res = fclose(IMG);
        printf("%d\n", res);
        if(res == 0){
          // Reset the current filename && file pointer to null
          currentFile = NULL;
          IMG = NULL;
          // Set a flag for checking on other operations
        }
      }else{
        printf("%s\n", "Error: File system not open.");
      }
    }
    else if (strcmp(token[0], "info") == 0){
      printf("%d %x\n", fat->BPB_BytsPerSec, fat->BPB_BytsPerSec);
      printf("%d %x\n", fat->BPB_SecPerClus, fat->BPB_SecPerClus);
      printf("%d %x\n", fat->BPB_RsvdSecCnt, fat->BPB_RsvdSecCnt);
      printf("%d %x\n", fat->BPB_NumFATS, fat->BPB_NumFATS);
      printf("%d %x\n", fat->BPB_FATSz32, fat->BPB_FATSz32);

	  }
    else if (strcmp(token[0], "stat") == 0){
      
    }
    else if (strcmp(token[0], "get") == 0){
     
    }
    else if (strcmp(token[0], "cd") == 0) {
     
    }
    else if (strcmp(token[0], "ls") == 0) {
     
    }
    else if (strcmp(token[0], "read") == 0) {

    }
    else if (strcmp(token[0], "volume") == 0) {

    }
    
    // DEBUG - delete before turning in
    else if (strcmp(token[0], "exit") == 0) {
      return 0;
    }

    free( working_root );
  }
  return 0;
}


FILE* openFile(char *fileName, struct FAT32 *img){
  FILE *file;
  if(!(file=fopen(fileName, "r"))){
    printf("Error: File system image not found.\n");
    return 0;
  }

  // We have the file reading
  // Set all values of BPB Structure
  // ...this could be a lot cleaner.
  // 1. BPB_BytsPerSec, at offset 11
  fseek(file, 11, SEEK_SET);
  // read the value into the pointer to the struct
  fread(&img->BPB_BytsPerSec, 2, 1, file);
  // DEBUG - delete before turning in
  // printf("%d %x\n", img->BPB_BytsPerSec, img->BPB_BytsPerSec);
  // 2. BPB_SecPerClus, at offset 13
  fseek(file, 13, SEEK_SET);
  // ...repeat
  fread(&img->BPB_SecPerClus, 1, 1, file);
  // DEBUG - delete before turning in
  // printf("%d %x\n", img->BPB_SecPerClus, img->BPB_SecPerClus);
  // 3. BPB_RsvdSecCnt, at offset 14
  fseek(file, 14, SEEK_SET);
  // ...repeat
  fread(&img->BPB_RsvdSecCnt, 2, 1, file);
  // DEBUG - delete before turning in
  // printf("%d %x\n", img->BPB_RsvdSecCnt, img->BPB_RsvdSecCnt);
  // 4. BPB_NumFATs, at offset 16
  fseek(file, 16, SEEK_SET);
  // ...repeat
  fread(&img->BPB_NumFATS, 1, 1, file);
  // DEBUG - delete before turning in
  // printf("%d %x\n", img->BPB_NumFATS, img->BPB_NumFATS);
  // 5. BPB_FATSz32, at offset 36
  fseek(file, 36, SEEK_SET);
  // ...repeat
  fread(&img->BPB_FATSz32, 4, 1, file);
  // DEBUG - delete before turning in
  // printf("%d %x\n", img->BPB_FATSz32, img->BPB_FATSz32);

  // ...Others we didn't cover in class?
  // BS_OEMNAME
  fseek(file, 3, SEEK_SET);
  fread(&img->BS_OEMNAME, 8, 1, file);

  // BS_VolLab
  fseek(file, 71, SEEK_SET);
  fread(&img->BS_VolLab, 11, 1, file);

  // BPB_RootEntCnt
  fseek(file, 17, SEEK_SET);
  fread(&img->BPB_RootEntCnt, 2, 1, file);

  // BPB_RootClus
  fseek(file, 44, SEEK_SET);
  fread(&img->BPB_RootEntCnt, 4, 1, file);

  // Defaults from pdf
  img->RootDirSectors = 0;
  img->FirstDataSector = 0;
  img->FirstSectorofCluser = 0;

  return file;
}
