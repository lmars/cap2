module Cap2
  class File < Entity
    # = Cap2::File
    #
    # A class with methods for querying capabilities for the
    # file with filename provided to the initialize method.
    def initialize(filename)
      @entity_id = filename
    end
  end
end
