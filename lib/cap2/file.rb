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

    def enable_on_exec(capability)
      if permitted?(capability) || inheritable?(capability)
        set_effective(capability)
      else
        raise IncompatibleCapabilities, 'cannot enable_on_exec a capability which is neither permitted nor inheritable'
      end
    end
  end
end
