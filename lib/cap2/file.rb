module Cap2
  class File
    def initialize(filename)
      @filename = filename
    end

    def permitted?(capability)
      Cap2.has_capability?(@filename, Cap2::PERMITTED, capability)
    end
  end
end
