require 'spec_helper'

shared_examples_for 'an entity' do
  let(:entity_id) { double 'entity id' }
  let(:capability) { double 'capability' }

  subject { described_class.new(entity_id) }

  describe '#permitted?' do
    it 'should call Cap2.has_capability? correctly' do
      Cap2.
        should_receive(:has_capability?).
        with(entity_id, Cap2::PERMITTED, capability)

      subject.permitted?(capability)
    end
  end

  describe '#effective?' do
    it 'should call Cap2.has_capability? correctly' do
      Cap2.
        should_receive(:has_capability?).
        with(entity_id, Cap2::EFFECTIVE, capability)

      subject.effective?(capability)
    end
  end

  describe '#inheritable?' do
    it 'should call Cap2.has_capability? correctly' do
      Cap2.
        should_receive(:has_capability?).
        with(entity_id, Cap2::INHERITABLE, capability)

      subject.inheritable?(capability)
    end
  end
end
