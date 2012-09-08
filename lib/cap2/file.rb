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
      @caps[:permitted].merge parse(capabilities)
      save
    end

    # Dont permit processes executing this file to enable the given capabilities.
    def unpermit(*capabilities)
      @caps[:permitted].subtract parse(capabilities)
      save
    end

    # Allow processes executing this file to inherit the given capabilities.
    def allow_inherit(*capabilities)
      @caps[:inheritable].merge parse(capabilities)
      save
    end

    # Dont allow processes executing this file to inherit the given capabilities.
    def disallow_inherit(*capabilities)
      @caps[:inheritable].subtract parse(capabilities)
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

    def parse(caps)
      if (options = caps.first).is_a? Hash
        if caps.size > 1
          raise(
            ArgumentError,
            'cannot pass both a hash and a list of capabilities'
          )
        end

        if options.has_key?(:only) && !options.has_key?(:except)
          Array(options[:only])
        elsif options.has_key?(:except) && !options.has_key?(:only)
          Cap2.allcaps - Array(options[:except])
        else
          raise(
            ArgumentError,
            "expected exactly one of [:only, :except], got #{options.keys}"
          )
        end
      else
        caps
      end
    end
  end
end
