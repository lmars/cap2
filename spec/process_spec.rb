require 'spec_helper'

describe Cap2::Process do
  describe '#permitted?' do
    context "when the process doesn't have the given capability" do
      subject { Cap2::Process.new(Process.pid) }

      it { should_not be_permitted(:dac_override) }
    end

    context 'when the process does have the given capability' do
      subject { Cap2::Process.new(1) }

      it { should be_permitted(:dac_override) }
    end
  end

  describe '#effective?' do
    context "when the process doesn't have the given capability" do
      subject { Cap2::Process.new(Process.pid) }

      it { should_not be_effective(:dac_override) }
    end

    context 'when the process does have the given capability' do
      subject { Cap2::Process.new(1) }

      it { should be_effective(:dac_override) }
    end
  end
end
