#include "libtags.h"
#include <string.h>

int main(int argc, char** args){
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
}
