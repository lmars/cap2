require 'cap2.so'
require 'cap2/entity'
require 'cap2/process'
require 'cap2/file'

module Cap2
  # = Cap2
  #
  # Cap2 is a module for querying the POSIX 1003.1e capabilities
  # available in Linux kernels. These capabilities are a
  # partitioning of the all powerful root privilege into a set
  # of distinct privileges.
  class << self
    # A wrapper function around the native Cap2.pid_has_cap?
    # and Cap2.file_has_cap?. Returns true if the given
    # capability is enabled in the given set for the given pid /
    # filename.
    #
    # @param [String, Fixnum] pid_or_filename
    #   When a Fixnum, query the capabilities for the process
    #   with this value. When a String, query for the file
    #   with a filename of this value.
    #
    # @param [Symbol] set
    #   One of :permitted, :effective or :inheritable.
    #
    # @param [Symbol] cap
    #   A lower cased name of a capability, without the 'CAP_'
    #   prefix. For example, :chown would query the CAP_CHOWN
    #   capability
    def has_capability?(pid_or_filename, set, cap)
      set = SETS[set.to_s]
      cap = CAPS[cap.to_s]

      case pid_or_filename
      when Fixnum
        pid_has_cap?(pid_or_filename, set, cap)
      when String
        file_has_cap?(pid_or_filename, set, cap)
      else
        raise ArgumentError, "wrong argument type, expected Fixnum or String, got #{pid_or_filename.class}"
      end
    end

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

  class ProcessNotFound < StandardError; end
  class FileNotFound < StandardError; end
end
