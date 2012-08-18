$LOAD_PATH.unshift 'lib'
require 'cap2/version'

Gem::Specification.new do |s|
  s.name     = 'cap2'
  s.version  = Cap2::Version
  s.date     = Time.now.strftime('%Y-%m-%d')
  s.summary  = 'A Ruby library for managing Linux file and process capabilities'
  s.authors  = ['Lewis Marshall']
  s.email    = ['lewismarshall86@gmail.com']
  s.homepage = 'https://github.com/lmars/cap2'

  s.files = %w(README.md Rakefile LICENSE)
  s.files += Dir.glob('ext/**/*')
  s.files += Dir.glob('lib/**/*')
  s.files += Dir.glob('spec/**/*')

  s.extensions = ['ext/cap2/extconf.rb']

  s.description = <<-DESC
    Cap2 is a Ruby library for managing the POSIX 1003.1e capabilities
    available in Linux kernels.

    These capabilities are a partitioning of the all powerful root
    privilege into a set of distinct privileges.

    See capabilites(7) for more information.
  DESC
end
