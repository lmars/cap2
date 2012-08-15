module Cap2
  class Entity
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

