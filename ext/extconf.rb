#!/usr/bin/env ruby
require 'mkmf'
dir_config('lpsolve')
$libs = append_library($libs, "m -ldl -llpsolve55 -lm")
# if debug:
$CFLAGS = "-Wall -g -fno-strict-aliasing -fPIC"
create_makefile('lpsolve')
