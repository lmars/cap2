require 'spec_helper'

describe Cap2::File do
  let(:file) { Tempfile.new('cap-test') }

  subject { Cap2::File.new(file.path) }

  describe '#permitted?' do
    context "when the file doesn't have the given capability" do
      it { should_not be_permitted(:dac_override) }
    end

    context 'when the process does have the given capability' do
      before(:each) do
        system %{sudo setcap "cap_dac_override+p" #{file.path}}
      end

      it { should be_permitted(:dac_override) }
    end
  end

  describe '#effective?' do
    context "when the file doesn't have the given capability" do
      it { should_not be_effective(:dac_override) }
    end

    context 'when the process does have the given capability' do
      before(:each) do
        system %{sudo setcap "cap_dac_override+pe" #{file.path}}
      end

      it { should be_effective(:dac_override) }
    end
  end

  describe '#inheritable?' do
    context "when the file doesn't have the given capability" do
      it { should_not be_inheritable(:dac_override) }
    end

    context 'when the process does have the given capability' do
      before(:each) do
        system %{sudo setcap "cap_dac_override+i" #{file.path}}
      end

      it { should be_inheritable(:dac_override) }
    end
  end
end

