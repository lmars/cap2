require 'cap2/set_methods'

module Cap2
  class File
    # = Cap2::File
    #
    # A class with methods for querying capabilities for the
    # file with filename provided to the initialize method.

    include SetMethods

    def initialize(filename)
      @filename = filename
    end
  end
end
