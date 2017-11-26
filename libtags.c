#include "libtags.h"
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <openssl/md5.h>
#include <stdlib.h>

#define print_error(line, result, db) printf("Line %d, error %d: %s\n", line, result, sqlite3_errmsg(db))

bool perform_md5sum_file(FILE* file, uint8_t* md5){
  bool done = true;
  const size_t BUFFER_SIZE = 16*1024;
  MD5_CTX c;
  size_t num_read_bytes;
  uint8_t buff[BUFFER_SIZE];

  rewind(file);
  MD5_Init(&c);
  while(true){
    num_read_bytes = fread(buff, 1, BUFFER_SIZE, file);

    if(ferror(file)){
      done = false;
      break;
    }

    if(num_read_bytes > 0){
      MD5_Update(&c, buff, num_read_bytes);
    }

    if(feof(file)){
      break;
    }
  }

  if(done == true){
    MD5_Final(md5, &c);
  }

  return done;
}

bool perform_md5sum_file_path(const char* path, unsigned char* md5){
  bool done = false;
  FILE* file;

  if((file = fopen(path, "r")) != NULL){
    done = perform_md5sum_file(file, md5);
    fclose(file);
  }
  return done;
}

void delete_hashtag(sqlite3* db, sqlite3_int64 hash_index, const char* tag){
  sqlite3_stmt* stmt;
  int sqlite3_result;

  // Compile statement
  sqlite3_result = sqlite3_prepare_v2(db, "delete from hashtags where id = ? and tag = ?;", -1, &stmt, 0);
  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    return;
  }

  // Bind data
  sqlite3_bind_int64(stmt, 1, hash_index);
  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    return;
  }

  sqlite3_bind_text(stmt, 2, tag, strlen(tag), SQLITE_STATIC);
  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    return;
  }

  // Perform
  sqlite3_result = sqlite3_step(stmt);

  if(sqlite3_result == SQLITE_DONE){
  }
  else{
    print_error(__LINE__, sqlite3_result, db);
  }
}

bool insert_new_hashtag(sqlite3* db, sqlite3_int64 hash_index, const char* tag){
  bool done;
  sqlite3_stmt* stmt;
  int sqlite3_result;

  // Compile statement
  sqlite3_result = sqlite3_prepare_v2(db, "insert into hashtags (id, tag) values (?, ?);", -1, &stmt, 0);

  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    done = false;
    return done;
  }

  // Bind data
  sqlite3_bind_int64(stmt, 1, hash_index);
  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    done = false;
    return done;
  }

  sqlite3_bind_text(stmt, 2, tag, strlen(tag), SQLITE_STATIC);
  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    done = false;
    return done;
  }

  // Perform
  sqlite3_result = sqlite3_step(stmt);

  if(sqlite3_result == SQLITE_DONE){
    done = true;
  }
  else{
    print_error(__LINE__, sqlite3_result, db);
    done = false;
  }


  return done;
}

bool insert_new_hash(sqlite3* db, const void* hash_bytes, size_t hash_size, sqlite3_int64* index){
  bool done;
  sqlite3_stmt* stmt;
  int sqlite3_result;

  // Compile statement
  sqlite3_result = sqlite3_prepare_v2(db, "insert into hashes (id, md5sum) values (NULL, ?);", -1, &stmt, 0);

  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    done = false;
    return done;
  }

  // Bind data
  sqlite3_bind_blob(stmt, 1, hash_bytes, hash_size, SQLITE_STATIC);

  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    done = false;
    return done;
  }

  // Perform
  sqlite3_result = sqlite3_step(stmt);

  if(sqlite3_result == SQLITE_DONE){
    done = true;
    *index = (uint64_t)sqlite3_last_insert_rowid(db);
  }
  else{
    print_error(__LINE__, sqlite3_result, db);
    done = false;
  }


  return done;
}

bool search_hashtag_index(sqlite3* db, sqlite3_int64 hash_index, const char* tag){
  bool found;
  sqlite3_stmt* stmt;
  int sqlite3_result;

  // Compile statement
  sqlite3_result = sqlite3_prepare_v2(db, "select id from hashtags where id = ? and tag = ?;", -1, &stmt, 0);

  if(sqlite3_result != SQLITE_OK){
    found = false;
    print_error(__LINE__, sqlite3_result, db);
  }

  // Bind data
  sqlite3_bind_int64(stmt, 1, hash_index);
  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    found = false;
    return found;
  }

  sqlite3_bind_text(stmt, 2, tag, strlen(tag), SQLITE_STATIC);
  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    found = false;
    return found;
  }

  // Perform
  sqlite3_result = sqlite3_step(stmt);
  if(sqlite3_result == SQLITE_ROW){
    found = true;
  }
  else{
    found = false;
    if(sqlite3_result != SQLITE_DONE){
      print_error(__LINE__, sqlite3_result, db);
    }
  }

  sqlite3_finalize(stmt);
  return found;
}

bool search_hash_index(sqlite3* db, const void* hash_bytes, size_t hash_size, sqlite3_int64* index){
  bool found;
  sqlite3_stmt* stmt;
  int sqlite3_result;

  // Compile statement
  sqlite3_result = sqlite3_prepare_v2(db, "select id from hashes where md5sum = ?;", -1, &stmt, 0);

  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    found = false;
    return found;
  }

  // Bind data
  sqlite3_bind_blob(stmt, 1, hash_bytes, hash_size, SQLITE_STATIC);

  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    found = false;
    return found;
  }

  // Perform
  sqlite3_result = sqlite3_step(stmt);
  if(sqlite3_result == SQLITE_ROW){
    found = true;
    *index = sqlite3_column_int64(stmt, 0);
  }
  else{
    found = false;
    if(sqlite3_result != SQLITE_DONE){
      print_error(__LINE__, sqlite3_result, db);
    }
  }

  sqlite3_finalize(stmt);
  return found;

}

