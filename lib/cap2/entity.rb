module Cap2
  class Entity
    def permitted?(capability)
      Cap2.has_capability?(@entity_id, Cap2::PERMITTED, capability)
    end
  end
end
