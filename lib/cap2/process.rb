module Cap2
  class Process
    def initialize(pid)
      @pid = pid
    end

    def permitted?(capability)
      Cap2.has_capability?(@pid, Cap2::PERMITTED, capability)
    end
  end
end
