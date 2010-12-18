require "rubygems"
Mypath  = File.expand_path(File.dirname(__FILE__))
old_dir = File.expand_path(Dir.pwd)
if old_dir != Mypath
  Dir.chdir(Mypath)
end
$: << Mypath + '/../ext'
require "lpsolve"
