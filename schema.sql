create table hashes(
  id integer primary key,
  md5sum blob 
);

create table hashtags(
  id integer,
  tag text,
  primary key(id, tag)
);
