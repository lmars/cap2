require 'set'
require 'cap2.so'
require 'cap2/process'
require 'cap2/file'

# Cap2 is a module for querying the POSIX 1003.1e capabilities
# available in Linux kernels. These capabilities are a
# partitioning of the all powerful root privilege into a set
# of distinct privileges.
#
# For more information see capabilities(7).
module Cap2
  class << self
    # Returns a Cap2::Process initialized with the given pid,
    # defaulting to the current pid.
    def process(pid = ::Process.pid)
      check_pid! pid

      Process.new(pid)
    end

    # Returns a Cap2::File initialized with the given filename
    def file(filename)
      check_file! filename

      File.new(filename)
    end

    private
    # Check that a process with the given pid exists by sending
    # a 0 signal to the process via Process.kill
    def check_pid!(pid)
      ::Process.kill(0, pid)
    rescue Exception => e
      if Errno::ESRCH === e
        raise ProcessNotFound
      end
    end

    # Check that a file exists with the given filename
    def check_file!(filename)
      unless ::File.exists?(filename)
        raise FileNotFound
      end
    end
  end

  # Raised when trying to initialise a Process object for a non-existent pid.
  class ProcessNotFound < StandardError; end

  # Raised when trying to initialise a File object for a non-existent file.
  class FileNotFound < StandardError; end
end
