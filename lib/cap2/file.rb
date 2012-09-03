require 'cap2/set_methods'

module Cap2
  # A class with methods for querying capabilities for the
  # file with filename provided to the initialize method.
  class File
    include SetMethods

    # Initialize a new File object for the given filename.
    def initialize(filename)
      @filename = filename
    end

    # Returns whether or not the file has any effective
    # capabilities.
    def enabled?
      NAMES.any? { |c| effective?(c) }
    end
  end
end
