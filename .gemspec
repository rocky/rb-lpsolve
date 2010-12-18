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
  
  spec.homepage = "http://rubyforge.org/projects/lpsolve/"
  spec.summary = "Ruby interface lpsolve (Mixed Integer Linear Programming, MILP, solver)"
  spec.description = <<-EOF
A library for using CD-ROM and CD image access. Applications wishing to be
oblivious of the OS- and device-dependent properties of a CD-ROM or of
the specific details of various CD-image formats may benefit from
using this library. A library for working with ISO-9660 filesystems
is included.
EOF

  spec.version = LPSOLVE_VERSION

  spec.author = "Rocky Bernstein"
  spec.email = "rocky@ce-interactive.com"
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
