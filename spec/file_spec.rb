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
        run_as_root('permit(:dac_override)')
      end

      it { should be_permitted(:dac_override) }
    end
  end

  describe '#enabled?' do
    context "when the file's enabled bit is not set" do
      it { should_not be_enabled }
    end

    context "when the file's enabled bit is set" do
      before(:each) do
        run_as_root('permit(:dac_override)', 'enable')
      end

      it { should be_enabled }
    end
  end

  describe '#inheritable?' do
    context "when the file doesn't have the given capability" do
      it { should_not be_inheritable(:dac_override) }
    end

    context 'when the file does have the given capability' do
      before(:each) do
        run_as_root('allow_inherit(:dac_override)')
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

  context 'enabling and disabling' do
    context 'when at least one capability is permitted' do
      before(:each) do
        run_as_root('permit(:kill)')
      end

      describe '#enable' do
        specify do
          expect { running_as_root('enable') }.to \
            change { subject.enabled? }.from(false).to(true)
        end
      end

      describe '#disable' do
        before(:each) do
          run_as_root('enable')
        end

        specify do
          expect { running_as_root('disable') }.to \
            change { subject.enabled? }.from(true).to(false)
        end
      end
    end

    context 'when no capabilities are permitted or inheritable' do
      describe '#enable' do
        specify do
          expect { running_as_root('enable') }.to_not \
            change { subject.enabled? }.from(false)
        end
      end

      describe '#disable' do
        before(:each) do
          run_as_root('enable')
        end

        specify do
          expect { running_as_root('disable') }.to_not \
            change { subject.enabled? }.from(false)
        end
      end
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
