#include "libtags.h"
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

/*
tags tag1 tag2 ... tagN -t file1.a file2.b ... fileN.z
tags tag1 tag2 ... tagN -u file1.a file2.b ... fileN.z
tags tag1 tag2 ... tagN
*/

int main(int argc, char** args){

  opterr = 0;

  int c;
  int tag_count = -1;
  int files_first_index = -1;
  int command = -1;

  while((c = getopt(argc, args, "tu")) != -1){
    switch(c){
      case 't':
      case 'u':
        command = c;
        if(tag_count == -1){
          tag_count = optind - 2;
          files_first_index = optind;
        }
        else{
          fprintf(stderr, "%s: only one 't' or 'u' option is allowed\n", args[0]);
          return 1;
        }
        break;
      case '?':
          fprintf(stderr, "%s: invalid option\n", args[0]);
          return 1;
        break;
    }
  }

  if(files_first_index == -1){
    for(int file_i = 2; file_i < argc; file_i++){
      printf("Tags for file %s:\n", args[file_i]);
      tags_list_file(args[file_i]);
    }

  }
  else{
    char** tags = malloc(tag_count*sizeof(char*));
    size_t t = 0;
    for(int tag_i = 2; tag_i < tag_count+2; tag_i++){
      //printf("args[%d] = %s\n", tag_i, args[tag_i]);
      tags[t] = args[tag_i];
      t++;
    }

    for(int file_i = files_first_index; file_i < argc; file_i++){
      if(command == 'u'){
        if(tags_untag_file(args[file_i], tag_count, tags)){
          printf("Untag file %s with tags", args[file_i]);
        }
        else{
          printf("Did not untagged file %s with tags", args[file_i]);
        }
      }
      else if(command == 't'){
        if(tags_tag_file(args[file_i], tag_count, tags)){
          printf("Tag file %s with tags", args[file_i]);
        }
        else{
          printf("Did not tagged file %s with tags", args[file_i]);
        }
      }

      for(t = 0; t < tag_count; t++){
        printf(" %s", tags[t]);
      }
      printf("\n");
    }


    free(tags);
  }

  return 0;
/*
  bool done;
  if(argc < 3){
    done = false;
    return done ? 0 : 1;
  }

  char* command = args[1];
  char* file_path = args[2];
  
  if(argc >= 4){
    size_t tagc = argc - 3;
    char** tags = args + 3;
    if(strcmp(command, "tag") == 0){
      done = tags_tag_file(file_path, tagc, tags); 
    }
    else if(strcmp(command, "untag") == 0){
      done = tags_untag_file(file_path, tagc, tags); 
    }
    else{
      done = false;
    }
  }
  else{
    if(strcmp(command, "list") == 0){
      done = tags_list_file(file_path); 
    }
    else{
      done = false;
    }
  }

  return done ? 0 : 1;
  */
}
