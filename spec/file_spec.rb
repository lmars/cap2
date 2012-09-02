require 'spec_helper'

describe Cap2::File do
  let(:file) { Tempfile.new('cap-test') }

  subject { Cap2::File.new(file.path) }

  describe '#permitted?' do
    context "when the file doesn't have the given capability" do
      it { should_not be_permitted(:dac_override) }
    end

    context 'when the file does have the given capability' do
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

    context 'when the file does have the given capability' do
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

    context 'when the file does have the given capability' do
      before(:each) do
        system %{sudo setcap "cap_dac_override+i" #{file.path}}
      end

      it { should be_inheritable(:dac_override) }
    end
  end

  describe '#permit' do
    specify do
      expect { running_as_root('permit(:fowner)') }.to \
        change { subject.permitted?(:fowner) }.from(false).to(true)
    end
  end

  describe '#unpermit' do
    before(:each) do
      run_as_root('permit(:fowner)')
    end

    specify do
      expect { running_as_root('unpermit(:fowner)') }.to \
        change { subject.permitted?(:fowner) }.from(true).to(false)
    end
  end

  describe '#allow_inherit' do
    specify do
      expect { running_as_root('allow_inherit(:chown)') }.to \
        change { subject.inheritable?(:chown) }.from(false).to(true)
    end
  end

  describe '#disallow_inherit' do
    before(:each) do
      run_as_root('allow_inherit(:chown)')
    end

    specify do
      expect { running_as_root('disallow_inherit(:chown)') }.to \
        change { subject.inheritable?(:chown) }.from(true).to(false)
    end
  end

  describe '#enable_on_exec' do
    context 'when the capability is not permitted or inheritable' do
      specify do
        expect { subject.enable_on_exec(:lease) }.to \
          raise_error(
            Cap2::IncompatibleCapabilities,
            'cannot enable_on_exec a capability which is neither permitted nor inheritable'
          )
      end
    end

    context 'when the capability is permitted' do
      before(:each) do
        run_as_root('permit(:lease)')
      end

      specify do
        expect { running_as_root('enable_on_exec(:lease)') }.to \
          change { subject.effective?(:lease) }.from(false).to(true)
      end
    end

    context 'when the capability is inheritable' do
      before(:each) do
        run_as_root('allow_inherit(:lease)')
      end

      specify do
        expect { running_as_root('enable_on_exec(:lease)') }.to \
          change { subject.effective?(:lease) }.from(false).to(true)
      end
    end
  end

  describe '#disable_on_exec' do
    before(:each) do
      run_as_root('permit(:kill)', 'enable_on_exec(:kill)')
    end

    specify do
      expect { running_as_root('disable_on_exec(:kill)') }.to \
        change { subject.effective?(:kill) }.from(true).to(false)
    end
  end

  # FIXME: Would like to call the given code on subject directly (e.g.
  #        `subject.permit(:fowner)`) but this would require the test
  #        suite to be run as root?
  def run_as_root(*codes)
    codes.each do |code|
      system %{sudo ruby -Ilib -rcap2 -e 'Cap2.file("#{file.path}").#{code}'}
    end
  end
  alias running_as_root run_as_root
end
