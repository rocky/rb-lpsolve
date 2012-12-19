#!/usr/bin/env rake
# -*- Ruby -*-
require 'rubygems'
require 'fileutils'

Gemspec_filename='rb-lpsolve.gemspec'

ROOT_DIR = File.dirname(__FILE__)

def gemspec
  @gemspec ||= eval(File.read(Gemspec_filename), binding, Gemspec_filename)
end

require 'rubygems/package_task'
desc "Build the gem"
task :package=>:gem
task :gem=>:gemspec do
  Dir.chdir(ROOT_DIR) do
    sh "gem build #{Gemspec_filename}"
    FileUtils.mkdir_p 'pkg'
    FileUtils.mv("#{gemspec.file_name}", "pkg/")
  end
end

desc "Install the gem locally"
task :install => :gem do
  Dir.chdir(ROOT_DIR) do
    sh %{gem install --local pkg/#{gemspec.file_name}}
  end
end

### Windows specification
##win_spec = default_spec.clone
##win_spec.extensions = []
##win_spec.platform = Gem::Platform::WIN32
##win_spec.files += ["lib/#{SO_NAME}"]
##
##desc "Create Windows Gem"
##task :win32_gem do
##  # Copy the win32 extension the top level directory
##  current_dir = File.expand_path(File.dirname(__FILE__))
##  source = File.join(current_dir, "ext", "win32", SO_NAME)
##  target = File.join(current_dir, "lib", SO_NAME)
##  cp(source, target)
##
##  # Create the gem, then move it to pkg
##	Gem::Builder.new(win_spec).build
##	gem_file = "#{win_spec.name}-#{win_spec.version}-#{win_spec.platform}.gem"
##  mv(gem_file, "pkg/#{gem_file}")
##
##  # Remove win extension fro top level directory	
##	rm(target)
##end


desc 'generate Doxygen Documentation'
task :doc do
  system("cd doc && ./run_doxygen")
end

task :default => [:test]
require 'rake/testtask'
desc 'Test the lpsolve shared object.'
Rake::TestTask.new('test' => :ext) do |t|
  t.pattern = 'test/*.rb'
  t.warning = true
end

# 'check' is an the same thing as 'test'
desc "same as test"
task :check => :test

desc "clean derived files"
task :clean do
  Dir.chdir File.join(ROOT_DIR, 'ext') do
    if File.exist?('Makefile')
      sh 'make clean'
      rm 'Makefile'
    end
    derived_files = Dir.glob('.o') + Dir.glob('*.so')
    rm derived_files unless derived_files.empty?
  end
end

desc "make C extension"
task :'ext/Makefile'=>:ext
task :ext do
  Dir.chdir('ext') do
    system("#{Gem.ruby} extconf.rb && make")
  end
end

