require 'spec_helper'

describe Cap2::Process do
  let(:pid) { double 'pid' }
  let(:capability) { double 'capability' }

  subject { Cap2::Process.new(pid) }

  describe '#permitted?' do
    it 'should call Cap2.has_capability? correctly' do
      Cap2.
        should_receive(:has_capability?).
        with(pid, Cap2::PERMITTED, capability)

      subject.permitted?(capability)
    end
  end
end
