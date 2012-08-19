require 'rake/extensiontask'

Rake::ExtensionTask.new('cap2')

require 'rspec/core'
require 'rspec/core/rake_task'

desc 'Run all specs in the spec directory'
RSpec::Core::RakeTask.new(:spec => [:clobber, :compile]) do |t|
  t.pattern = FileList['spec/**/*_spec.rb']
end

require 'rdoc/task'

RDoc::Task.new do |rd|
  rd.title = 'Cap2'
  rd.rdoc_files.include("lib/**/*.rb", "ext/**/*.c")
end
