require 'cap2/set_methods'

module Cap2
  # A class with methods for querying capabilities for the
  # process with pid provided to the initialize method.
  class Process
    include SetMethods

    # Initialize a new Process object for the given pid.
    def initialize(pid)
      @pid  = pid
      @caps = getcaps
    end

    # Enable the given capability for this process.
    def enable(capability)
      check_pid
      @caps[:effective].add(capability)
      save
    end

    # Disable the given capability for this process.
    def disable(capability)
      check_pid
      @caps[:effective].delete(capability)
      save
    end

    private
    # Raises a RuntimeError if the process's pid is not the same as the current
    # pid (you cannot enable capabilities for other processes, that's their job).
    def check_pid
      unless @pid == Process.pid
        raise 'Cannot modify capabilities of other processes'
      end
    end
  end
end
