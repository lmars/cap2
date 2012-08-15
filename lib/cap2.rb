require 'cap2.so'
require 'cap2/process'

module Cap2
  class << self
    def process(pid = ::Process.pid)
      check_pid! pid

      Process.new(pid)
    end

    private
    def check_pid!(pid)
      ::Process.kill(0, pid)
    rescue Exception => e
      if Errno::ESRCH === e
        raise ProcessNotFound
      end
    end
  end

  class ProcessNotFound < StandardError; end
end
