module Cap2
  # A mixin for the Cap2::Process and Cap2::File
  # classes providing convenience methods for querying
  # permitted, effective and inheritable capabilities.
  #
  # Each method takes a capability argument, a lower
  # cased name of a capability, without the 'CAP_' prefix.
  # For example, :chown would query the CAP_CHOWN capability.
  module SetMethods
    # Returns whether the given capability is permitted
    def permitted?(capability)
      has?(:permitted, capability)
    end

    # Returns whether the given capability is effective
    def effective?(capability)
      has?(:effective, capability)
    end

    # Returns whether the given capability is inheritable
    def inheritable?(capability)
      has?(:inheritable, capability)
    end
  end
end

