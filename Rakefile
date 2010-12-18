#!/usr/bin/env rake
# -*- Ruby -*-
require 'rubygems'

ROOT_DIR = File.dirname(__FILE__)

@gemspec ||= eval(File.read('.gemspec'), binding, '.gemspec')

require 'rake/gempackagetask'
task :default => [:package]

desc "Install the gem locally"
task :install => :gem do
  Dir.chdir(ROOT_DIR) do
    sh %{gem install --local pkg/#{gemspec.name}-#{gemspec.version}}
  end
end

# --- Redo Rake::PackageTask::define so tar uses -h to include
# files of a symbolic link.
module Rake
  class PackageTask < TaskLib
    # Create the tasks defined by this task library.
    def define
      fail "Version required (or :noversion)" if @version.nil?
      @version = nil if :noversion == @version

      desc "Build all the packages"
      task :package => [:lib]
      
      desc "Force a rebuild of the package files"
      task :repackage => [:clobber_package, :package]
      
      desc "Remove package products" 
      task :clobber_package do
	rm_r package_dir rescue nil
      end

      task :clobber => [:clobber_package]

      [
	[need_tar, tgz_file, "z"],
	[need_tar_gz, tar_gz_file, "z"],
	[need_tar_bz2, tar_bz2_file, "j"]
      ].each do |(need, file, flag)|
	if need
	  task :package => ["#{package_dir}/#{file}"]
	  file "#{package_dir}/#{file}" => [package_dir_path] + package_files do
	    chdir(package_dir) do
	      sh %{tar #{flag}hcvf #{file} #{package_name}}
	    end
	  end
	end
      end
      
      if need_zip
	task :package => ["#{package_dir}/#{zip_file}"]
	file "#{package_dir}/#{zip_file}" => [package_dir_path] + package_files do
	  chdir(package_dir) do
	    sh %{zip -r #{zip_file} #{package_name}}
	  end
	end
      end

      directory package_dir

      file package_dir_path => @package_files do
	mkdir_p package_dir rescue nil
	@package_files.each do |fn|
	  f = File.join(package_dir_path, fn)
	  fdir = File.dirname(f)
	  mkdir_p(fdir) if !File.exist?(fdir)
	  if File.directory?(fn)
	    mkdir_p(f)
	  else
	    rm_f f
	    safe_ln(fn, f)
	  end
	end
      end
      self
    end
  end
end

# ---------  GEM package ------
# Rake task to build the default package
desc "Build all the packages (gem, tgz, zip)"
Rake::GemPackageTask.new(@gemspec) do |pkg|
  pkg.need_zip = true
  pkg.need_tar = true
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


desc 'Generate Doxygen Documentation'
task :doc do
  system("cd doc && ./run_doxygen")
end

require 'rake/testtask'
desc 'Test the lpsolve shared object.'
Rake::TestTask.new('test' => :lib) do |t|
  t.pattern = 'test/*.rb'
  t.warning = true
end

# 'check' is an the same thing as 'test'
desc "same as test"
task :check => :test

# ---------  Clean derived files ------
task :clean do
  system("cd ext && rm Makefile *.o *.so")
end

# ---------  Clean derived files ------
task :lib do
  system("cd ext && ruby extconf.rb && make")
end

