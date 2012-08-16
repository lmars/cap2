module Cap2
  class Process < Entity
    # = Cap2::Process
    #
    # A class with methods for querying capabilities for the
    # process with pid provided to the initialize method.
    def initialize(pid)
      @entity_id = pid
    end
  end
end
