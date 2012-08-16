module Cap2
  class Entity
    # = Entity
    #
    # A superclass for the Cap2::Process and Cap2::File
    # classes providing convenience methods for querying
    # permitted, effective and inheritable capabilities.
    #
    # Each method takes a capability argument, a lower
    # cased name of a capability, without the 'CAP_' prefix.
    # For example, :chown would query the CAP_CHOWN capability.

    def permitted?(capability)
      has?(:permitted, capability)
    end

    def effective?(capability)
      has?(:effective, capability)
    end

    def inheritable?(capability)
      has?(:inheritable, capability)
    end

    private
    def has?(set, cap)
      Cap2.has_capability?(@entity_id, set, cap)
    end
  end
end

