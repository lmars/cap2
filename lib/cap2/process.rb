module Cap2
  # A class with methods for managing capabilities for the
  # process with pid provided to the initialize method.
  class Process
    # Initialize a new Process object for the given pid.
    def initialize(pid)
      @pid  = pid
      @caps = getcaps
    end

    # Returns whether the given capability is permitted
    def permitted?(capability)
      reload
      @caps[:permitted].include? capability
    end

    # Returns whether the given capability is effective
    def effective?(capability)
      reload
      @caps[:effective].include? capability
    end

    # Returns whether the given capability is inheritable
    def inheritable?(capability)
      reload
      @caps[:inheritable].include? capability
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

    def reload
      @caps = getcaps
    end
  end
end
