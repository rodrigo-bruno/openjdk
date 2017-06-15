#!/bin/bash

ctags -R .

find src/ -type f > cscope.files
cscope -b -q -k

# Setup .vimrc
# Setup .vim/plugin/cscope