bool get_tags_for_index(sqlite3* db, sqlite3_int64 hash_index, char*** tags, size_t* tagc){

  bool done;
  sqlite3_stmt* stmt;
  int sqlite3_result;

  // Compile statement
  sqlite3_result = sqlite3_prepare_v2(db, "select count(tag) from hashtags where id = ?;", -1, &stmt, 0);

  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    done = false;
    return done;
  }

  // Bind data
  sqlite3_bind_int64(stmt, 1, hash_index);
  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    done = false;
    return done;
  }

  // Perform
  sqlite3_int64 count;
  sqlite3_result = sqlite3_step(stmt);
  if(sqlite3_result == SQLITE_ROW){
    count = sqlite3_column_int64(stmt, 0);
  }
  else{
    print_error(__LINE__, sqlite3_result, db);
    done = false;
    return done;
  }


  *tagc = count;
  if(count == 0){
    done = true;
  }
  else{
    *tags = malloc(count*sizeof(char*));
    // Compile statement
    sqlite3_result = sqlite3_prepare_v2(db, "select tag from hashtags where id = ?;", -1, &stmt, 0);

    if(sqlite3_result != SQLITE_OK){
      print_error(__LINE__, sqlite3_result, db);
      done = false;
      return done;
    }

    // Bind data
    sqlite3_bind_int64(stmt, 1, hash_index);
    if(sqlite3_result != SQLITE_OK){
      print_error(__LINE__, sqlite3_result, db);
      done = false;
      return done;
    }

    // Perform
    size_t t = 0;
    while(true){
      sqlite3_result = sqlite3_step(stmt);
      if(sqlite3_result == SQLITE_ROW){
        char* aux = (char*)sqlite3_column_text(stmt, 0);
        (*tags)[t] = malloc(strlen(aux));
        strcpy((*tags)[t], aux);
        t++; 
      }
      else if(sqlite3_result == SQLITE_DONE){
        done = true;
        break;
      }
      else{
        print_error(__LINE__, sqlite3_result, db);
        done = false;
        break;
      }
    }
  }
  sqlite3_finalize(stmt);
  return done;
}

bool tags_tag_file(const char* path, size_t tagc, char** tags){

  bool done;

  unsigned char md5[MD5_DIGEST_LENGTH];
  if(!perform_md5sum_file_path(path, md5)){
    done = false;
    return done;
  }

  sqlite3* db;
  int sqlite3_result;


  // Open database
  sqlite3_result = sqlite3_open("tags.db", &db);

  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    sqlite3_close(db);
    done = false;
  }
  else{
    // If hash is not already saved, save it
    sqlite3_int64 index;

    bool is_md5_in_db = search_hash_index(db, md5, sizeof(md5), &index);
    if(!is_md5_in_db){
      is_md5_in_db = insert_new_hash(db, md5, sizeof(md5), &index);
    }

    if(is_md5_in_db){
      // Check if relation exists. If not, make it.
      for(int t = 0; t < tagc; t++){
        bool is_relation_in_db = search_hashtag_index(db, index, tags[t]);
        if(!is_relation_in_db){
          is_relation_in_db = insert_new_hashtag(db, index, tags[t]);
        }

        if(is_relation_in_db){
          done = true;
        }
        else{
          done = false;
        }
      }
    }
    else{
      done = false;
    } 

  }

  sqlite3_close(db);
  return done;
  

}

bool tags_untag_file(const char* path, size_t tagc, char** tags){

  bool done;

  unsigned char md5[MD5_DIGEST_LENGTH];
  if(!perform_md5sum_file_path(path, md5)){
    done = false;
    return done;
  }

  sqlite3* db;
  int sqlite3_result;


  // Open database
  sqlite3_result = sqlite3_open("tags.db", &db);

  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    sqlite3_close(db);
    done = false;
  }
  else{
    // Search for hash
    sqlite3_int64 index;

    bool is_md5_in_db = search_hash_index(db, md5, sizeof(md5), &index);
    if(!is_md5_in_db){
      // Not registered hash. done.
      done = true;
    }
    else{
      // Delete hashtag relations
      for(int t = 0; t < tagc; t++){
        delete_hashtag(db, index, tags[t]);
      }
    }
  }

  sqlite3_close(db);
  return done;
}

bool tags_list_file(const char* path){
  bool done;

  unsigned char md5[MD5_DIGEST_LENGTH];
  if(!perform_md5sum_file_path(path, md5)){
    done = false;
    return done;
  }

  sqlite3* db;
  int sqlite3_result;


  // Open database
  sqlite3_result = sqlite3_open("tags.db", &db);

  if(sqlite3_result != SQLITE_OK){
    print_error(__LINE__, sqlite3_result, db);
    sqlite3_close(db);
    done = false;
  }
  else{
    // Search for hash
    sqlite3_int64 index;

    bool is_md5_in_db = search_hash_index(db, md5, sizeof(md5), &index);
    if(!is_md5_in_db){
      // Not registered hash. done.
      done = true;
    }
    else{
      // List tags
      size_t tagc;
      char** tags = NULL;
      if(get_tags_for_index(db, index, &tags, &tagc)){
        //free(tag);
        //for(int t = 0; t < tagc; t++){
        //  printf("%s\n", tags[t]);
        ////}
        if(tagc > 0){
          for(int t = 0; t < tagc; t++){
            printf("%s\n", tags[t]);
            free(tags[t]);
          }
          free(tags);
        }
        else{
          printf("no tags\n");
        }
      }
    }
  }

  sqlite3_close(db);
  return done;
}
