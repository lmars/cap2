# = Cap2::File
#
# A class with methods for querying capabilities for the
# file with filename provided to the initialize method.

require 'cap2/set_methods'

module Cap2
  class File
    include SetMethods

    # Initialize a new File object for the given filename.
    def initialize(filename)
      @filename = filename
    end

    # Enable the given capability in the file's effective set.
    #
    # The capability must be either permitted or inheritable (or else it cannot
    # possibly be enabled in the new process).
    def enable_on_exec(capability)
      if permitted?(capability) || inheritable?(capability)
        set_effective(capability)
      else
        raise IncompatibleCapabilities, 'cannot enable_on_exec a capability which is neither permitted nor inheritable'
      end
    end
  end
end
