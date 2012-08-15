require 'cap2.so'
require 'cap2/process'

module Cap2
  class << self
    def process(pid = ::Process.pid)
      Process.new(pid)
    end
  end
end
