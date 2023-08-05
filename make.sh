#/usr/bin/bash

function create_dir() {
  if [ ! -d $1 ]; then
    mkdir $1
  fi
}

create_dir 'bin'
create_dir 'log'
create_dir 'obj'
create_dir 'lib'

make clean && make -j4
