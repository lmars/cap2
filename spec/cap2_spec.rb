require 'spec_helper'

describe Cap2 do
  describe '.has_capability?' do
    context 'when the given process does not have the given capability' do
      it { should_not have_capability(
          Process.pid, Cap2::PERMITTED, Cap2::DAC_OVERRIDE) }
    end

    context 'when the given process does have the given capability' do
      it { should have_capability(
          1, Cap2::PERMITTED, Cap2::DAC_OVERRIDE) }
    end
  end
end
