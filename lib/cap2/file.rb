module Cap2
  # A class with methods for querying capabilities for the
  # file with filename provided to the initialize method.
  class File
    # Initialize a new File object for the given filename.
    def initialize(filename)
      @filename = filename
      @caps     = getcaps
    end

    # Returns whether the given capability is permitted
    def permitted?(capability)
      reload
      @caps[:permitted].include? capability
    end

    # Returns whether the given capability is inheritable
    def inheritable?(capability)
      reload
      @caps[:inheritable].include? capability
    end

    # Returns whether or not the file has any effective
    # capabilities.
    def enabled?
      reload
      !@caps[:effective].empty?
    end

    # Permit processes executing this file to enable the given capability.
    def permit(capability)
      @caps[:permitted].add(capability)
      save
    end

    # Dont permit processes executing this file to enable the given capability.
    def unpermit(capability)
      @caps[:permitted].delete(capability)
      save
    end

    # Allow processes executing this file to inherit the given capability.
    def allow_inherit(capability)
      @caps[:inheritable].add(capability)
      save
    end

    # Dont allow processes executing this file to inherit the given capability.
    def disallow_inherit(capability)
      @caps[:inheritable].delete(capability)
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

    private
    def reload
      @caps = getcaps
    end
  end
end
