# -*- Ruby -*-
# -*- encoding: utf-8 -*-
require 'rake'
require 'rubygems' unless 
  Object.const_defined?(:Gem)

LPSOLVE_VERSION = open("VERSION").read.chomp
PKG_NAME = 'lpsolve'

FILES = FileList[
  'Rakefile',
  'VERSION',
  'doc/*',
  'example/*',
  'ext/*.c',
  'ext/Makefile',
  'ext/extconf.rb',
  'test/*.rb',
  'test/*.right',
  'test/Rakefile',
]

desc "Create GEM spec file"
default_spec = Gem::Specification.new do |spec|
  spec.name = PKG_NAME
  
  spec.homepage = "http://github.org/rb-lpsolve/"
  spec.summary = "Ruby interface to lpsolve version 5.5.0.10"
  spec.description = <<-EOF
A Ruby library for using simplex-method Mixed Integer Linear Programming solver, lpsolve version 0.5.5. 
Pick up the C code for lpsolve at http://bashdb.sf.net/lpsolve-5.5.0.10i.tar.bz2
EOF

  spec.version = LPSOLVE_VERSION

  spec.author = "Rocky Bernstein"
  spec.email = "rockby@rubyforge.org"
  spec.platform = Gem::Platform::RUBY
  spec.require_path = "lib" 
  spec.bindir = "bin"
  spec.executables = []
  spec.extensions = ["ext/extconf.rb"]
  spec.files = FILES.to_a  
  spec.test_files = FileList['test/*.rb', 'test/*.right', 'test/Rakefile']

  spec.required_ruby_version = '>= 1.8.4'
  spec.date = Time.now
  spec.rubyforge_project = 'lpsolve'
  
  # rdoc
  # spec.has_rdoc = false
end
