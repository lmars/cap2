module Cap2
  # A class with methods for managing capabilities for the
  # file with filename provided to the initialize method.
  class File
    # Initialize a new File object for the given filename.
    def initialize(filename)
      @filename = filename
      @caps     = getcaps
    end

    # Returns whether the given capabilities are permitted
    def permitted?(*capabilities)
      reload
      @caps[:permitted].superset? Set[*capabilities]
    end

    # Returns whether the given capabilities are inheritable
    def inheritable?(*capabilities)
      reload
      @caps[:inheritable].superset? Set[*capabilities]
    end

    # Returns whether or not the file has any effective
    # capabilities.
    def enabled?
      reload
      !@caps[:effective].empty?
    end

    # Permit processes executing this file to enable the given capabilities.
    def permit(*capabilities)
      @caps[:permitted].merge(capabilities)
      save
    end

    # Dont permit processes executing this file to enable the given capabilities.
    def unpermit(*capabilities)
      @caps[:permitted].subtract(capabilities)
      save
    end

    # Allow processes executing this file to inherit the given capabilities.
    def allow_inherit(*capabilities)
      @caps[:inheritable].merge(capabilities)
      save
    end

    # Dont allow processes executing this file to inherit the given capabilities.
    def disallow_inherit(*capabilities)
      @caps[:inheritable].subtract(capabilities)
      save
    end

    # Enable the permitted capabilities when a proces executes this file.
    def enable
      @caps[:effective] = @caps[:permitted] + @caps[:inheritable]
      save
    end

    # Dont enable the permitted capabilities when a proces executes this file.
    def disable
      @caps[:effective].clear
      save
    end

    # Clear all capabilites
    def clear
      @caps.each_pair { |_, s| s.clear }
      save
    end

    private
    def reload
      @caps = getcaps
    end
  end
end
