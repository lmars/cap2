require 'spec_helper'

describe Cap2::File do
  let(:filename) { double 'filename' }
  let(:capability) { double 'capability' }

  subject { Cap2::File.new(filename) }

  describe '#permitted?' do
    it 'should call Cap2.has_capability? correctly' do
      Cap2.
        should_receive(:has_capability?).
        with(filename, Cap2::PERMITTED, capability)

      subject.permitted?(capability)
    end
  end
end

