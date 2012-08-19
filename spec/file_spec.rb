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

  describe '#permit' do
    let(:permitting_fowner_on_the_file) do
      # FIXME: Would like to use `subject.permit(:fowner)` but this would
      #        require the test suite to be run as root?
      lambda { system %{sudo ruby -Ilib -rcap2 -e 'Cap2.file("#{file.path}").permit(:fowner)'} }
    end

    specify do
      expect(&permitting_fowner_on_the_file).to \
        change { subject.permitted?(:fowner) }.from(false).to(true)
    end
  end

  describe '#unpermit' do
    before(:each) do
      system %{sudo ruby -Ilib -rcap2 -e 'Cap2.file("#{file.path}").permit(:fowner)'}
    end

    let(:unpermitting_fowner_on_the_file) do
      # FIXME: Would like to use `subject.unpermit(:fowner)` but this would
      #        require the test suite to be run as root?
      lambda { system %{sudo ruby -Ilib -rcap2 -e 'Cap2.file("#{file.path}").unpermit(:fowner)'} }
    end

    specify do
      expect(&unpermitting_fowner_on_the_file).to \
        change { subject.permitted?(:fowner) }.from(true).to(false)
    end
  end
end
