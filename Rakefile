directory 'tmp'

# Generate a C source file to build a custom Ruby binary which will be used to
# run the tests with the CAP_SETFCAP capability.
#
# NOTE: Need to override the Ruby `bindir` so that FileUtils::RUBY (set by
#       rake in lib/rake/file_utils.rb) will execute the custom binary we
#       compile rather than the system Ruby
file 'tmp/ruby.c' => 'tmp' do
  File.open('tmp/ruby.c', 'w') do |f|
    bindir_override =
      "RbConfig::CONFIG['bindir'] = '#{File.dirname(__FILE__) + '/tmp'}'"

    f.puts <<-EOS
#include <ruby.h>

int main(int argc, char *argv[]) {
  ruby_sysinit(&argc, &argv);
  RUBY_INIT_STACK;
  ruby_init();
  ruby_init_loadpath();
  rb_eval_string("require 'rbconfig'");
  rb_eval_string("#{bindir_override}");
  return ruby_run_node(ruby_options(argc, argv));
}
    EOS
  end
end

file 'tmp/Makefile' => 'tmp' do
  # Check to make sure Ruby was compiled with --enable-shared as we will
  # need libruby.so
  unless RbConfig::CONFIG['ENABLE_SHARED'] == 'yes'
    fail %{
      You can only run these tasks if your Ruby was compiled with
      --enable-shared. Please either recompile your Ruby or switch
      to a suitable version.
    }
  end

  File.open('tmp/Makefile', 'w') do |f|
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

file 'tmp/ruby' => %w(tmp/ruby.c tmp/Makefile) do
  system 'cd tmp && make'
end

desc 'Compile a custom Ruby binary for running tests with CAP_SETFCAP enabled'
task :setup => ['tmp/ruby', :clean, :compile] do
  require File.dirname(__FILE__) + '/lib/cap2'

  if Cap2.process.permitted?(:setfcap)
    Cap2.file('tmp/ruby').permit(:setfcap)
  else
    fail 'The setup task must be run as root'
  end
end

task :check_caps do
  require File.dirname(__FILE__) + '/lib/cap2'

  unless Cap2.process.permitted?(:setfcap)
    if File.exists?('tmp/ruby')
      exec("tmp/ruby -S rake #{ARGV.join ' '}")
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
RSpec::Core::RakeTask.new(:spec => [:clean, :compile, :check_caps]) do |t|
  t.pattern = FileList['spec/**/*_spec.rb']
end

require 'rdoc/task'

RDoc::Task.new do |rd|
  rd.title = 'Cap2'
  rd.rdoc_files.include("lib/**/*.rb", "ext/**/*.c")
end
