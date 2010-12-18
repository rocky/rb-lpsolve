#!/usr/bin/env ruby
require 'mkmf'
dir_config('lpsolve')
$libs = append_library($libs, "m -ldl -llpsolve55 -lm")

config_file = File.join(File.dirname(__FILE__), 'config_options.rb')
load config_file if File.exist?(config_file)

# if debug:
# $CFLAGS = "-Wall -g -fno-strict-aliasing -fPIC"
create_makefile('lpsolve')
