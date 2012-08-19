require 'cap2/set_methods'

module Cap2
  # A class with methods for querying capabilities for the
  # process with pid provided to the initialize method.
  class Process
    include SetMethods

    # Initialize a new Process object for the given pid.
    def initialize(pid)
      @pid = pid
    end
  end
end
