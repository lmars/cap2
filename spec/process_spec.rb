require 'spec_helper'

describe Cap2::Process do
  describe '#permitted?' do
    context "when the process doesn't have the given capabilities" do
      subject { Cap2::Process.new(Process.pid) }

      it { should_not be_permitted(:dac_override, :chown) }
    end

    context 'when the process does have the given capabilities' do
      subject { Cap2::Process.new(1) }

      it { should be_permitted(:dac_override, :chown) }
    end
  end

  describe '#enabled?' do
    context "when the process doesn't have the given capabilities" do
      subject { Cap2::Process.new(Process.pid) }

      it { should_not be_enabled(:dac_override, :chown) }
    end

    context 'when the process does have the given capabilities' do
      subject { Cap2::Process.new(1) }

      it { should be_enabled(:dac_override, :chown) }
    end
  end
end
