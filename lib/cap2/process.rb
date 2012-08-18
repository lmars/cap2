require 'cap2/set_methods'

module Cap2
  class Process
    # = Cap2::Process
    #
    # A class with methods for querying capabilities for the
    # process with pid provided to the initialize method.
    include SetMethods

    def initialize(pid)
      @pid = pid
    end
  end
end
