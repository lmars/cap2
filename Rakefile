$:.unshift File.dirname(__FILE__) + '/lib'
require 'cap2'

file 'bin/ruby.c' do
  File.open('bin/ruby.c', 'w') do |f|
    f.puts <<-EOS
#include <ruby.h>

int main(int argc, char *argv[]) {
  ruby_sysinit(&argc, &argv);
  RUBY_INIT_STACK;
  ruby_init();
  ruby_init_loadpath();
  rb_eval_string("require 'rbconfig'");
  rb_eval_string("RbConfig::CONFIG['bindir'] = '#{File.dirname(__FILE__) + '/bin'}'");
  return ruby_run_node(ruby_options(argc, argv));
}
    EOS
  end
end

file 'bin/Makefile' do
  File.open('bin/Makefile', 'w') do |f|
    prefix = RbConfig::CONFIG['prefix']
    arch   = RbConfig::CONFIG['arch']

    f.puts <<-EOS
LIB=#{prefix}/lib
INC=#{prefix}/include/ruby-1.9.1
CFLAGS=-I$(INC) -I$(INC)/#{arch}
LDFLAGS=-L$(LIB) -lruby

ruby: ruby.o
	$(CC) -o ruby ruby.o $(LDFLAGS)
    EOS
  end
end

file 'bin/ruby' => %w(bin/ruby.c bin/Makefile) do
  system 'cd bin && make'
end

task :setup => 'bin/ruby' do
  if Cap2.process.permitted?(:setfcap)
    Cap2.file('bin/ruby').permit(:setfcap)
  else
    fail 'The setup task must be run as root'
  end
end

task :check_caps do
  unless Cap2.process.permitted?(:setfcap)
    if File.exists?('bin/ruby')
      exec("bin/ruby -S rake #{ARGV.join ' '}")
    else
      fail 'You need to run "sudo rake setup" first'
    end
  end
end

require 'rake/extensiontask'

Rake::ExtensionTask.new('cap2')

require 'rspec/core'
require 'rspec/core/rake_task'

desc 'Run all specs in the spec directory'
RSpec::Core::RakeTask.new(:spec => [:check_caps, :clobber, :compile]) do |t|
  t.pattern = FileList['spec/**/*_spec.rb']
end

require 'rdoc/task'

RDoc::Task.new do |rd|
  rd.title = 'Cap2'
  rd.rdoc_files.include("lib/**/*.rb", "ext/**/*.c")
end
